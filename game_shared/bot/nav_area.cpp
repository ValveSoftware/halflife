// nav_area.cpp
// AI Navigation areas
// Author: Michael S. Booth (mike@turtlerockstudios.com), January 2003

#pragma warning( disable : 4530 )					// STL uses exceptions, but we are not compiling with them - ignore warning
#pragma warning( disable : 4786 )					// long STL names get truncated in browse info.

#include <list>
#include <vector>
#include <algorithm>

#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>

#ifdef _WIN32
#include <io.h>

#else
#include <unistd.h>
#define _write write
#define _close close
#define MAX_OSPATH PATH_MAX
#endif

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "bot_util.h"

/// @todo Abstract hostages and cs-bots out of here
#include "cs_bot.h"
#include "cs_bot_manager.h"
#include "hostage.h"

#include "nav.h"
#include "nav_node.h"
#include "nav_area.h"

#include "pm_shared.h" // for OBS_ROAMING

extern void HintMessageToAllPlayers( const char *message );

unsigned int CNavArea::m_nextID = 1;
NavAreaList TheNavAreaList;

NavLadderList TheNavLadderList;

unsigned int CNavArea::m_masterMarker = 1;
CNavArea *CNavArea::m_openList = NULL;

bool CNavArea::m_isReset = false;
static float lastDrawTimestamp = 0.0f;

//--------------------------------------------------------------------------------------------------------------
/**
 * This list contains all "good-sized" areas used to compute "approach points"
 */
static NavAreaList goodSizedAreaList;

static void buildGoodSizedList( void )
{
	const float minSize = 200.0f;		// 150

	NavAreaList::iterator iter;
	for( iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
	{
		CNavArea *area = *iter;

		// skip the small areas
		const Extent *extent = area->GetExtent();
		if (extent->SizeX() < minSize || extent->SizeY() < minSize)
			continue;

		goodSizedAreaList.push_back( area );
	}
}

//--------------------------------------------------------------------------------------------------------------

HidingSpotList TheHidingSpotList;
unsigned int HidingSpot::m_nextID = 1;
unsigned int HidingSpot::m_masterMarker = 0;

void DestroyHidingSpots( void )
{
	// remove all hiding spot references from the nav areas
	for( NavAreaList::iterator areaIter = TheNavAreaList.begin(); areaIter != TheNavAreaList.end(); ++areaIter )
	{
		CNavArea *area = *areaIter;

		area->m_hidingSpotList.clear();
	}

	HidingSpot::m_nextID = 0;

	// free all the HidingSpots
	for( HidingSpotList::iterator iter = TheHidingSpotList.begin(); iter != TheHidingSpotList.end(); ++iter )
		delete *iter;

	TheHidingSpotList.clear();
}

/**
 * For use when loading from a file
 */
HidingSpot::HidingSpot( void )
{
	m_pos = Vector( 0, 0, 0 );
	m_id = 0;
	m_flags = 0;

	TheHidingSpotList.push_back( this );
}

/**
 * For use when generating - assigns unique ID
 */
HidingSpot::HidingSpot( const Vector *pos, unsigned char flags )
{
	m_pos = *pos;
	m_id = m_nextID++;
	m_flags = flags;

	TheHidingSpotList.push_back( this );
}

void HidingSpot::Save( int fd, unsigned int version ) const
{
	_write( fd, &m_id, sizeof(unsigned int) );
	_write( fd, &m_pos, 3 * sizeof(float) );
	_write( fd, &m_flags, sizeof(unsigned char) );
}

void HidingSpot::Load( SteamFile *file, unsigned int version )
{
	file->Read( &m_id, sizeof(unsigned int) );
	file->Read( &m_pos, 3 * sizeof(float) );
	file->Read( &m_flags, sizeof(unsigned char) );

	// update next ID to avoid ID collisions by later spots
	if (m_id >= m_nextID)
		m_nextID = m_id+1;
}

/**
 * Given a HidingSpot ID, return the associated HidingSpot
 */
HidingSpot *GetHidingSpotByID( unsigned int id )
{
	for( HidingSpotList::iterator iter = TheHidingSpotList.begin(); iter != TheHidingSpotList.end(); ++iter )
	{
		HidingSpot *spot = *iter;

		if (spot->GetID() == id)
			return spot;
	}	

	return NULL;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * To keep constructors consistent
 */
void CNavArea::Initialize( void )
{
	m_marker = 0;
	m_parent = NULL;
	m_parentHow = GO_NORTH;
	m_attributeFlags = 0;
	m_place = 0;

	for ( int i=0; i<MAX_AREA_TEAMS; ++i )
	{
		m_danger[i] = 0.0f;
		m_dangerTimestamp[i] = 0.0f;

		m_clearedTimestamp[i] = 0.0f;
	}

	m_approachCount = 0;

	// set an ID for splitting and other interactive editing - loads will overwrite this
	m_id = m_nextID++;

	m_prevHash = NULL;
	m_nextHash = NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Constructor used during normal runtime.
 */
CNavArea::CNavArea( void )
{
	Initialize();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Assumes Z is flat
 */
CNavArea::CNavArea( const Vector *corner, const Vector *otherCorner )
{
	Initialize();

	if (corner->x < otherCorner->x)
	{
		m_extent.lo.x = corner->x;
		m_extent.hi.x = otherCorner->x;
	}
	else
	{
		m_extent.hi.x = corner->x;
		m_extent.lo.x = otherCorner->x;
	}

	if (corner->y < otherCorner->y)
	{
		m_extent.lo.y = corner->y;
		m_extent.hi.y = otherCorner->y;
	}
	else
	{
		m_extent.hi.y = corner->y;
		m_extent.lo.y = otherCorner->y;
	}

	m_extent.lo.z = corner->z;
	m_extent.hi.z = corner->z;

	m_center.x = (m_extent.lo.x + m_extent.hi.x)/2.0f;
	m_center.y = (m_extent.lo.y + m_extent.hi.y)/2.0f;
	m_center.z = (m_extent.lo.z + m_extent.hi.z)/2.0f;

	m_neZ = corner->z;
	m_swZ = otherCorner->z;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * 
 */
CNavArea::CNavArea( const Vector *nwCorner, const Vector *neCorner, const Vector *seCorner, const Vector *swCorner )
{
	Initialize();

	m_extent.lo = *nwCorner;
	m_extent.hi = *seCorner;

	m_center.x = (m_extent.lo.x + m_extent.hi.x)/2.0f;
	m_center.y = (m_extent.lo.y + m_extent.hi.y)/2.0f;
	m_center.z = (m_extent.lo.z + m_extent.hi.z)/2.0f;

	m_neZ = neCorner->z;
	m_swZ = swCorner->z;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Constructor used during generation phase.
 */
CNavArea::CNavArea( CNavNode *nwNode, CNavNode *neNode, CNavNode *seNode, CNavNode *swNode )
{
	Initialize();

	m_extent.lo = *nwNode->GetPosition();
	m_extent.hi = *seNode->GetPosition();

	m_center.x = (m_extent.lo.x + m_extent.hi.x)/2.0f;
	m_center.y = (m_extent.lo.y + m_extent.hi.y)/2.0f;
	m_center.z = (m_extent.lo.z + m_extent.hi.z)/2.0f;

	m_neZ = neNode->GetPosition()->z;
	m_swZ = swNode->GetPosition()->z;

	m_node[ NORTH_WEST ] = nwNode;
	m_node[ NORTH_EAST ] = neNode;
	m_node[ SOUTH_EAST ] = seNode;
	m_node[ SOUTH_WEST ] = swNode;

	// mark internal nodes as part of this area
	AssignNodes( this );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Destructor
 */
CNavArea::~CNavArea()
{
	// if we are resetting the system, don't bother cleaning up - all areas are being destroyed
	if (m_isReset)
		return;

	// tell the other areas we are going away
	NavAreaList::iterator iter;
	for( iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
	{
		CNavArea *area = *iter;

		if (area == this)
			continue;

		area->OnDestroyNotify( this );
	}

	// unhook from ladders
	for( int i=0; i<NUM_LADDER_DIRECTIONS; ++i )
	{
		for( NavLadderList::iterator liter = m_ladder[i].begin(); liter != m_ladder[i].end(); ++liter )
		{
			CNavLadder *ladder = *liter;

			ladder->OnDestroyNotify( this );
		}
	}

	// remove the area from the grid
	TheNavAreaGrid.RemoveNavArea( this );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * This is invoked when an area is going away.
 * Remove any references we have to it.
 */
void CNavArea::OnDestroyNotify( CNavArea *dead )
{
	NavConnect con;
	con.area = dead;
	for( int d=0; d<NUM_DIRECTIONS; ++d )
		m_connect[ d ].remove( con );

	m_overlapList.remove( dead );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Connect this area to given area in given direction
 */
void CNavArea::ConnectTo( CNavArea *area, NavDirType dir )
{
	// check if already connected
	for( NavConnectList::iterator iter = m_connect[ dir ].begin(); iter != m_connect[ dir ].end(); ++iter )
		if ((*iter).area == area)
			return;

	NavConnect con;
	con.area = area;
	m_connect[ dir ].push_back( con );

	//static char *dirName[] = { "NORTH", "EAST", "SOUTH", "WEST" };
	//CONSOLE_ECHO( "  Connected area #%d to #%d, %s\n", m_id, area->m_id, dirName[ dir ] );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Disconnect this area from given area
 */
void CNavArea::Disconnect( CNavArea *area )
{
	NavConnect connect;
	connect.area = area;

	for( int dir = 0; dir<NUM_DIRECTIONS; dir++ )
		m_connect[ dir ].remove( connect );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Recompute internal data once nodes have been adjusted during merge
 * Destroy adjArea.
 */
void CNavArea::FinishMerge( CNavArea *adjArea )
{
	// update extent
	m_extent.lo = *m_node[ NORTH_WEST ]->GetPosition();
	m_extent.hi = *m_node[ SOUTH_EAST ]->GetPosition();

	m_center.x = (m_extent.lo.x + m_extent.hi.x)/2.0f;
	m_center.y = (m_extent.lo.y + m_extent.hi.y)/2.0f;
	m_center.z = (m_extent.lo.z + m_extent.hi.z)/2.0f;

	m_neZ = m_node[ NORTH_EAST ]->GetPosition()->z;
	m_swZ = m_node[ SOUTH_WEST ]->GetPosition()->z;

	// reassign the adjacent area's internal nodes to the final area
	adjArea->AssignNodes( this );

	// merge adjacency links - we gain all the connections that adjArea had
	MergeAdjacentConnections( adjArea );

	// remove subsumed adjacent area
	TheNavAreaList.remove( adjArea );
	delete adjArea;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * For merging with "adjArea" - pick up all of "adjArea"s connections
 */
void CNavArea::MergeAdjacentConnections( CNavArea *adjArea )
{
	// merge adjacency links - we gain all the connections that adjArea had
	NavConnectList::iterator iter;
	int dir;
	for( dir = 0; dir<NUM_DIRECTIONS; dir++ )
	{
		for( iter = adjArea->m_connect[ dir ].begin(); iter != adjArea->m_connect[ dir ].end(); ++iter )
		{
			NavConnect connect = *iter;

			if (connect.area != adjArea && connect.area != this)
				ConnectTo( connect.area, (NavDirType)dir );
		}
	}

	// remove any references from this area to the adjacent area, since it is now part of us
	for( dir = 0; dir<NUM_DIRECTIONS; dir++ )
	{
		NavConnect connect;
		connect.area = adjArea;

		m_connect[dir].remove( connect );
	}

	// Change other references to adjArea to refer instead to us
	// We can't just replace existing connections, as several adjacent areas may have been merged into one,
	// resulting in a large area adjacent to all of them ending up with multiple redunandant connections
	// into the merged area, one for each of the adjacent subsumed smaller ones.
	// If an area has a connection to the merged area, we must remove all references to adjArea, and add
	// a single connection to us.
	for( NavAreaList::iterator areaIter = TheNavAreaList.begin(); areaIter != TheNavAreaList.end(); ++areaIter )
	{
		CNavArea *area = *areaIter;

		if (area == this || area == adjArea)
			continue;

		for( dir = 0; dir<NUM_DIRECTIONS; dir++ )
		{
			// check if there are any references to adjArea in this direction
			bool connected = false;
			for( iter = area->m_connect[ dir ].begin(); iter != area->m_connect[ dir ].end(); ++iter )
			{
				NavConnect connect = *iter;

				if (connect.area == adjArea)
				{
					connected = true;
					break;
				}
			}

			if (connected)
			{
				// remove all references to adjArea
				NavConnect connect;
				connect.area = adjArea;
				area->m_connect[dir].remove( connect );

				// remove all references to the new area
				connect.area = this;
				area->m_connect[dir].remove( connect );

				// add a single connection to the new area
				connect.area = this;
				area->m_connect[dir].push_back( connect );
			}
		}
	}

}

//--------------------------------------------------------------------------------------------------------------
/**
 * Assign internal nodes to the given area
 * NOTE: "internal" nodes do not include the east or south border nodes
 */
void CNavArea::AssignNodes( CNavArea *area )
{
	CNavNode *horizLast = m_node[ NORTH_EAST ];

	for( CNavNode *vertNode = m_node[ NORTH_WEST ]; vertNode != m_node[ SOUTH_WEST ]; vertNode = vertNode->GetConnectedNode( SOUTH ) )
	{
		for( CNavNode *horizNode = vertNode; horizNode != horizLast; horizNode = horizNode->GetConnectedNode( EAST ) )
		{
			horizNode->AssignArea( area );
		}

		horizLast = horizLast->GetConnectedNode( SOUTH );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Split this area into two areas at the given edge.
 * Preserve all adjacency connections.
 * NOTE: This does not update node connections, only areas.
 */
bool CNavArea::SplitEdit( bool splitAlongX, float splitEdge, CNavArea **outAlpha, CNavArea **outBeta )
{
	CNavArea *alpha = NULL;
	CNavArea *beta = NULL;

	if (splitAlongX)
	{
		// +-----+->X
		// |  A  |
		// +-----+
		// |  B  |
		// +-----+
		// |
		// Y

		// don't do split if at edge of area
		if (splitEdge <= m_extent.lo.y + 1.0f)
			return false;

		if (splitEdge >= m_extent.hi.y - 1.0f)
			return false;

		alpha = new CNavArea;
		alpha->m_extent.lo = m_extent.lo;

		alpha->m_extent.hi.x = m_extent.hi.x;
		alpha->m_extent.hi.y = splitEdge;
		alpha->m_extent.hi.z = GetZ( &alpha->m_extent.hi );

		beta = new CNavArea;
		beta->m_extent.lo.x = m_extent.lo.x;
		beta->m_extent.lo.y = splitEdge;
		beta->m_extent.lo.z = GetZ( &beta->m_extent.lo );

		beta->m_extent.hi = m_extent.hi;

		alpha->ConnectTo( beta, SOUTH );
		beta->ConnectTo( alpha, NORTH );

		FinishSplitEdit( alpha, SOUTH );
		FinishSplitEdit( beta, NORTH );
	}
	else
	{
		// +--+--+->X
		// |  |  |
		// | A|B |
		// |  |  |
		// +--+--+
		// |
		// Y

		// don't do split if at edge of area
		if (splitEdge <= m_extent.lo.x + 1.0f)
			return false;

		if (splitEdge >= m_extent.hi.x - 1.0f)
			return false;

		alpha = new CNavArea;
		alpha->m_extent.lo = m_extent.lo;

		alpha->m_extent.hi.x = splitEdge;
		alpha->m_extent.hi.y = m_extent.hi.y;
		alpha->m_extent.hi.z = GetZ( &alpha->m_extent.hi );

		beta = new CNavArea;
		beta->m_extent.lo.x = splitEdge;
		beta->m_extent.lo.y = m_extent.lo.y;
		beta->m_extent.lo.z = GetZ( &beta->m_extent.lo );

		beta->m_extent.hi = m_extent.hi;

		alpha->ConnectTo( beta, EAST );
		beta->ConnectTo( alpha, WEST );

		FinishSplitEdit( alpha, EAST );
		FinishSplitEdit( beta, WEST );
	}

	// new areas inherit attributes from original area
	alpha->SetAttributes( GetAttributes() );
	beta->SetAttributes( GetAttributes() );

	// new areas inherit place from original area
	alpha->SetPlace( GetPlace() );
	beta->SetPlace( GetPlace() );

	// return new areas
	if (outAlpha)
		*outAlpha = alpha;

	if (outBeta)
		*outBeta = beta;

	// remove original area
	TheNavAreaList.remove( this );
	delete this;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if given area is connected in given direction
 * if dir == NUM_DIRECTIONS, check all directions (direction is unknown)
 * @todo Formalize "asymmetric" flag on connections
 */
bool CNavArea::IsConnected( const CNavArea *area, NavDirType dir ) const
{
	// we are connected to ourself
	if (area == this)
		return true;

	NavConnectList::const_iterator iter;

	if (dir == NUM_DIRECTIONS)
	{
		// search all directions
		for( int d=0; d<NUM_DIRECTIONS; ++d )
		{
			for( iter = m_connect[ d ].begin(); iter != m_connect[ d ].end(); ++iter )
			{
				if (area == (*iter).area)
					return true;
			}
		}

		// check ladder connections
		NavLadderList::const_iterator liter;
		for( liter = m_ladder[ LADDER_UP ].begin(); liter != m_ladder[ LADDER_UP ].end(); ++liter )
		{
			CNavLadder *ladder = *liter;

			if (ladder->m_topBehindArea == area ||
					ladder->m_topForwardArea == area ||
					ladder->m_topLeftArea == area ||
					ladder->m_topRightArea == area)
				return true;
		}

		for( liter = m_ladder[ LADDER_DOWN ].begin(); liter != m_ladder[ LADDER_DOWN ].end(); ++liter )
		{
			CNavLadder *ladder = *liter;

			if (ladder->m_bottomArea == area)
				return true;
		}
	}
	else
	{
		// check specific direction
		for( iter = m_connect[ dir ].begin(); iter != m_connect[ dir ].end(); ++iter )
		{
			if (area == (*iter).area)
				return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Compute change in height from this area to given area
 * @todo This is approximate for now
 */
float CNavArea::ComputeHeightChange( const CNavArea *area )
{
	float ourZ = GetZ( GetCenter() );
	float areaZ = area->GetZ( area->GetCenter() );

	return areaZ - ourZ;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Given the portion of the original area, update its internal data
 * The "ignoreEdge" direction defines the side of the original area that the new area does not include
 */
void CNavArea::FinishSplitEdit( CNavArea *newArea, NavDirType ignoreEdge )
{
	newArea->m_center.x = (newArea->m_extent.lo.x + newArea->m_extent.hi.x)/2.0f;
	newArea->m_center.y = (newArea->m_extent.lo.y + newArea->m_extent.hi.y)/2.0f;
	newArea->m_center.z = (newArea->m_extent.lo.z + newArea->m_extent.hi.z)/2.0f;

	newArea->m_neZ = GetZ( newArea->m_extent.hi.x, newArea->m_extent.lo.y );
	newArea->m_swZ = GetZ( newArea->m_extent.lo.x, newArea->m_extent.hi.y );

	// connect to adjacent areas
	for( int d=0; d<NUM_DIRECTIONS; ++d )
	{
		if (d == ignoreEdge)
			continue;

		int count = GetAdjacentCount( (NavDirType)d );

		for( int a=0; a<count; ++a )
		{
			CNavArea *adj = GetAdjacentArea( (NavDirType)d, a );

			switch( d )
			{
				case NORTH:
				case SOUTH:
					if (newArea->IsOverlappingX( adj ))
					{
						newArea->ConnectTo( adj, (NavDirType)d );			

						// add reciprocal connection if needed
						if (adj->IsConnected( this, OppositeDirection( (NavDirType)d )))
							adj->ConnectTo( newArea, OppositeDirection( (NavDirType)d ) );
					}
					break;

				case EAST:
				case WEST:
					if (newArea->IsOverlappingY( adj ))
					{
						newArea->ConnectTo( adj, (NavDirType)d );			

						// add reciprocal connection if needed
						if (adj->IsConnected( this, OppositeDirection( (NavDirType)d )))
							adj->ConnectTo( newArea, OppositeDirection( (NavDirType)d ) );
					}
					break;
			}
		}
	}

	TheNavAreaList.push_back( newArea );
	TheNavAreaGrid.AddNavArea( newArea );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Create a new area between this area and given area 
 */
bool CNavArea::SpliceEdit( CNavArea *other )
{
	CNavArea *newArea = NULL;
	Vector nw, ne, se, sw;

	if (m_extent.lo.x > other->m_extent.hi.x)
	{
		// 'this' is east of 'other'
		float top = max( m_extent.lo.y, other->m_extent.lo.y );
		float bottom = min( m_extent.hi.y, other->m_extent.hi.y );

		nw.x = other->m_extent.hi.x;
		nw.y = top;
		nw.z = other->GetZ( &nw );

		se.x = m_extent.lo.x;
		se.y = bottom;
		se.z = GetZ( &se );

		ne.x = se.x;
		ne.y = nw.y;
		ne.z = GetZ( &ne );

		sw.x = nw.x;
		sw.y = se.y;
		sw.z = other->GetZ( &sw );

		newArea = new CNavArea( &nw, &ne, &se, &sw );

		this->ConnectTo( newArea, WEST );
		newArea->ConnectTo( this, EAST );

		other->ConnectTo( newArea, EAST );
		newArea->ConnectTo( other, WEST );
	}
	else if (m_extent.hi.x < other->m_extent.lo.x)
	{
		// 'this' is west of 'other'
		float top = max( m_extent.lo.y, other->m_extent.lo.y );
		float bottom = min( m_extent.hi.y, other->m_extent.hi.y );

		nw.x = m_extent.hi.x;
		nw.y = top;
		nw.z = GetZ( &nw );

		se.x = other->m_extent.lo.x;
		se.y = bottom;
		se.z = other->GetZ( &se );

		ne.x = se.x;
		ne.y = nw.y;
		ne.z = other->GetZ( &ne );

		sw.x = nw.x;
		sw.y = se.y;
		sw.z = GetZ( &sw );

		newArea = new CNavArea( &nw, &ne, &se, &sw );

		this->ConnectTo( newArea, EAST );
		newArea->ConnectTo( this, WEST );

		other->ConnectTo( newArea, WEST );
		newArea->ConnectTo( other, EAST );
	}
	else	// 'this' overlaps in X
	{
		if (m_extent.lo.y > other->m_extent.hi.y)
		{
			// 'this' is south of 'other'
			float left = max( m_extent.lo.x, other->m_extent.lo.x );
			float right = min( m_extent.hi.x, other->m_extent.hi.x );

			nw.x = left;
			nw.y = other->m_extent.hi.y;
			nw.z = other->GetZ( &nw );

			se.x = right;
			se.y = m_extent.lo.y;
			se.z = GetZ( &se );

			ne.x = se.x;
			ne.y = nw.y;
			ne.z = other->GetZ( &ne );

			sw.x = nw.x;
			sw.y = se.y;
			sw.z = GetZ( &sw );

			newArea = new CNavArea( &nw, &ne, &se, &sw );

			this->ConnectTo( newArea, NORTH );
			newArea->ConnectTo( this, SOUTH );

			other->ConnectTo( newArea, SOUTH );
			newArea->ConnectTo( other, NORTH );
		}
		else if (m_extent.hi.y < other->m_extent.lo.y)
		{
			// 'this' is north of 'other'
			float left = max( m_extent.lo.x, other->m_extent.lo.x );
			float right = min( m_extent.hi.x, other->m_extent.hi.x );

			nw.x = left;
			nw.y = m_extent.hi.y;
			nw.z = GetZ( &nw );

			se.x = right;
			se.y = other->m_extent.lo.y;
			se.z = other->GetZ( &se );

			ne.x = se.x;
			ne.y = nw.y;
			ne.z = GetZ( &ne );

			sw.x = nw.x;
			sw.y = se.y;
			sw.z = other->GetZ( &sw );

			newArea = new CNavArea( &nw, &ne, &se, &sw );

			this->ConnectTo( newArea, SOUTH );
			newArea->ConnectTo( this, NORTH );

			other->ConnectTo( newArea, NORTH );
			newArea->ConnectTo( other, SOUTH );
		}
		else
		{
			// areas overlap
			return false;
		}
	}

	// if both areas have the same place, the new area inherits it
	if (GetPlace() == other->GetPlace())
	{
		newArea->SetPlace( GetPlace() );
	}
	else if (GetPlace() == UNDEFINED_PLACE)
	{
		newArea->SetPlace( other->GetPlace() );
	}
	else if (other->GetPlace() == UNDEFINED_PLACE)
	{
		newArea->SetPlace( GetPlace() );
	}
	else
	{
		// both have valid, but different places - pick on at random
		if (RANDOM_LONG( 0, 100 ) < 50)
			newArea->SetPlace( GetPlace() );
		else
			newArea->SetPlace( other->GetPlace() );
	}

	TheNavAreaList.push_back( newArea );
	TheNavAreaGrid.AddNavArea( newArea );

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Merge this area and given adjacent area 
 */
bool CNavArea::MergeEdit( CNavArea *adj )
{
	// can only merge if attributes of both areas match


	// check that these areas can be merged
	const float tolerance = 1.0f;
	bool merge = false;
	if (ABS( m_extent.lo.x - adj->m_extent.lo.x ) < tolerance && 
			ABS( m_extent.hi.x - adj->m_extent.hi.x ) < tolerance)
		merge = true;

	if (ABS( m_extent.lo.y - adj->m_extent.lo.y ) < tolerance && 
			ABS( m_extent.hi.y - adj->m_extent.hi.y ) < tolerance)
		merge = true;

	if (merge == false)
		return false;

	Extent origExtent = m_extent;
	
	// update extent
	if (m_extent.lo.x > adj->m_extent.lo.x || m_extent.lo.y > adj->m_extent.lo.y)
		m_extent.lo = adj->m_extent.lo;

	if (m_extent.hi.x < adj->m_extent.hi.x || m_extent.hi.y < adj->m_extent.hi.y)
		m_extent.hi = adj->m_extent.hi;

	m_center.x = (m_extent.lo.x + m_extent.hi.x)/2.0f;
	m_center.y = (m_extent.lo.y + m_extent.hi.y)/2.0f;
	m_center.z = (m_extent.lo.z + m_extent.hi.z)/2.0f;

	if (m_extent.hi.x > origExtent.hi.x || m_extent.lo.y < origExtent.lo.y)
		m_neZ = adj->GetZ( m_extent.hi.x, m_extent.lo.y );
	else
		m_neZ = GetZ( m_extent.hi.x, m_extent.lo.y );

	if (m_extent.lo.x < origExtent.lo.x || m_extent.hi.y > origExtent.hi.y)
		m_swZ = adj->GetZ( m_extent.lo.x, m_extent.hi.y );
	else
		m_swZ = GetZ( m_extent.lo.x, m_extent.hi.y );

	// merge adjacency links - we gain all the connections that adjArea had
	MergeAdjacentConnections( adj );

	// remove subsumed adjacent area
	TheNavAreaList.remove( adj );
	delete adj;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
void ApproachAreaAnalysisPrep( void )
{
	// collect "good-sized" areas for computing approach areas
	buildGoodSizedList();
}

//--------------------------------------------------------------------------------------------------------------
void CleanupApproachAreaAnalysisPrep( void )
{
	goodSizedAreaList.clear();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Destroy ladder representations
 */
void DestroyLadders( void )
{
	while( !TheNavLadderList.empty() )
	{
		CNavLadder *ladder = TheNavLadderList.front();
		TheNavLadderList.pop_front();
		delete ladder;
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Free navigation map data.
 */
void DestroyNavigationMap( void )
{
	CNavArea::m_isReset = true;

	// remove each element of the list and delete them
	while( !TheNavAreaList.empty() )
	{
		CNavArea *area = TheNavAreaList.front();
		TheNavAreaList.pop_front();
		delete area;
	}

	CNavArea::m_isReset = false;

	// destroy ladder representations
	DestroyLadders();

	// destroy all hiding spots
	DestroyHidingSpots();

	// destroy navigation nodes created during map learning
	CNavNode *node, *next;
	for( node = CNavNode::m_list; node; node = next )
	{
		next = node->m_next;
		delete node;
	}
	CNavNode::m_list = NULL;

	// reset the grid
	TheNavAreaGrid.Reset();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Strip the "analyzed" data out of all navigation areas
 */
void StripNavigationAreas( void )
{
	NavAreaList::iterator iter;
	for( iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
	{
		CNavArea *area = *iter;

		area->Strip();
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Remove "analyzed" data from nav area
 */
void CNavArea::Strip( void )
{
	m_approachCount = 0;
	m_spotEncounterList.clear();		// memory leak	
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Start at given position and find first area in given direction
 */
inline CNavArea *FindFirstAreaInDirection( const Vector *start, NavDirType dir, float range, float beneathLimit, CBaseEntity *traceIgnore = NULL, Vector *closePos = NULL )
{
	CNavArea *area = NULL;

	Vector pos = *start;

	int end = (int)((range / GenerationStepSize) + 0.5f);

	for( int i=1; i<=end; i++ )
	{
		AddDirectionVector( &pos, dir, GenerationStepSize );

		// make sure we dont look thru the wall
		TraceResult result;

		if (traceIgnore)
			UTIL_TraceLine( *start, pos, ignore_monsters, ENT( traceIgnore->pev ), &result );
		else
			UTIL_TraceLine( *start, pos, ignore_monsters, NULL, &result );

		if (result.flFraction != 1.0f)
			break;

		area = TheNavAreaGrid.GetNavArea( &pos, beneathLimit );
		if (area)
		{
			if (closePos)
			{
				closePos->x = pos.x;
				closePos->y = pos.y;
				closePos->z = area->GetZ( pos.x, pos.y );
			}

			break;
		}
	}

	return area;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Determine if we can "jump down" from given point
 */
inline bool testJumpDown( const Vector *fromPos, const Vector *toPos )
{
	float dz = fromPos->z - toPos->z;

	// drop can't be too far, or too short (or nonexistant)
	if (dz <= JumpCrouchHeight || dz >= DeathDrop)
		return false;

	//
	// Check LOS out and down
	//
	// ------+
	//       |
	// F     |
	//       |
	//       T 
	//

	Vector from( fromPos->x, fromPos->y, fromPos->z + HumanHeight );
	Vector to( toPos->x, toPos->y, from.z );

	TraceResult result;
	UTIL_TraceLine( from, to, ignore_monsters, NULL, &result );
	if (result.flFraction != 1.0f || result.fStartSolid)
		return false;

	from = to;
	to.z = toPos->z + 2.0f;
	UTIL_TraceLine( from, to, ignore_monsters, NULL, &result );
	if (result.flFraction != 1.0f || result.fStartSolid)
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
inline CNavArea *findJumpDownArea( const Vector *fromPos, NavDirType dir )
{
	Vector start( fromPos->x, fromPos->y, fromPos->z + HalfHumanHeight );
	AddDirectionVector( &start, dir, GenerationStepSize/2.0f );

	Vector toPos;
	CNavArea *downArea = FindFirstAreaInDirection( &start, dir, 4.0f * GenerationStepSize, DeathDrop, NULL, &toPos );

	if (downArea && testJumpDown( fromPos, &toPos ))
		return downArea;

	return NULL;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Define connections between adjacent generated areas
 */
void ConnectGeneratedAreas( void )
{
	CONSOLE_ECHO( "  Connecting navigation areas...\n" );

	for( NavAreaList::iterator iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
	{
		CNavArea *area = *iter;

		// scan along edge nodes, stepping one node over into the next area
		// for now, only use bi-directional connections

		// north edge
		CNavNode *node;
		for( node = area->m_node[ NORTH_WEST ]; node != area->m_node[ NORTH_EAST ]; node = node->GetConnectedNode( EAST ) )
		{
			CNavNode *adj = node->GetConnectedNode( NORTH );

			if (adj && adj->GetArea() && adj->GetConnectedNode( SOUTH ) == node)
			{
				area->ConnectTo( adj->GetArea(), NORTH );
			}
			else
			{
				CNavArea *downArea = findJumpDownArea( node->GetPosition(), NORTH );
				if (downArea && downArea != area)
					area->ConnectTo( downArea, NORTH );
			}
		}

		// west edge
		for( node = area->m_node[ NORTH_WEST ]; node != area->m_node[ SOUTH_WEST ]; node = node->GetConnectedNode( SOUTH ) )
		{
			CNavNode *adj = node->GetConnectedNode( WEST );
			
			if (adj && adj->GetArea() && adj->GetConnectedNode( EAST ) == node)
			{
				area->ConnectTo( adj->GetArea(), WEST );
			}
			else
			{
				CNavArea *downArea = findJumpDownArea( node->GetPosition(), WEST );
				if (downArea && downArea != area)
					area->ConnectTo( downArea, WEST );
			}
		}

		// south edge - this edge's nodes are actually part of adjacent areas
		// move one node north, and scan west to east
		/// @todo This allows one-node-wide areas - do we want this?
		node = area->m_node[ SOUTH_WEST ];
		node = node->GetConnectedNode( NORTH );
		if (node)
		{
			CNavNode *end = area->m_node[ SOUTH_EAST ]->GetConnectedNode( NORTH );
			/// @todo Figure out why cs_backalley gets a NULL node in here...
			for( ; node && node != end; node = node->GetConnectedNode( EAST ) )
			{
				CNavNode *adj = node->GetConnectedNode( SOUTH );
				
				if (adj && adj->GetArea() && adj->GetConnectedNode( NORTH ) == node)
				{
					area->ConnectTo( adj->GetArea(), SOUTH );
				}
				else
				{
					CNavArea *downArea = findJumpDownArea( node->GetPosition(), SOUTH );
					if (downArea && downArea != area)
						area->ConnectTo( downArea, SOUTH );
				}
			}
		}

		// east edge - this edge's nodes are actually part of adjacent areas
		node = area->m_node[ NORTH_EAST ];
		node = node->GetConnectedNode( WEST );
		if (node)
		{
			CNavNode *end = area->m_node[ SOUTH_EAST ]->GetConnectedNode( WEST );
			for( ; node && node != end; node = node->GetConnectedNode( SOUTH ) )
			{
				CNavNode *adj = node->GetConnectedNode( EAST );
				
				if (adj && adj->GetArea() && adj->GetConnectedNode( WEST ) == node)
				{
					area->ConnectTo( adj->GetArea(), EAST );
				}
				else
				{
					CNavArea *downArea = findJumpDownArea( node->GetPosition(), EAST );
					if (downArea && downArea != area)
						area->ConnectTo( downArea, EAST );
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Merge areas together to make larger ones (must remain rectangular - convex).
 * Areas can only be merged if their attributes match.
 */
void MergeGeneratedAreas( void )
{
	CONSOLE_ECHO( "  Merging navigation areas...\n" );

	bool merged;

	do
	{
		merged = false;

		for( NavAreaList::iterator iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
		{
			CNavArea *area = *iter;

			// north edge
			NavConnectList::iterator citer;
			for( citer = area->m_connect[ NORTH ].begin(); citer != area->m_connect[ NORTH ].end(); ++citer )
			{
				CNavArea *adjArea = (*citer).area;

				if (area->m_node[ NORTH_WEST ] == adjArea->m_node[ SOUTH_WEST ] &&
						area->m_node[ NORTH_EAST ] == adjArea->m_node[ SOUTH_EAST ] &&
						area->GetAttributes() == adjArea->GetAttributes() &&
						area->IsCoplanar( adjArea ))
				{
					// merge vertical
					area->m_node[ NORTH_WEST ] = adjArea->m_node[ NORTH_WEST ];
					area->m_node[ NORTH_EAST ] = adjArea->m_node[ NORTH_EAST ];

					merged = true;
					//CONSOLE_ECHO( "  Merged (north) areas #%d and #%d\n", area->m_id, adjArea->m_id );

					area->FinishMerge( adjArea );

					// restart scan - iterator is invalidated
					break;
				}
			}

			if (merged)
				break;

			// south edge
			for( citer = area->m_connect[ SOUTH ].begin(); citer != area->m_connect[ SOUTH ].end(); ++citer )
			{
				CNavArea *adjArea = (*citer).area;

				if (adjArea->m_node[ NORTH_WEST ] == area->m_node[ SOUTH_WEST ] &&
						adjArea->m_node[ NORTH_EAST ] == area->m_node[ SOUTH_EAST ] &&
						area->GetAttributes() == adjArea->GetAttributes() &&
						area->IsCoplanar( adjArea ))
				{
					// merge vertical
					area->m_node[ SOUTH_WEST ] = adjArea->m_node[ SOUTH_WEST ];
					area->m_node[ SOUTH_EAST ] = adjArea->m_node[ SOUTH_EAST ];

					merged = true;
					//CONSOLE_ECHO( "  Merged (south) areas #%d and #%d\n", area->m_id, adjArea->m_id );

					area->FinishMerge( adjArea );

					// restart scan - iterator is invalidated
					break;
				}

			}

			if (merged)
				break;


			// west edge
			for( citer = area->m_connect[ WEST ].begin(); citer != area->m_connect[ WEST ].end(); ++citer )
			{
				CNavArea *adjArea = (*citer).area;

				if (area->m_node[ NORTH_WEST ] == adjArea->m_node[ NORTH_EAST ] &&
						area->m_node[ SOUTH_WEST ] == adjArea->m_node[ SOUTH_EAST ] &&
						area->GetAttributes() == adjArea->GetAttributes() &&
						area->IsCoplanar( adjArea ))
				{
					// merge horizontal
					area->m_node[ NORTH_WEST ] = adjArea->m_node[ NORTH_WEST ];
					area->m_node[ SOUTH_WEST ] = adjArea->m_node[ SOUTH_WEST ];

					merged = true;
					//CONSOLE_ECHO( "  Merged (west) areas #%d and #%d\n", area->m_id, adjArea->m_id );

					area->FinishMerge( adjArea );

					// restart scan - iterator is invalidated
					break;
				}

			}

			if (merged)
				break;

			// east edge
			for( citer = area->m_connect[ EAST ].begin(); citer != area->m_connect[ EAST ].end(); ++citer )
			{
				CNavArea *adjArea = (*citer).area;
				
				if (adjArea->m_node[ NORTH_WEST ] == area->m_node[ NORTH_EAST ] &&
						adjArea->m_node[ SOUTH_WEST ] == area->m_node[ SOUTH_EAST ] &&
						area->GetAttributes() == adjArea->GetAttributes() &&
						area->IsCoplanar( adjArea ))
				{
					// merge horizontal
					area->m_node[ NORTH_EAST ] = adjArea->m_node[ NORTH_EAST ];
					area->m_node[ SOUTH_EAST ] = adjArea->m_node[ SOUTH_EAST ];

					merged = true;
					//CONSOLE_ECHO( "  Merged (east) areas #%d and #%d\n", area->m_id, adjArea->m_id );

					area->FinishMerge( adjArea );

					// restart scan - iterator is invalidated
					break;
				}
			}

			if (merged)
				break;
		}
	}
	while( merged );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if area is more or less square.
 * This is used when merging to prevent long, thin, areas being created.
 */
inline bool IsAreaRoughlySquare( const CNavArea *area )
{
	float aspect = area->GetSizeX() / area->GetSizeY();

	const float maxAspect = 3.01;
	const float minAspect = 1.0f / maxAspect;
	if (aspect < minAspect || aspect > maxAspect)
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Recursively chop area in half along X until child areas are roughly square
 */
void SplitX( CNavArea *area )
{
	if (IsAreaRoughlySquare( area ))
		return;

	float split = area->GetSizeX();
	split /= 2.0f;
	split += area->GetExtent()->lo.x;

	SnapToGrid( &split );

	const float epsilon = 0.1f;
	if (abs(split - area->GetExtent()->lo.x) < epsilon ||
			abs(split - area->GetExtent()->hi.x) < epsilon)
	{
		// too small to subdivide
		return;
	}

	CNavArea *alpha, *beta;
	if (area->SplitEdit( false, split, &alpha, &beta ))
	{
		// split each new area until square
		SplitX( alpha );
		SplitX( beta );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Recursively chop area in half along Y until child areas are roughly square
 */
void SplitY( CNavArea *area )
{
	if (IsAreaRoughlySquare( area ))
		return;

	float split = area->GetSizeY();
	split /= 2.0f;
	split += area->GetExtent()->lo.y;

	SnapToGrid( &split );

	const float epsilon = 0.1f;
	if (abs(split - area->GetExtent()->lo.y) < epsilon ||
			abs(split - area->GetExtent()->hi.y) < epsilon)
	{
		// too small to subdivide
		return;
	}

	CNavArea *alpha, *beta;
	if (area->SplitEdit( true, split, &alpha, &beta ))
	{
		// split each new area until square
		SplitY( alpha );
		SplitY( beta );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Split any long, thin, areas into roughly square chunks.
 */
void SquareUpAreas( void )
{
	NavAreaList::iterator iter = TheNavAreaList.begin();

	while( iter != TheNavAreaList.end() )
	{
		CNavArea *area = *iter;
		++iter;

		if (!IsAreaRoughlySquare( area ))
		{
			// chop this area into square pieces
			if (area->GetSizeX() > area->GetSizeY())
				SplitX( area );
			else
				SplitY( area );
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/** 
 * Check if an rectangular area of the given size can be
 * made starting from the given node as the NW corner.
 * Only consider fully connected nodes for this check.
 * All of the nodes within the test area must have the same attributes.
 * All of the nodes must be approximately co-planar w.r.t the NW node's normal, with the
 * exception of 1x1 areas which can be any angle.
 */
bool TestArea( CNavNode *node, int width, int height )
{
	Vector normal = *node->GetNormal();
	float d = -DotProduct( normal, *node->GetPosition() );

	const float offPlaneTolerance = 5.0f;

	CNavNode *vertNode, *horizNode;

	vertNode = node;
	for( int y=0; y<height; y++ )
	{
		horizNode = vertNode;

		for( int x=0; x<width; x++ )
		{
			// all nodes must have the same attributes
			if (horizNode->GetAttributes() != node->GetAttributes())
				return false;

			if (horizNode->IsCovered())
				return false;

			if (!horizNode->IsClosedCell())
				return false;

			horizNode = horizNode->GetConnectedNode( EAST );
			if (horizNode == NULL)
				return false;

			// nodes must lie on/near the plane
			if (width > 1 || height > 1)
			{
				float dist = abs(DotProduct( *horizNode->GetPosition(), normal ) + d);
				if (dist > offPlaneTolerance)
					return false;
			}					
		}

		vertNode = vertNode->GetConnectedNode( SOUTH );
		if (vertNode == NULL)
			return false;

		// nodes must lie on/near the plane
		if (width > 1 || height > 1)
		{
			float dist = abs(DotProduct( *vertNode->GetPosition(), normal ) + d);
			if (dist > offPlaneTolerance)
				return false;
		}					
	}

	// check planarity of southern edge
	if (width > 1 || height > 1)
	{
		horizNode = vertNode;

		for( int x=0; x<width; x++ )
		{
			horizNode = horizNode->GetConnectedNode( EAST );
			if (horizNode == NULL)
				return false;

			// nodes must lie on/near the plane
			float dist = abs(DotProduct( *horizNode->GetPosition(), normal ) + d);
			if (dist > offPlaneTolerance)
				return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/** 
 * Create a nav area, and mark all nodes it overlaps as "covered"
 * NOTE: Nodes on the east and south edges are not included.
 * Returns number of nodes covered by this area, or -1 for error;
 */
int BuildArea( CNavNode *node, int width, int height )
{
	//CONSOLE_ECHO( "BuildArea( #%d, %d, %d )\n", node->GetID(), width, height );

	CNavNode *nwNode = node;
	CNavNode *neNode = NULL;
	CNavNode *swNode = NULL;
	CNavNode *seNode = NULL;

	CNavNode *vertNode = node;
	CNavNode *horizNode;

	int coveredNodes = 0;

	for( int y=0; y<height; y++ )
	{
		horizNode = vertNode;

		for( int x=0; x<width; x++ )
		{
			horizNode->Cover();
			++coveredNodes;
			
			horizNode = horizNode->GetConnectedNode( EAST );
		}

		if (y == 0)
			neNode = horizNode;

		vertNode = vertNode->GetConnectedNode( SOUTH );
	}

	swNode = vertNode;

	horizNode = vertNode;		
	for( int x=0; x<width; x++ )
	{
		horizNode = horizNode->GetConnectedNode( EAST );
	}
	seNode = horizNode;

	if (!nwNode || !neNode || !seNode || !swNode)
	{
		CONSOLE_ECHO( "ERROR: BuildArea - NULL node.\n" );
		return -1;
	}

	CNavArea *area = new CNavArea( nwNode, neNode, seNode, swNode );
	TheNavAreaList.push_back( area );

	// since all internal nodes have the same attributes, set this area's attributes
	area->SetAttributes( node->GetAttributes() );

//	fprintf( fp, "f %d %d %d %d\n", nwNode->m_id, neNode->m_id, seNode->m_id, swNode->m_id );

	return coveredNodes;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * For each ladder in the map, create a navigation representation of it.
 */
void BuildLadders( void )
{
	// remove any left-over ladders
	DestroyLadders();

	TraceResult result;
	CBaseEntity *entity = UTIL_FindEntityByClassname( NULL, "func_ladder" );
	while( entity && !FNullEnt( entity->edict() ) )
	{
		CNavLadder *ladder = new CNavLadder;

		// compute top & bottom of ladder
		ladder->m_top.x = (entity->pev->absmin.x + entity->pev->absmax.x) / 2.0f;
		ladder->m_top.y = (entity->pev->absmin.y + entity->pev->absmax.y) / 2.0f;
		ladder->m_top.z = entity->pev->absmax.z;

		ladder->m_bottom.x = ladder->m_top.x;
		ladder->m_bottom.y = ladder->m_top.y;
		ladder->m_bottom.z = entity->pev->absmin.z;

		// determine facing - assumes "normal" runged ladder
		float xSize = entity->pev->absmax.x - entity->pev->absmin.x;
		float ySize = entity->pev->absmax.y - entity->pev->absmin.y;
		if (xSize > ySize)
		{
			// ladder is facing north or south - determine which way
			// "pull in" traceline from bottom and top in case ladder abuts floor and/or ceiling
			Vector from = ladder->m_bottom + Vector( 0.0f, GenerationStepSize, GenerationStepSize );
			Vector to = ladder->m_top + Vector( 0.0f, GenerationStepSize, -GenerationStepSize );

			UTIL_TraceLine( from, to, ignore_monsters, ENT( entity->pev ), &result );

			if (result.flFraction != 1.0f || result.fStartSolid)
				ladder->m_dir = NORTH;
			else
				ladder->m_dir = SOUTH;
		}
		else
		{
			// ladder is facing east or west - determine which way
			Vector from = ladder->m_bottom + Vector( GenerationStepSize, 0.0f, GenerationStepSize );
			Vector to = ladder->m_top + Vector( GenerationStepSize, 0.0f, -GenerationStepSize );

			UTIL_TraceLine( from, to, ignore_monsters, ENT( entity->pev ), &result );

			if (result.flFraction != 1.0f || result.fStartSolid)
				ladder->m_dir = WEST;
			else
				ladder->m_dir = EAST;
		}

		// adjust top and bottom of ladder to make sure they are reachable
		// (cs_office has a crate right in front of the base of a ladder)
		Vector along = ladder->m_top - ladder->m_bottom;
		float length = along.NormalizeInPlace();
		Vector on, out;
		const float minLadderClearance = 32.0f;

		// adjust bottom to bypass blockages
		const float inc = 10.0f;
		float t;		
		for( t = 0.0f; t <= length; t += inc )
		{
			on = ladder->m_bottom + t * along;

			out = on;
			AddDirectionVector( &out, ladder->m_dir, minLadderClearance );

			UTIL_TraceLine( on, out, ignore_monsters, ENT( entity->pev ), &result );

			if (result.flFraction == 1.0f && !result.fStartSolid)
			{
				// found viable ladder bottom
				ladder->m_bottom = on;
				break;
			}
		}

		// adjust top to bypass blockages
		for( t = 0.0f; t <= length; t += inc )
		{
			on = ladder->m_top - t * along;

			out = on;
			AddDirectionVector( &out, ladder->m_dir, minLadderClearance );

			UTIL_TraceLine( on, out, ignore_monsters, ENT( entity->pev ), &result );

			if (result.flFraction == 1.0f && !result.fStartSolid)
			{
				// found viable ladder top
				ladder->m_top = on;
				break;
			}
		}

		ladder->m_length = (ladder->m_top - ladder->m_bottom).Length();

		DirectionToVector2D( ladder->m_dir, &ladder->m_dirVector );

		ladder->m_entity = entity;
		const float nearLadderRange = 75.0f;		// 50

		//
		// Find naviagtion area at bottom of ladder
		//

		// get approximate postion of player on ladder
		Vector center = ladder->m_bottom + Vector( 0, 0, GenerationStepSize );
		AddDirectionVector( &center, ladder->m_dir, HalfHumanWidth );

		ladder->m_bottomArea = TheNavAreaGrid.GetNearestNavArea( &center, true );
		if (!ladder->m_bottomArea)
		{
			ALERT( at_console, "ERROR: Unconnected ladder bottom at ( %g, %g, %g )\n", ladder->m_bottom.x, ladder->m_bottom.y, ladder->m_bottom.z );
		}
		else
		{
			// store reference to ladder in the area
			ladder->m_bottomArea->AddLadderUp( ladder );
		}

		//
		// Find adjacent navigation areas at the top of the ladder
		//

		// get approximate postion of player on ladder
		center = ladder->m_top + Vector( 0, 0, GenerationStepSize );
		AddDirectionVector( &center, ladder->m_dir, HalfHumanWidth );

		// find "ahead" area
		ladder->m_topForwardArea = FindFirstAreaInDirection( &center, OppositeDirection( ladder->m_dir ), nearLadderRange, 120.0f, entity );
		if (ladder->m_topForwardArea == ladder->m_bottomArea)
			ladder->m_topForwardArea = NULL;

		// find "left" area
		ladder->m_topLeftArea = FindFirstAreaInDirection( &center, DirectionLeft( ladder->m_dir ), nearLadderRange, 120.0f, entity );
		if (ladder->m_topLeftArea == ladder->m_bottomArea)
			ladder->m_topLeftArea = NULL;

		// find "right" area
		ladder->m_topRightArea = FindFirstAreaInDirection( &center, DirectionRight( ladder->m_dir ), nearLadderRange, 120.0f, entity );
		if (ladder->m_topRightArea == ladder->m_bottomArea)
			ladder->m_topRightArea = NULL;

		// find "behind" area - must look farther, since ladder is against the wall away from this area
		ladder->m_topBehindArea = FindFirstAreaInDirection( &center, ladder->m_dir, 2.0f*nearLadderRange, 120.0f, entity );
		if (ladder->m_topBehindArea == ladder->m_bottomArea)
			ladder->m_topBehindArea = NULL;

		// can't include behind area, since it is not used when going up a ladder
		if (!ladder->m_topForwardArea && !ladder->m_topLeftArea && !ladder->m_topRightArea)
			ALERT( at_console, "ERROR: Unconnected ladder top at ( %g, %g, %g )\n", ladder->m_top.x, ladder->m_top.y, ladder->m_top.z );

		// store reference to ladder in the area(s)
		if (ladder->m_topForwardArea)
			ladder->m_topForwardArea->AddLadderDown( ladder );

		if (ladder->m_topLeftArea)
			ladder->m_topLeftArea->AddLadderDown( ladder );

		if (ladder->m_topRightArea)
			ladder->m_topRightArea->AddLadderDown( ladder );

		if (ladder->m_topBehindArea)
			ladder->m_topBehindArea->AddLadderDown( ladder );

		// adjust top of ladder to highest connected area
		float topZ = -99999.9f;
		bool topAdjusted = false;
		CNavArea *topAreaList[4];
		topAreaList[0] = ladder->m_topForwardArea;
		topAreaList[1] = ladder->m_topLeftArea;
		topAreaList[2] = ladder->m_topRightArea;
		topAreaList[3] = ladder->m_topBehindArea;

		for( int a=0; a<4; ++a )
		{
			CNavArea *topArea = topAreaList[a];
			if (topArea == NULL)
				continue;

			Vector close;
			topArea->GetClosestPointOnArea( &ladder->m_top, &close );
			if (topZ < close.z)
			{
				topZ = close.z;
				topAdjusted = true;
			}
		}

		if (topAdjusted)
			ladder->m_top.z = topZ;

		//
		// Determine whether this ladder is "dangling" or not
		// "Dangling" ladders are too high to go up
		//
		ladder->m_isDangling = false;
		if (ladder->m_bottomArea)
		{
			Vector bottomSpot;
			ladder->m_bottomArea->GetClosestPointOnArea( &ladder->m_bottom, &bottomSpot );
			if (ladder->m_bottom.z - bottomSpot.z > HumanHeight)
				ladder->m_isDangling = true;
		}

		// add ladder to global list
		TheNavLadderList.push_back( ladder );		

		entity = UTIL_FindEntityByClassname( entity, "func_ladder" );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Mark all areas that require a jump to get through them.
 * This currently relies on jump areas having extreme slope.
 */
void MarkJumpAreas( void )
{
	for( NavAreaList::iterator iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
	{
		CNavArea *area = *iter;
		Vector u, v;

		// compute our unit surface normal
		u.x = area->m_extent.hi.x - area->m_extent.lo.x;
		u.y = 0.0f;
		u.z = area->m_neZ - area->m_extent.lo.z;

		v.x = 0.0f;
		v.y = area->m_extent.hi.y - area->m_extent.lo.y;
		v.z = area->m_swZ - area->m_extent.lo.z;

		Vector normal = CrossProduct( u, v );
		normal.NormalizeInPlace();

		if (normal.z < MaxUnitZSlope)
			area->SetAttributes( area->GetAttributes() | NAV_JUMP );
	}
}


//--------------------------------------------------------------------------------------------------------------
/**
 * This function uses the CNavNodes that have been sampled from the map to
 * generate CNavAreas - rectangular areas of "walkable" space. These areas
 * are connected to each other, allowing the AI to know how to move from
 * area to area.
 *
 * This is a "greedy" algorithm that attempts to cover the walkable area 
 * with the fewest, largest, rectangles.
 */
void GenerateNavigationAreaMesh( void )
{
	// haven't yet seen a map use larger than 30...
	int tryWidth = 50;
	int tryHeight = 50;
	int uncoveredNodes = CNavNode::GetListLength();

	while( uncoveredNodes > 0 )
	{
		for( CNavNode *node = CNavNode::GetFirst(); node; node = node->GetNext() )
		{
			if (node->IsCovered())
				continue;

			if (TestArea( node, tryWidth, tryHeight ))
			{
				int covered = BuildArea( node, tryWidth, tryHeight );
				if (covered < 0)
				{
					CONSOLE_ECHO( "GenerateNavigationAreaMesh: Error - Data corrupt.\n" );
					return;
				}

				uncoveredNodes -= covered;
			}
		}

		if (tryWidth >= tryHeight)
			--tryWidth;
		else
			--tryHeight;

		if (tryWidth <= 0 || tryHeight <= 0)
			break;
	}

	Extent extent;
	extent.lo.x = 9999999999.9f;
	extent.lo.y = 9999999999.9f;
	extent.hi.x = -9999999999.9f;
	extent.hi.y = -9999999999.9f;

	// compute total extent
	NavAreaList::iterator iter;
	for( iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
	{
		CNavArea *area = *iter;
		const Extent *areaExtent = area->GetExtent();

		if (areaExtent->lo.x < extent.lo.x)
			extent.lo.x = areaExtent->lo.x;
		if (areaExtent->lo.y < extent.lo.y)
			extent.lo.y = areaExtent->lo.y;
		if (areaExtent->hi.x > extent.hi.x)
			extent.hi.x = areaExtent->hi.x;
		if (areaExtent->hi.y > extent.hi.y)
			extent.hi.y = areaExtent->hi.y;
	}

	// add the areas to the grid
	TheNavAreaGrid.Initialize( extent.lo.x, extent.hi.x, extent.lo.y, extent.hi.y );

	for( iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
		TheNavAreaGrid.AddNavArea( *iter );


	ConnectGeneratedAreas();
	MergeGeneratedAreas();
	SquareUpAreas();
	MarkJumpAreas();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if 'pos' is within 2D extents of area.
 */
bool CNavArea::IsOverlapping( const Vector *pos ) const
{
	if (pos->x >= m_extent.lo.x && pos->x <= m_extent.hi.x &&
		pos->y >= m_extent.lo.y && pos->y <= m_extent.hi.y)
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if 'area' overlaps our 2D extents
 */
bool CNavArea::IsOverlapping( const CNavArea *area ) const
{
	if (area->m_extent.lo.x < m_extent.hi.x && area->m_extent.hi.x > m_extent.lo.x && 
		area->m_extent.lo.y < m_extent.hi.y && area->m_extent.hi.y > m_extent.lo.y)
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if 'area' overlaps our X extent
 */
bool CNavArea::IsOverlappingX( const CNavArea *area ) const
{
	if (area->m_extent.lo.x < m_extent.hi.x && area->m_extent.hi.x > m_extent.lo.x)
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if 'area' overlaps our Y extent
 */
bool CNavArea::IsOverlappingY( const CNavArea *area ) const
{
	if (area->m_extent.lo.y < m_extent.hi.y && area->m_extent.hi.y > m_extent.lo.y)
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if given point is on or above this area, but no others
 */
bool CNavArea::Contains( const Vector *pos ) const
{
	// check 2D overlap
	if (!IsOverlapping( pos ))
		return false;

	// the point overlaps us, check that it is above us, but not above any areas that overlap us
	float ourZ = GetZ( pos );

	// if we are above this point, fail
	if (ourZ > pos->z)
		return false;

	for( NavAreaList::const_iterator iter = m_overlapList.begin(); iter != m_overlapList.end(); ++iter )
	{
		const CNavArea *area = *iter;

		// skip self
		if (area == this)
			continue;

		// check 2D overlap
		if (!area->IsOverlapping( pos ))
			continue;

		float theirZ = area->GetZ( pos );
		if (theirZ > pos->z)
		{
			// they are above the point
			continue;
		}

		if (theirZ > ourZ)
		{
			// we are below an area that is closer underneath the point
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if this area and given area are approximately co-planar
 */
bool CNavArea::IsCoplanar( const CNavArea *area ) const
{
	Vector u, v;

	// compute our unit surface normal
	u.x = m_extent.hi.x - m_extent.lo.x;
	u.y = 0.0f;
	u.z = m_neZ - m_extent.lo.z;

	v.x = 0.0f;
	v.y = m_extent.hi.y - m_extent.lo.y;
	v.z = m_swZ - m_extent.lo.z;

	Vector normal = CrossProduct( u, v );
	normal.NormalizeInPlace();


	// compute their unit surface normal
	u.x = area->m_extent.hi.x - area->m_extent.lo.x;
	u.y = 0.0f;
	u.z = area->m_neZ - area->m_extent.lo.z;

	v.x = 0.0f;
	v.y = area->m_extent.hi.y - area->m_extent.lo.y;
	v.z = area->m_swZ - area->m_extent.lo.z;

	Vector otherNormal = CrossProduct( u, v );
	otherNormal.NormalizeInPlace();

	// can only merge areas that are nearly planar, to ensure areas do not differ from underlying geometry much
	const float tolerance = 0.99f; // 0.7071f;		// 0.9
	if (DotProduct( normal, otherNormal ) > tolerance)
		return true;

	return false;	
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return Z of area at (x,y) of 'pos'
 * Trilinear interpolation of Z values at quad edges.
 * NOTE: pos->z is not used.
 */
float CNavArea::GetZ( const Vector *pos ) const
{
	float dx = m_extent.hi.x - m_extent.lo.x;
	float dy = m_extent.hi.y - m_extent.lo.y;

	// guard against division by zero due to degenerate areas
	if (dx == 0.0f || dy == 0.0f)
		return m_neZ;

	float u = (pos->x - m_extent.lo.x) / dx;
	float v = (pos->y - m_extent.lo.y) / dy;

	// clamp Z values to (x,y) volume
	if (u < 0.0f)
		u = 0.0f;
	else if (u > 1.0f)
		u = 1.0f;
		
	if (v < 0.0f)
		v = 0.0f;
	else if (v > 1.0f)
		v = 1.0f;

	float northZ = m_extent.lo.z + u * (m_neZ - m_extent.lo.z);
	float southZ = m_swZ + u * (m_extent.hi.z - m_swZ);
	
	return northZ + v * (southZ - northZ);
}

float CNavArea::GetZ( float x, float y ) const
{
	Vector pos( x, y, 0.0f );
	return GetZ( &pos );
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return closest point to 'pos' on 'area'.
 * Returned point is in 'close'.
 */
void CNavArea::GetClosestPointOnArea( const Vector *pos, Vector *close ) const
{
	const Extent *extent = GetExtent();

	if (pos->x < extent->lo.x)
	{
		if (pos->y < extent->lo.y)
		{
			// position is north-west of area
			*close = extent->lo;
		}
		else if (pos->y > extent->hi.y)
		{
			// position is south-west of area
			close->x = extent->lo.x;
			close->y = extent->hi.y;
		}
		else
		{
			// position is west of area
			close->x = extent->lo.x;
			close->y = pos->y;
		}
	}
	else if (pos->x > extent->hi.x)
	{
		if (pos->y < extent->lo.y)
		{
			// position is north-east of area
			close->x = extent->hi.x;
			close->y = extent->lo.y;
		}
		else if (pos->y > extent->hi.y)
		{
			// position is south-east of area
			*close = extent->hi;
		}
		else
		{
			// position is east of area
			close->x = extent->hi.x;
			close->y = pos->y;
		}
	}
	else if (pos->y < extent->lo.y)
	{
		// position is north of area
		close->x = pos->x;
		close->y = extent->lo.y;
	}
	else if (pos->y > extent->hi.y)
	{
		// position is south of area
		close->x = pos->x;
		close->y = extent->hi.y;
	}
	else
	{
		// position is inside of area - it is the 'closest point' to itself
		*close = *pos;
	}

	close->z = GetZ( close );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return shortest distance squared between point and this area
 */
float CNavArea::GetDistanceSquaredToPoint( const Vector *pos ) const
{
	const Extent *extent = GetExtent();

	if (pos->x < extent->lo.x)
	{
		if (pos->y < extent->lo.y)
		{
			// position is north-west of area
			return (extent->lo - *pos).LengthSquared();
		}
		else if (pos->y > extent->hi.y)
		{
			// position is south-west of area
			Vector d;
			d.x = extent->lo.x - pos->x;
			d.y = extent->hi.y - pos->y;
			d.z = m_swZ - pos->z;
			return d.LengthSquared();
		}
		else
		{
			// position is west of area
			float d = extent->lo.x - pos->x;
			return d * d;
		}
	}
	else if (pos->x > extent->hi.x)
	{
		if (pos->y < extent->lo.y)
		{
			// position is north-east of area
			Vector d;
			d.x = extent->hi.x - pos->x;
			d.y = extent->lo.y - pos->y;
			d.z = m_neZ - pos->z;
			return d.LengthSquared();
		}
		else if (pos->y > extent->hi.y)
		{
			// position is south-east of area
			return (extent->hi - *pos).LengthSquared();
		}
		else
		{
			// position is east of area
			float d = pos->z - extent->hi.x;
			return d * d;
		}
	}
	else if (pos->y < extent->lo.y)
	{
		// position is north of area
		float d = extent->lo.y - pos->y;
		return d * d;
	}
	else if (pos->y > extent->hi.y)
	{
		// position is south of area
		float d = pos->y - extent->hi.y;
		return d * d;
	}
	else
	{
		// position is inside of 2D extent of area - find delta Z
		float z = GetZ( pos );
		float d = z - pos->z;
		return d * d;
	}
}



//--------------------------------------------------------------------------------------------------------------
CNavArea *CNavArea::GetRandomAdjacentArea( NavDirType dir ) const
{
	int count = m_connect[ dir ].size();
	int which = RANDOM_LONG( 0, count-1 );

	int i = 0;
	NavConnectList::const_iterator iter;
	for( iter = m_connect[ dir ].begin(); iter != m_connect[ dir ].end(); ++iter )
	{
		if (i == which)
			return (*iter).area;

		i++;
	}

	return NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Compute "portal" between to adjacent areas. 
 * Return center of portal opening, and half-width defining sides of portal from center.
 * NOTE: center->z is unset.
 */
void CNavArea::ComputePortal( const CNavArea *to, NavDirType dir, Vector *center, float *halfWidth ) const
{
	if (dir == NORTH || dir == SOUTH)
	{
		if (dir == NORTH)
			center->y = m_extent.lo.y;
		else
			center->y = m_extent.hi.y;

		float left = max( m_extent.lo.x, to->m_extent.lo.x );
		float right = min( m_extent.hi.x, to->m_extent.hi.x );

		// clamp to our extent in case areas are disjoint
		if (left < m_extent.lo.x)
			left = m_extent.lo.x;
		else if (left > m_extent.hi.x)
			left = m_extent.hi.x;

		if (right < m_extent.lo.x)
			right = m_extent.lo.x;
		else if (right > m_extent.hi.x)
			right = m_extent.hi.x;

		center->x = (left + right)/2.0f;
		*halfWidth = (right - left)/2.0f;
	}
	else	// EAST or WEST
	{
		if (dir == WEST)
			center->x = m_extent.lo.x;
		else
			center->x = m_extent.hi.x;

		float top = max( m_extent.lo.y, to->m_extent.lo.y );
		float bottom = min( m_extent.hi.y, to->m_extent.hi.y );

		// clamp to our extent in case areas are disjoint
		if (top < m_extent.lo.y)
			top = m_extent.lo.y;
		else if (top > m_extent.hi.y)
			top = m_extent.hi.y;

		if (bottom < m_extent.lo.y)
			bottom = m_extent.lo.y;
		else if (bottom > m_extent.hi.y)
			bottom = m_extent.hi.y;

		center->y = (top + bottom)/2.0f;
		*halfWidth = (bottom - top)/2.0f;
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Compute closest point within the "portal" between to adjacent areas. 
 */
void CNavArea::ComputeClosestPointInPortal( const CNavArea *to, NavDirType dir, const Vector *fromPos, Vector *closePos ) const
{
	const float margin = GenerationStepSize/2.0f;

	if (dir == NORTH || dir == SOUTH)
	{
		if (dir == NORTH)
			closePos->y = m_extent.lo.y;
		else
			closePos->y = m_extent.hi.y;

		float left = max( m_extent.lo.x, to->m_extent.lo.x );
		float right = min( m_extent.hi.x, to->m_extent.hi.x );

		// clamp to our extent in case areas are disjoint
		if (left < m_extent.lo.x)
			left = m_extent.lo.x;
		else if (left > m_extent.hi.x)
			left = m_extent.hi.x;

		if (right < m_extent.lo.x)
			right = m_extent.lo.x;
		else if (right > m_extent.hi.x)
			right = m_extent.hi.x;

		// keep margin if against edge
		const float leftMargin = (to->IsEdge( WEST )) ? (left + margin) : left;
		const float rightMargin = (to->IsEdge( EAST )) ? (right - margin) : right;

		// limit x to within portal
		if (fromPos->x < leftMargin)
			closePos->x = leftMargin;
		else if (fromPos->x > rightMargin)
			closePos->x = rightMargin;
		else
			closePos->x = fromPos->x;

	}
	else	// EAST or WEST
	{
		if (dir == WEST)
			closePos->x = m_extent.lo.x;
		else
			closePos->x = m_extent.hi.x;

		float top = max( m_extent.lo.y, to->m_extent.lo.y );
		float bottom = min( m_extent.hi.y, to->m_extent.hi.y );

		// clamp to our extent in case areas are disjoint
		if (top < m_extent.lo.y)
			top = m_extent.lo.y;
		else if (top > m_extent.hi.y)
			top = m_extent.hi.y;

		if (bottom < m_extent.lo.y)
			bottom = m_extent.lo.y;
		else if (bottom > m_extent.hi.y)
			bottom = m_extent.hi.y;

		// keep margin if against edge
		const float topMargin = (to->IsEdge( NORTH )) ? (top + margin) : top;
		const float bottomMargin = (to->IsEdge( SOUTH )) ? (bottom - margin) : bottom;

		// limit y to within portal
		if (fromPos->y < topMargin)
			closePos->y = topMargin;
		else if (fromPos->y > bottomMargin)
			closePos->y = bottomMargin;
		else
			closePos->y = fromPos->y;
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if there are no bi-directional links on the given side
 */
bool CNavArea::IsEdge( NavDirType dir ) const
{
	for( NavConnectList::const_iterator it = m_connect[ dir ].begin(); it != m_connect[ dir ].end(); ++it )
	{
		const NavConnect connect = *it;

		if (connect.area->IsConnected( this, OppositeDirection( dir ) ))
			return false;
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return direction from this area to the given point
 */
NavDirType CNavArea::ComputeDirection( Vector *point ) const
{
	if (point->x >= m_extent.lo.x && point->x <= m_extent.hi.x)
	{
		if (point->y < m_extent.lo.y)
			return NORTH;
		else if (point->y > m_extent.hi.y)
			return SOUTH;
	}
	else if (point->y >= m_extent.lo.y && point->y <= m_extent.hi.y)
	{
		if (point->x < m_extent.lo.x)
			return WEST;
		else if (point->x > m_extent.hi.x)
			return EAST;
	}

	// find closest direction
	Vector to = *point - m_center;

	if (abs(to.x) > abs(to.y))
	{
		if (to.x > 0.0f)
			return EAST;
		return WEST;
	}
	else
	{
		if (to.y > 0.0f)
			return SOUTH;
		return NORTH;
	}

	return NUM_DIRECTIONS;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Draw area for debugging
 */
void CNavArea::Draw( byte red, byte green, byte blue, int duration )
{
	Vector nw, ne, sw, se;

	nw = m_extent.lo;
	se = m_extent.hi;
	ne.x = se.x;
	ne.y = nw.y;
	ne.z = m_neZ;
	sw.x = nw.x;
	sw.y = se.y;
	sw.z = m_swZ;

	nw.z += cv_bot_nav_zdraw.value;
	ne.z += cv_bot_nav_zdraw.value;
	sw.z += cv_bot_nav_zdraw.value;
	se.z += cv_bot_nav_zdraw.value;

	float border = 2.0f;
	nw.x += border;
	nw.y += border;
	ne.x -= border;
	ne.y += border;
	sw.x += border;
	sw.y -= border;
	se.x -= border;
	se.y -= border;

	UTIL_DrawBeamPoints( nw, ne, duration, red, green, blue );
	UTIL_DrawBeamPoints( ne, se, duration, red, green, blue );
	UTIL_DrawBeamPoints( se, sw, duration, red, green, blue );
	UTIL_DrawBeamPoints( sw, nw, duration, red, green, blue );

	if (GetAttributes() & NAV_CROUCH)
		UTIL_DrawBeamPoints( nw, se, duration, red, green, blue );

	if (GetAttributes() & NAV_JUMP)
	{
		UTIL_DrawBeamPoints( nw, se, duration, red, green, blue );
		UTIL_DrawBeamPoints( ne, sw, duration, red, green, blue );
	}

	if (GetAttributes() & NAV_PRECISE)
	{
		float size = 8.0f;
		Vector up( m_center.x, m_center.y - size, m_center.z + cv_bot_nav_zdraw.value );
		Vector down( m_center.x, m_center.y + size, m_center.z + cv_bot_nav_zdraw.value );
		UTIL_DrawBeamPoints( up, down, duration, red, green, blue );

		Vector left( m_center.x - size, m_center.y, m_center.z + cv_bot_nav_zdraw.value );
		Vector right( m_center.x + size, m_center.y, m_center.z + cv_bot_nav_zdraw.value );
		UTIL_DrawBeamPoints( left, right, duration, red, green, blue );
	}

	if (GetAttributes() & NAV_NO_JUMP)
	{
		float size = 8.0f;
		Vector up( m_center.x, m_center.y - size, m_center.z + cv_bot_nav_zdraw.value );
		Vector down( m_center.x, m_center.y + size, m_center.z + cv_bot_nav_zdraw.value );
		Vector left( m_center.x - size, m_center.y, m_center.z + cv_bot_nav_zdraw.value );
		Vector right( m_center.x + size, m_center.y, m_center.z + cv_bot_nav_zdraw.value );
		UTIL_DrawBeamPoints( up, right, duration, red, green, blue );
		UTIL_DrawBeamPoints( right, down, duration, red, green, blue );
		UTIL_DrawBeamPoints( down, left, duration, red, green, blue );
		UTIL_DrawBeamPoints( left, up, duration, red, green, blue );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Draw selected corner for debugging
 */
void CNavArea::DrawMarkedCorner( NavCornerType corner, byte red, byte green, byte blue, int duration )
{
	Vector nw, ne, sw, se;

	nw = m_extent.lo;
	se = m_extent.hi;
	ne.x = se.x;
	ne.y = nw.y;
	ne.z = m_neZ;
	sw.x = nw.x;
	sw.y = se.y;
	sw.z = m_swZ;

	nw.z += cv_bot_nav_zdraw.value;
	ne.z += cv_bot_nav_zdraw.value;
	sw.z += cv_bot_nav_zdraw.value;
	se.z += cv_bot_nav_zdraw.value;

	float border = 2.0f;
	nw.x += border;
	nw.y += border;
	ne.x -= border;
	ne.y += border;
	sw.x += border;
	sw.y -= border;
	se.x -= border;
	se.y -= border;

	switch( corner )
	{
	case NORTH_WEST:
		UTIL_DrawBeamPoints( nw + Vector( 0, 0, 10 ), nw, duration, red, green, blue );
		break;
	case NORTH_EAST:
		UTIL_DrawBeamPoints( ne + Vector( 0, 0, 10 ), ne, duration, red, green, blue );
		break;
	case SOUTH_EAST:
		UTIL_DrawBeamPoints( se + Vector( 0, 0, 10 ), se, duration, red, green, blue );
		break;
	case SOUTH_WEST:
		UTIL_DrawBeamPoints( sw + Vector( 0, 0, 10 ), sw, duration, red, green, blue );
		break;
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Add to open list in decreasing value order
 */
void CNavArea::AddToOpenList( void )
{
	// mark as being on open list for quick check
	m_openMarker = m_masterMarker;

	// if list is empty, add and return
	if (m_openList == NULL)
	{
		m_openList = this;
		this->m_prevOpen = NULL;
		this->m_nextOpen = NULL;
		return;
	}

	// insert self in ascending cost order
	CNavArea *area, *last = NULL;
	for( area = m_openList; area; area = area->m_nextOpen )
	{
		if (this->GetTotalCost() < area->GetTotalCost())
			break;

		last = area;
	}

	if (area)
	{
		// insert before this area
		this->m_prevOpen = area->m_prevOpen;
		if (this->m_prevOpen)
			this->m_prevOpen->m_nextOpen = this;
		else
			m_openList = this;

		this->m_nextOpen = area;
		area->m_prevOpen = this;
	}
	else
	{
		// append to end of list
		last->m_nextOpen = this;

		this->m_prevOpen = last;
		this->m_nextOpen = NULL;
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * A smaller value has been found, update this area on the open list
 * @todo "bubbling" does unnecessary work, since the order of all other nodes will be unchanged - only this node is altered
 */
void CNavArea::UpdateOnOpenList( void )
{
	// since value can only decrease, bubble this area up from current spot
	while( m_prevOpen && 
				 this->GetTotalCost() < m_prevOpen->GetTotalCost() )
	{
		// swap position with predecessor
		CNavArea *other = m_prevOpen;
		CNavArea *before = other->m_prevOpen;
		CNavArea *after  = this->m_nextOpen;

		this->m_nextOpen = other;
		this->m_prevOpen = before;

		other->m_prevOpen = this;
		other->m_nextOpen = after;

		if (before)
			before->m_nextOpen = this;
		else
			m_openList = this;

		if (after)
			after->m_prevOpen = other;
	}
}

//--------------------------------------------------------------------------------------------------------------
void CNavArea::RemoveFromOpenList( void )
{
	if (m_prevOpen)
		m_prevOpen->m_nextOpen = m_nextOpen;
	else
		m_openList = m_nextOpen;

	if (m_nextOpen)
		m_nextOpen->m_prevOpen = m_prevOpen;

	// zero is an invalid marker
	m_openMarker = 0;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Clears the open and closed lists for a new search
 */
void CNavArea::ClearSearchLists( void )
{
	// effectively clears all open list pointers and closed flags
	CNavArea::MakeNewMarker();

	m_openList = NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the coordinates of the area's corner.
 * NOTE: Do not retain the returned pointer - it is temporary.
 */
const Vector *CNavArea::GetCorner( NavCornerType corner ) const
{
	static Vector pos;

	switch( corner )
	{
		case NORTH_WEST:
			return &m_extent.lo;

		case NORTH_EAST:
			pos.x = m_extent.hi.x;
			pos.y = m_extent.lo.y;
			pos.z = m_neZ;
			return &pos;

		case SOUTH_WEST:
			pos.x = m_extent.lo.x;
			pos.y = m_extent.hi.y;
			pos.z = m_swZ;
			return &pos;

		case SOUTH_EAST:
			return &m_extent.hi;
	}

	return NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Returns true if an existing hiding spot is too close to given position
 */
bool CNavArea::IsHidingSpotCollision( const Vector *pos ) const
{
	const float collisionRange = 30.0f;

	for( HidingSpotList::const_iterator iter = m_hidingSpotList.begin(); iter != m_hidingSpotList.end(); ++iter )
	{
		const HidingSpot *spot = *iter;

		if ((*spot->GetPosition() - *pos).IsLengthLessThan( collisionRange ))
			return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------
bool IsHidingSpotInCover( const Vector *spot )
{
	int coverCount = 0;
	TraceResult result;

	Vector from = *spot;
	from.z += HalfHumanHeight;

	Vector to;

	// if we are crouched underneath something, that counts as good cover
	to = from + Vector( 0, 0, 20.0f );
	UTIL_TraceLine( from, to, ignore_monsters, NULL, &result );
	if (result.flFraction != 1.0f)
		return true;

	const float coverRange = 100.0f;
	const float inc = M_PI / 8.0f;

	for( float angle = 0.0f; angle < 2.0f * M_PI; angle += inc )
	{
		to = from + Vector( coverRange * cos(angle), coverRange * sin(angle), HalfHumanHeight );

		UTIL_TraceLine( from, to, ignore_monsters, NULL, &result );

		// if traceline hit something, it hit "cover"
		if (result.flFraction != 1.0f)
			++coverCount;
	}

	// if more than half of the circle has no cover, the spot is not "in cover"
	const int halfCover = 8;
	if (coverCount < halfCover)
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Analyze local area neighborhood to find "hiding spots" for this area
 */
void CNavArea::ComputeHidingSpots( void )
{
	struct
	{
		float lo, hi;
	}
	extent;

	// "jump areas" cannot have hiding spots
	if (GetAttributes() & NAV_JUMP)
		return;

	int cornerCount[NUM_CORNERS];
	for( int i=0; i<NUM_CORNERS; ++i )
		cornerCount[i] = 0;

	const float cornerSize = 20.0f;

	// for each direction, find extents of adjacent areas along the wall
	for( int d=0; d<NUM_DIRECTIONS; ++d )
	{
		extent.lo = 999999.9f;
		extent.hi = -999999.9f;

		bool isHoriz = (d == NORTH || d == SOUTH) ? true : false;

		for( NavConnectList::iterator iter = m_connect[d].begin(); iter != m_connect[d].end(); ++iter )
		{
			NavConnect connect = *iter;

			// if connection is only one-way, it's a "jump down" connection (ie: a discontinuity that may mean cover) 
			// ignore it
			if (connect.area->IsConnected( this, OppositeDirection( static_cast<NavDirType>( d ) ) ) == false)
				continue;

			// ignore jump areas
			if (connect.area->GetAttributes() & NAV_JUMP)
				continue;

			if (isHoriz)
			{
				if (connect.area->m_extent.lo.x < extent.lo)
					extent.lo = connect.area->m_extent.lo.x;

				if (connect.area->m_extent.hi.x > extent.hi)
					extent.hi = connect.area->m_extent.hi.x;			
			}
			else
			{
				if (connect.area->m_extent.lo.y < extent.lo)
					extent.lo = connect.area->m_extent.lo.y;

				if (connect.area->m_extent.hi.y > extent.hi)
					extent.hi = connect.area->m_extent.hi.y;
			}
		}

		switch( d )
		{
			case NORTH:
				if (extent.lo - m_extent.lo.x >= cornerSize)
					++cornerCount[ NORTH_WEST ];

				if (m_extent.hi.x - extent.hi >= cornerSize)
					++cornerCount[ NORTH_EAST ];
				break;

			case SOUTH:
				if (extent.lo - m_extent.lo.x >= cornerSize)
					++cornerCount[ SOUTH_WEST ];

				if (m_extent.hi.x - extent.hi >= cornerSize)
					++cornerCount[ SOUTH_EAST ];
				break;

			case EAST:
				if (extent.lo - m_extent.lo.y >= cornerSize)
					++cornerCount[ NORTH_EAST ];

				if (m_extent.hi.y - extent.hi >= cornerSize)
					++cornerCount[ SOUTH_EAST ];
				break;

			case WEST:
				if (extent.lo - m_extent.lo.y >= cornerSize)
					++cornerCount[ NORTH_WEST ];

				if (m_extent.hi.y - extent.hi >= cornerSize)
					++cornerCount[ SOUTH_WEST ];
				break;
		}
	}

	// if a corner count is 2, then it really is a corner (walls on both sides)
	float offset = 12.5f;

	if (cornerCount[ NORTH_WEST ] == 2)
	{
		Vector pos = *GetCorner( NORTH_WEST ) + Vector(  offset,  offset, 0.0f );

		m_hidingSpotList.push_back( new HidingSpot( &pos, (IsHidingSpotInCover( &pos )) ? HidingSpot::IN_COVER : 0 ) );
	}

	if (cornerCount[ NORTH_EAST ] == 2)
	{
		Vector pos = *GetCorner( NORTH_EAST ) + Vector( -offset,  offset, 0.0f );
		if (!IsHidingSpotCollision( &pos ))
			m_hidingSpotList.push_back( new HidingSpot( &pos, (IsHidingSpotInCover( &pos )) ? HidingSpot::IN_COVER : 0 ) );
	}

	if (cornerCount[ SOUTH_WEST ] == 2)
	{
		Vector pos = *GetCorner( SOUTH_WEST ) + Vector(  offset, -offset, 0.0f );
		if (!IsHidingSpotCollision( &pos ))
			m_hidingSpotList.push_back( new HidingSpot( &pos, (IsHidingSpotInCover( &pos )) ? HidingSpot::IN_COVER : 0 ) );
	}

	if (cornerCount[ SOUTH_EAST ] == 2)
	{
		Vector pos = *GetCorner( SOUTH_EAST ) + Vector( -offset, -offset, 0.0f );
		if (!IsHidingSpotCollision( &pos ))
			m_hidingSpotList.push_back( new HidingSpot( &pos, (IsHidingSpotInCover( &pos )) ? HidingSpot::IN_COVER : 0 ) );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Determine how much walkable area we can see from the spot, and how far away we can see.
 */
void ClassifySniperSpot( HidingSpot *spot )
{
	Vector eye = *spot->GetPosition() + Vector( 0, 0, HalfHumanHeight );		// assume we are crouching
	Vector walkable;
	TraceResult result;

	Extent sniperExtent;
	float farthestRangeSq = 0.0f;
	const float minSniperRangeSq = 1000.0f * 1000.0f;
	bool found = false;

	for( NavAreaList::iterator iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
	{
		CNavArea *area = *iter;

		const Extent *extent = area->GetExtent();

		// scan this area
		for( walkable.y = extent->lo.y + GenerationStepSize/2.0f; walkable.y < extent->hi.y; walkable.y += GenerationStepSize )
		{
			for( walkable.x = extent->lo.x + GenerationStepSize/2.0f; walkable.x < extent->hi.x; walkable.x += GenerationStepSize )
			{
				walkable.z = area->GetZ( &walkable ) + HalfHumanHeight;
				
				// check line of sight
				UTIL_TraceLine( eye, walkable, ignore_monsters, ignore_glass, NULL, &result );

				if (result.flFraction == 1.0f && !result.fStartSolid)
				{
					// can see this spot

					// keep track of how far we can see
					float rangeSq = (eye - walkable).LengthSquared();
					if (rangeSq > farthestRangeSq)
					{
						farthestRangeSq = rangeSq;

						if (rangeSq >= minSniperRangeSq)
						{
							// this is a sniper spot
							// determine how good of a sniper spot it is by keeping track of the snipable area
							if (found)
							{
								if (walkable.x < sniperExtent.lo.x)
									sniperExtent.lo.x = walkable.x;
								if (walkable.x > sniperExtent.hi.x)
									sniperExtent.hi.x = walkable.x;

								if (walkable.y < sniperExtent.lo.y)
									sniperExtent.lo.y = walkable.y;
								if (walkable.y > sniperExtent.hi.y)
									sniperExtent.hi.y = walkable.y;
							}
							else
							{
								sniperExtent.lo = walkable;
								sniperExtent.hi = walkable;
								found = true;
							}
						}
					}
				}	
			}
		}
	}

	if (found)
	{
		// if we can see a large snipable area, it is an "ideal" spot
		float snipableArea = sniperExtent.Area();

		const float minIdealSniperArea = 200.0f * 200.0f;
		const float longSniperRangeSq = 1500.0f * 1500.0f;

		if (snipableArea >= minIdealSniperArea || farthestRangeSq >= longSniperRangeSq)
			spot->SetFlags( HidingSpot::IDEAL_SNIPER_SPOT );
		else
			spot->SetFlags( HidingSpot::GOOD_SNIPER_SPOT );
	}
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Analyze local area neighborhood to find "sniper spots" for this area
 */
void CNavArea::ComputeSniperSpots( void )
{
	if (cv_bot_quicksave.value > 0.0f)
		return;

	for( HidingSpotList::iterator iter = m_hidingSpotList.begin(); iter != m_hidingSpotList.end(); ++iter )
	{
		HidingSpot *spot = *iter;

		ClassifySniperSpot( spot );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Given the areas we are moving between, return the spots we will encounter
 */
SpotEncounter *CNavArea::GetSpotEncounter( const CNavArea *from, const CNavArea *to )
{
	if (from && to)
	{
		SpotEncounter *e;

		for( SpotEncounterList::iterator iter = m_spotEncounterList.begin(); iter != m_spotEncounterList.end(); ++iter )
		{
			e = &(*iter);

			if (e->from.area == from && e->to.area == to)
				return e;
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Add spot encounter data when moving from area to area
 */
void CNavArea::AddSpotEncounters( const CNavArea *from, NavDirType fromDir, const CNavArea *to, NavDirType toDir )
{
	SpotEncounter e;

	e.from.area = const_cast<CNavArea *>( from );
	e.fromDir = fromDir;

	e.to.area = const_cast<CNavArea *>( to );
	e.toDir = toDir;

	float halfWidth;
	ComputePortal( to, toDir, &e.path.to, &halfWidth );
	ComputePortal( from, fromDir, &e.path.from, &halfWidth );

	const float eyeHeight = HalfHumanHeight;
	e.path.from.z = from->GetZ( &e.path.from ) + eyeHeight;
	e.path.to.z = to->GetZ( &e.path.to ) + eyeHeight;

	// step along ray and track which spots can be seen
	Vector dir = e.path.to - e.path.from;
	float length = dir.NormalizeInPlace();

	// create unique marker to flag used spots
	HidingSpot::ChangeMasterMarker();

	const float stepSize = 25.0f;		// 50
	const float seeSpotRange = 2000.0f;	// 3000
	TraceResult result;

	Vector eye, delta;
	HidingSpot *spot;
	SpotOrder spotOrder;

	// step along path thru this area
	bool done = false;
	for( float along = 0.0f; !done; along += stepSize )
	{
		// make sure we check the endpoint of the path segment
		if (along >= length)
		{
			along = length;
			done = true;
		}

		// move the eyepoint along the path segment
		eye = e.path.from + along * dir;

		// check each hiding spot for visibility
		for( HidingSpotList::iterator iter = TheHidingSpotList.begin(); iter != TheHidingSpotList.end(); ++iter )
		{
			spot = *iter;

			// only look at spots with cover (others are out in the open and easily seen)
			if (!spot->HasGoodCover())
				continue;

			if (spot->IsMarked())
				continue;

			const Vector *spotPos = spot->GetPosition();

			delta.x = spotPos->x - eye.x;
			delta.y = spotPos->y - eye.y;
			delta.z = (spotPos->z + eyeHeight) - eye.z;

			// check if in range
			if (delta.IsLengthGreaterThan( seeSpotRange ))
				continue;

			// check if we have LOS
			UTIL_TraceLine( eye, Vector( spotPos->x, spotPos->y, spotPos->z + HalfHumanHeight ), ignore_monsters, ignore_glass, NULL, &result );
			if (result.flFraction != 1.0f)
				continue;

			// if spot is in front of us along our path, ignore it
			delta.NormalizeInPlace();
			float dot = DotProduct( dir, delta );
			if (dot < 0.7071f && dot > -0.7071f)
			{
				// we only want to keep spots that BECOME visible as we walk past them
				// therefore, skip ALL visible spots at the start of the path segment
				if (along > 0.0f)
				{
					// add spot to encounter
					spotOrder.spot = spot;
					spotOrder.t = along/length;
					e.spotList.push_back( spotOrder );
				}
			}

			// mark spot as encountered
			spot->Mark();
		}
	}

	// add encounter to list
	m_spotEncounterList.push_back( e );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Compute "spot encounter" data. This is an ordered list of spots to look at 
 * for each possible path thru a nav area.
 */
void CNavArea::ComputeSpotEncounters( void )
{
	m_spotEncounterList.clear();

	if (cv_bot_quicksave.value > 0.0f)
		return;

	// for each adjacent area
	for( int fromDir=0; fromDir<NUM_DIRECTIONS; ++fromDir )
	{
		for( NavConnectList::iterator fromIter = m_connect[ fromDir ].begin(); fromIter != m_connect[ fromDir ].end(); ++fromIter )
		{
			NavConnect *fromCon = &(*fromIter);

			// compute encounter data for path to each adjacent area
			for( int toDir=0; toDir<NUM_DIRECTIONS; ++toDir )
			{
				for( NavConnectList::iterator toIter = m_connect[ toDir ].begin(); toIter != m_connect[ toDir ].end(); ++toIter )
				{
					NavConnect *toCon = &(*toIter);

					if (toCon == fromCon)
						continue;

					// just do our direction, as we'll loop around for other direction
					AddSpotEncounters( fromCon->area, (NavDirType)fromDir, toCon->area, (NavDirType)toDir );
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Decay the danger values
 */
void CNavArea::DecayDanger( void )
{
	// one kill == 1.0, which we will forget about in two minutes
	const float decayRate = 1.0f / 120.0f;

	for( int i=0; i<MAX_AREA_TEAMS; ++i )
	{
		float deltaT = gpGlobals->time - m_dangerTimestamp[i];
		float decayAmount = decayRate * deltaT;

		m_danger[i] -= decayAmount;
		if (m_danger[i] < 0.0f)
			m_danger[i] = 0.0f;

		// update timestamp
		m_dangerTimestamp[i] = gpGlobals->time;
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Increase the danger of this area for the given team
 */
void CNavArea::IncreaseDanger( int teamID, float amount )
{
	// before we add the new value, decay what's there
	DecayDanger();

	m_danger[ teamID ] += amount;
	m_dangerTimestamp[ teamID ] = gpGlobals->time;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the danger of this area (decays over time)
 */
float CNavArea::GetDanger( int teamID )
{
	DecayDanger();
	return m_danger[ teamID ];
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Increase the danger of nav areas containing and near the given position
 */
void IncreaseDangerNearby( int teamID, float amount, CNavArea *startArea, const Vector *pos, float maxRadius )
{
	if (startArea == NULL)
		return;

	CNavArea::MakeNewMarker();
	CNavArea::ClearSearchLists();

	startArea->AddToOpenList();
	startArea->SetTotalCost( 0.0f );
	startArea->Mark();
	startArea->IncreaseDanger( teamID, amount );

	while( !CNavArea::IsOpenListEmpty() )
	{
		// get next area to check
		CNavArea *area = CNavArea::PopOpenList();
		
		// area has no hiding spots, explore adjacent areas
		for( int dir=0; dir<NUM_DIRECTIONS; ++dir )
		{
			int count = area->GetAdjacentCount( (NavDirType)dir );
			for( int i=0; i<count; ++i )
			{
				CNavArea *adjArea = area->GetAdjacentArea( (NavDirType)dir, i );

				if (!adjArea->IsMarked())
				{
					// compute distance from danger source
					float cost = (*adjArea->GetCenter() - *pos).Length();
					if (cost <= maxRadius)
					{
						adjArea->AddToOpenList();
						adjArea->SetTotalCost( cost );
						adjArea->Mark();
						adjArea->IncreaseDanger( teamID, amount * cost/maxRadius );
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Show danger levels for debugging
 */
void DrawDanger( void )
{
	for( NavAreaList::iterator iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
	{
		CNavArea *area = *iter;

		Vector center = *area->GetCenter();
		Vector top;
		center.z = area->GetZ( &center );

		float danger = area->GetDanger( 0 );
		if (danger > 0.1f)
		{
			top.x = center.x;
			top.y = center.y;
			top.z = center.z + 10.0f * danger;
			UTIL_DrawBeamPoints( center, top, 3.0f, 255, 0, 0 );
		}

		danger = area->GetDanger( 1 );
		if (danger > 0.1f)
		{
			top.x = center.x;
			top.y = center.y;
			top.z = center.z + 10.0f * danger;
			UTIL_DrawBeamPoints( center, top, 3.0f, 0, 0, 255 );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
/**
 * If a player is at the given spot, return true
 */
bool IsSpotOccupied( CBaseEntity *me, const Vector *pos )
{
	const float closeRange = 75.0f;		// 50

	// is there a player in this spot
	float range;
	CBasePlayer *player = UTIL_GetClosestPlayer( pos, &range );

	if (player != me)
	{
		if (player && range < closeRange)
			return true;
	}

	// is there is a hostage in this spot
	if (g_pHostages)
	{
		CHostage *hostage = g_pHostages->GetClosestHostage( *pos, &range );
		if (hostage && hostage != me && range < closeRange)
			return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------
class CollectHidingSpotsFunctor
{
public:
	CollectHidingSpotsFunctor( CBaseEntity *me, const Vector *origin, float range, unsigned char flags, Place place = UNDEFINED_PLACE, bool useCrouchAreas = true )
	{
		m_me = me;
		m_count = 0;
		m_origin = origin;
		m_range = range;
		m_flags = flags;
		m_place = place;
		m_useCrouchAreas = useCrouchAreas;
	}

	enum { MAX_SPOTS = 256 };

	bool operator() ( CNavArea *area )
	{
		// if a place is specified, only consider hiding spots from areas in that place
		if (m_place != UNDEFINED_PLACE && area->GetPlace() != m_place)
			return true;

		// collect all the hiding spots in this area
		const HidingSpotList *list = area->GetHidingSpotList();
		
		for( HidingSpotList::const_iterator iter = list->begin(); iter != list->end() && m_count < MAX_SPOTS; ++iter )
		{
			const HidingSpot *spot = *iter;

			if (m_useCrouchAreas == false)
			{
				CNavArea *area = TheNavAreaGrid.GetNavArea( spot->GetPosition() );
				if (area && (area->GetAttributes() & NAV_CROUCH))
					continue;
			}

			// make sure hiding spot is in range
			if (m_range > 0.0f)
				if ((*spot->GetPosition() - *m_origin).IsLengthGreaterThan( m_range ))
					continue;

			// if a Player is using this hiding spot, don't consider it
			if (IsSpotOccupied( m_me, spot->GetPosition() ))
			{
				// player is in hiding spot
				/// @todo Check if player is moving or sitting still
				continue;
			}

			// only collect hiding spots with matching flags
			if (m_flags & spot->GetFlags())
			{
				m_hidingSpot[ m_count++ ] = spot->GetPosition();
			}
		}

		// if we've filled up, stop searching
		if (m_count == MAX_SPOTS)
			return false;

		return true;
	}

	/**
	 * Remove the spot at index "i"
	 */
	void RemoveSpot( int i )
	{
		if (m_count == 0)
			return;

		for( int j=i+1; j<m_count; ++j )
			m_hidingSpot[j-1] = m_hidingSpot[j];

		--m_count;
	}


	CBaseEntity *m_me;
	const Vector *m_origin;
	float m_range;

	const Vector *m_hidingSpot[ MAX_SPOTS ];
	int m_count;

	unsigned char m_flags;
	Place m_place;
	bool m_useCrouchAreas;
};

/**
 * Do a breadth-first search to find a nearby hiding spot and return it.
 * Don't pick a hiding spot that a Player is currently occupying.
 * @todo Clean up this mess
 */
const Vector *FindNearbyHidingSpot( CBaseEntity *me, const Vector *pos, CNavArea *startArea, float maxRange, bool isSniper, bool useNearest )
{
	if (startArea == NULL)
		return NULL;

	// collect set of nearby hiding spots
	if (isSniper)
	{
		CollectHidingSpotsFunctor collector( me, pos, maxRange, HidingSpot::IDEAL_SNIPER_SPOT );
		SearchSurroundingAreas( startArea, pos, collector, maxRange );

		if (collector.m_count)
		{
			int which = RANDOM_LONG( 0, collector.m_count-1 );
			return collector.m_hidingSpot[ which ];
		}
		else
		{
			// no ideal sniping spots, look for "good" sniping spots
			CollectHidingSpotsFunctor collector( me, pos, maxRange, HidingSpot::GOOD_SNIPER_SPOT );
			SearchSurroundingAreas( startArea, pos, collector, maxRange );

			if (collector.m_count)
			{
				int which = RANDOM_LONG( 0, collector.m_count-1 );
				return collector.m_hidingSpot[ which ];
			}

			// no sniping spots at all.. fall through and pick a normal hiding spot
		}
	}

	// collect hiding spots with decent "cover"
	CollectHidingSpotsFunctor collector( me, pos, maxRange, HidingSpot::IN_COVER );
	SearchSurroundingAreas( startArea, pos, collector, maxRange );

	if (collector.m_count == 0)
		return NULL;

	if (useNearest)
	{
		// return closest hiding spot
		const Vector *closest = NULL;
		float closeRangeSq = 9999999999.9f;
		for( int i=0; i<collector.m_count; ++i )
		{
			float rangeSq = (*collector.m_hidingSpot[i] - *pos).LengthSquared();
			if (rangeSq < closeRangeSq)
			{
				closeRangeSq = rangeSq;
				closest = collector.m_hidingSpot[i];
			}
		}

		return closest;
	}

	// select a hiding spot at random
	int which = RANDOM_LONG( 0, collector.m_count-1 );
	return collector.m_hidingSpot[ which ];
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Select a random hiding spot among the nav areas that are tagged with the given place
 */
const Vector *FindRandomHidingSpot( CBaseEntity *me, Place place, bool isSniper )
{
	// collect set of nearby hiding spots
	if (isSniper)
	{
		CollectHidingSpotsFunctor collector( me, NULL, -1.0f, HidingSpot::IDEAL_SNIPER_SPOT, place );
		ForAllAreas( collector );

		if (collector.m_count)
		{
			int which = RANDOM_LONG( 0, collector.m_count-1 );
			return collector.m_hidingSpot[ which ];
		}
		else
		{
			// no ideal sniping spots, look for "good" sniping spots
			CollectHidingSpotsFunctor collector( me, NULL, -1.0f, HidingSpot::GOOD_SNIPER_SPOT, place );
			ForAllAreas( collector );

			if (collector.m_count)
			{
				int which = RANDOM_LONG( 0, collector.m_count-1 );
				return collector.m_hidingSpot[ which ];
			}

			// no sniping spots at all.. fall through and pick a normal hiding spot
		}
	}

	// collect hiding spots with decent "cover"
	CollectHidingSpotsFunctor collector( me, NULL, -1.0f, HidingSpot::IN_COVER, place );
	ForAllAreas( collector );

	if (collector.m_count == 0)
		return NULL;

	// select a hiding spot at random
	int which = RANDOM_LONG( 0, collector.m_count-1 );
	return collector.m_hidingSpot[ which ];
}

//--------------------------------------------------------------------------------------------------------------------
/**
 * Return true if moving from "start" to "finish" will cross a player's line of fire.
 * The path from "start" to "finish" is assumed to be a straight line.
 * "start" and "finish" are assumed to be points on the ground.
 */
bool IsCrossingLineOfFire( const Vector &start, const Vector &finish, CBaseEntity *ignore, int ignoreTeam  )
{
	for ( int p=1; p <= gpGlobals->maxClients; ++p )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( p ) );

		if (!IsEntityValid( player ))
			continue;

		if (player == ignore)
			continue;

		if (!player->IsAlive())
			continue;

		if (ignoreTeam && player->m_iTeam == ignoreTeam)
			continue;

		// compute player's unit aiming vector 
		UTIL_MakeVectors( player->pev->v_angle + player->pev->punchangle );

		const float longRange = 5000.0f;
		Vector playerTarget = player->pev->origin + longRange * gpGlobals->v_forward;

		Vector result;
		if (IsIntersecting2D( start, finish, player->pev->origin, playerTarget, &result ))
		{
			// simple check to see if intersection lies in the Z range of the path
			float loZ, hiZ;

			if (start.z < finish.z)
			{
				loZ = start.z;
				hiZ = finish.z;
			}
			else
			{
				loZ = finish.z;
				hiZ = start.z;
			}

			if (result.z >= loZ && result.z <= hiZ + HumanHeight)
				return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------------
/**
 * Select a nearby retreat spot.
 * Don't pick a hiding spot that a Player is currently occupying.
 * If "avoidTeam" is nonzero, avoid getting close to members of that team.
 */
const Vector *FindNearbyRetreatSpot( CBaseEntity *me, const Vector *start, CNavArea *startArea, float maxRange, int avoidTeam, bool useCrouchAreas )
{
	if (startArea == NULL)
		return NULL;

	// collect hiding spots with decent "cover"
	CollectHidingSpotsFunctor collector( me, start, maxRange, HidingSpot::IN_COVER, UNDEFINED_PLACE, useCrouchAreas );
	SearchSurroundingAreas( startArea, start, collector, maxRange );

	if (collector.m_count == 0)
		return NULL;

	// find the closest unoccupied hiding spot that crosses the least lines of fire and has the best cover
	for( int i=0; i<collector.m_count; ++i )
	{
		// check if we would have to cross a line of fire to reach this hiding spot
		if (IsCrossingLineOfFire( *start, *collector.m_hidingSpot[i], me ))
		{
			collector.RemoveSpot( i );

			// back up a step, so iteration won't skip a spot
			--i;

			continue;
		}

		// check if there is someone on the avoidTeam near this hiding spot
		if (avoidTeam)
		{
			float range;
			if (UTIL_GetClosestPlayer( collector.m_hidingSpot[i], avoidTeam, &range ))
			{
				const float dangerRange = 150.0f;
				if (range < dangerRange)
				{
					// there is an avoidable player too near this spot - remove it
					collector.RemoveSpot( i );

					// back up a step, so iteration won't skip a spot
					--i;

					continue;
				}
			}
		}
	}

	if (collector.m_count <= 0)
		return NULL;

	// all remaining spots are ok - pick one at random
	int which = RANDOM_LONG( 0, collector.m_count-1 );
	return collector.m_hidingSpot[ which ];
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return number of players with given teamID in this area (teamID == 0 means any/all)
 * @todo Keep pointers to contained Players to make this a zero-time query
 */
int CNavArea::GetPlayerCount( int teamID, CBasePlayer *ignore ) const
{
	int count = 0;

	for( int i=1; i<=gpGlobals->maxClients; ++i )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

		if (player == ignore)
			continue;

		if (!IsEntityValid( player ))
			continue;

		if (!player->IsPlayer())
			continue;

		if (!player->IsAlive())
			continue;

		if (teamID == 0 || player->m_iTeam == teamID)
			if (Contains( &player->pev->origin ))
				++count;
	}
	
	return count;
}

//--------------------------------------------------------------------------------------------------------------
static CNavArea *markedArea = NULL;
static CNavArea *lastSelectedArea = NULL;
static NavCornerType markedCorner = NUM_CORNERS;

static bool isCreatingNavArea = false;						///< if true, we are manually creating a new nav area
static bool isAnchored = false;
static Vector anchor;

static bool isPlaceMode = false;								///< if true, we are in place editing mode
static bool isPlacePainting = false;						///< if true, we set an area's place by pointing at it

static float editTimestamp = 0.0f;

CNavArea *GetMarkedArea( void )
{
	return markedArea;
}

/**
 * Draw navigation areas and edit them
 */
void EditNavAreasReset( void )
{
	markedArea = NULL;
	lastSelectedArea = NULL;
	isCreatingNavArea = false;
	editTimestamp = 0.0f;
	isPlacePainting = false;
	lastDrawTimestamp = 0.0f;
}

void DrawHidingSpots( const CNavArea *area )
{
	const HidingSpotList *list = area->GetHidingSpotList();
	for( HidingSpotList::const_iterator iter = list->begin(); iter != list->end(); ++iter )
	{
		const HidingSpot *spot = *iter;

		int r, g, b;

		if (spot->IsIdealSniperSpot())
		{
			r = 255; g = 0; b = 0;
		}
		else if (spot->IsGoodSniperSpot())
		{
			r = 255; g = 0; b = 255;
		}
		else if (spot->HasGoodCover())
		{
			r = 0; g = 255; b = 0;
		}
		else
		{
			r = 0; g = 0; b = 1;
		}

		UTIL_DrawBeamPoints( *spot->GetPosition(), *spot->GetPosition() + Vector( 0, 0, 50 ), 3, r, g, b );
	}
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Draw ourselves and adjacent areas
 */
void CNavArea::DrawConnectedAreas( void )
{
	CBasePlayer *player = UTIL_GetLocalPlayer();
	if (player == NULL)
		return;

	CCSBotManager *ctrl = static_cast<CCSBotManager *>( TheBots );
	const float maxRange = 500.0f;

	// draw self
	if (isPlaceMode)
	{
		if (GetPlace() == 0)
			Draw( 50, 0, 0, 3 );
		else if (GetPlace() != ctrl->GetNavPlace())
			Draw( 0, 0, 200, 3 );
		else
			Draw( 0, 255, 0, 3 );
	}
	else
	{
		Draw( 255, 255, 0, 3 );
		DrawHidingSpots( this );
	}

	// randomize order of directions to make sure all connected areas are
	// drawn, since we may have too many to render all at once
	int dirSet[ NUM_DIRECTIONS ];
	int i;
	for( i=0; i<NUM_DIRECTIONS; ++i )
		dirSet[i] = i;

	// shuffle dirSet[]
	for( int swapCount=0; swapCount < 3; ++swapCount )
	{
		int swapI = RANDOM_LONG( 0, NUM_DIRECTIONS-1 );
		int nextI = swapI + 1;
		if (nextI >= NUM_DIRECTIONS)
			nextI = 0;

		int tmp = dirSet[nextI];
		dirSet[nextI] = dirSet[swapI];
		dirSet[swapI] = tmp;
	}

	// draw connected areas
	for( i=0; i<NUM_DIRECTIONS; ++i )
	{
		NavDirType dir = (NavDirType)dirSet[i];

		int count = GetAdjacentCount( dir );

		for( int a=0; a<count; ++a )
		{
			CNavArea *adj = GetAdjacentArea( dir, a );

			if (isPlaceMode)
			{
				if (adj->GetPlace() == 0)
					adj->Draw( 50, 0, 0, 3 );
				else if (adj->GetPlace() != ctrl->GetNavPlace())
					adj->Draw( 0, 0, 200, 3 );
				else
					adj->Draw( 0, 255, 0, 3 );
			}
			else
			{
				if (adj->IsDegenerate())
				{
					static IntervalTimer blink;
					static bool blinkOn = false;

					if (blink.GetElapsedTime() > 1.0f)
					{
						blink.Reset();
						blinkOn = !blinkOn;
					}

					if (blinkOn)
						adj->Draw( 255, 255, 255, 3 );
					else
						adj->Draw( 255, 0, 255, 3 );
				}
				else
				{
					adj->Draw( 255, 0, 0, 3 );
				}

				DrawHidingSpots( adj );

				Vector from, to;
				Vector hookPos;
				float halfWidth;
				float size = 5.0f;
				ComputePortal( adj, dir, &hookPos, &halfWidth );

				switch( dir )
				{
					case NORTH:
						from = hookPos + Vector( 0.0f, size, 0.0f );
						to = hookPos + Vector( 0.0f, -size, 0.0f );
						break;
					case SOUTH:
						from = hookPos + Vector( 0.0f, -size, 0.0f );
						to = hookPos + Vector( 0.0f, size, 0.0f );
						break;
					case EAST:
						from = hookPos + Vector( -size, 0.0f, 0.0f );
						to = hookPos + Vector( +size, 0.0f, 0.0f );
						break;
					case WEST:
						from = hookPos + Vector( size, 0.0f, 0.0f );
						to = hookPos + Vector( -size, 0.0f, 0.0f );
						break;
				}

				from.z = GetZ( &from ) + cv_bot_nav_zdraw.value;
				to.z = adj->GetZ( &to ) + cv_bot_nav_zdraw.value;

				Vector drawTo;
				adj->GetClosestPointOnArea( &to, &drawTo );

				if (adj->IsConnected( this, OppositeDirection( dir ) ) )
					UTIL_DrawBeamPoints( from, drawTo, 3, 0, 255, 255 );
				else
					UTIL_DrawBeamPoints( from, drawTo, 3, 0, 0, 255 );
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Raise/lower a corner
 */
void CNavArea::RaiseCorner( NavCornerType corner, int amount )
{
	if ( corner == NUM_CORNERS )
	{
		m_extent.lo.z += amount;
		m_extent.hi.z += amount;
		m_neZ += amount;
		m_swZ += amount;
	}
	else
	{
		switch (corner)
		{
		case NORTH_WEST:
			m_extent.lo.z += amount;
			break;
		case NORTH_EAST:
			m_neZ += amount;
			break;
		case SOUTH_WEST:
			m_swZ += amount;
			break;
		case SOUTH_EAST:
			m_extent.hi.z += amount;
			break;
		}
	}

	m_center.x = (m_extent.lo.x + m_extent.hi.x)/2.0f;
	m_center.y = (m_extent.lo.y + m_extent.hi.y)/2.0f;
	m_center.z = (m_extent.lo.z + m_extent.hi.z)/2.0f;
}

/**
 * Flood fills all areas with current place
 */
class PlaceFloodFillFunctor
{
public:
	PlaceFloodFillFunctor( CNavArea *area )
	{
		m_initialPlace = area->GetPlace();
	}

	bool operator() ( CNavArea *area )
	{
		CCSBotManager *ctrl = static_cast<CCSBotManager *>( TheBots );

		if (area->GetPlace() != m_initialPlace)
			return false;

		area->SetPlace( ctrl->GetNavPlace() );

		return true;
	}

private:
	unsigned int m_initialPlace;
};


//--------------------------------------------------------------------------------------------------------------
/**
 * Draw navigation areas and edit them
 */
void EditNavAreas( NavEditCmdType cmd )
{
	CCSBotManager *ctrl = static_cast<CCSBotManager *>( TheBots );

	CBasePlayer *player = UTIL_GetLocalPlayer();
	if (player == NULL)
		return;

	// don't draw too often on fast video cards or the areas may not appear (odd video effect)
	float drawTimestamp = gpGlobals->time;
	const float maxDrawRate = 0.05f;

	bool doDraw;
	if (drawTimestamp - lastDrawTimestamp < maxDrawRate)
	{
		doDraw = false;
	}
	else
	{
		doDraw = true;
		lastDrawTimestamp = drawTimestamp;
	}


	const float maxRange = 1000.0f;		// 500

	int beamTime = 1;

	if (doDraw)
	{
		// show ladder connections
		for( NavLadderList::iterator iter = TheNavLadderList.begin(); iter != TheNavLadderList.end(); ++iter )
		{
			CNavLadder *ladder = *iter;

			float dx = player->pev->origin.x - ladder->m_bottom.x;
			float dy = player->pev->origin.y - ladder->m_bottom.y;
			if (dx*dx + dy*dy > maxRange*maxRange)
				continue;


			UTIL_DrawBeamPoints( ladder->m_top, ladder->m_bottom, beamTime, 255, 0, 255 );

			Vector bottom = ladder->m_bottom;
			Vector top = ladder->m_top;

			AddDirectionVector( &top, ladder->m_dir, HalfHumanWidth );
			AddDirectionVector( &bottom, ladder->m_dir, HalfHumanWidth );

			UTIL_DrawBeamPoints( top, bottom, beamTime, 0, 0, 255 );

			if (ladder->m_bottomArea)
				UTIL_DrawBeamPoints( bottom + Vector( 0, 0, GenerationStepSize ), *ladder->m_bottomArea->GetCenter(), beamTime, 0, 0, 255 );

			if (ladder->m_topForwardArea)
				UTIL_DrawBeamPoints( top, *ladder->m_topForwardArea->GetCenter(), beamTime, 0, 0, 255 );

			if (ladder->m_topLeftArea)
				UTIL_DrawBeamPoints( top, *ladder->m_topLeftArea->GetCenter(), beamTime, 0, 0, 255 );

			if (ladder->m_topRightArea)
				UTIL_DrawBeamPoints( top, *ladder->m_topRightArea->GetCenter(), beamTime, 0, 0, 255 );

			if (ladder->m_topBehindArea)
				UTIL_DrawBeamPoints( top, *ladder->m_topBehindArea->GetCenter(), beamTime, 0, 0, 255 );
		}

		// draw approach points for marked area
		if (cv_bot_traceview.value == 3 && markedArea)
		{
			Vector ap;
			float halfWidth;
			for( int i=0; i<markedArea->GetApproachInfoCount(); ++i )
			{
				const CNavArea::ApproachInfo *info = markedArea->GetApproachInfo( i );

				// compute approach point
				if (info->hereToNextHow <= GO_WEST)
				{
					info->here.area->ComputePortal( info->next.area, (NavDirType)info->hereToNextHow, &ap, &halfWidth );
					ap.z = info->next.area->GetZ( &ap );
				}
				else
				{
					// use the area's center as an approach point
					ap = *info->here.area->GetCenter();
				}

				UTIL_DrawBeamPoints( ap + Vector( 0, 0, 50 ), ap + Vector( 10, 0, 0 ), beamTime, 255, 100, 0 );
				UTIL_DrawBeamPoints( ap + Vector( 0, 0, 50 ), ap + Vector( -10, 0, 0 ), beamTime, 255, 100, 0 );
				UTIL_DrawBeamPoints( ap + Vector( 0, 0, 50 ), ap + Vector( 0, 10, 0 ), beamTime, 255, 100, 0 );
				UTIL_DrawBeamPoints( ap + Vector( 0, 0, 50 ), ap + Vector( 0, -10, 0 ), beamTime, 255, 100, 0 );
			}
		}
	}

	Vector dir;
	UTIL_MakeVectorsPrivate( player->pev->v_angle, dir, NULL, NULL );

	Vector from = player->pev->origin + player->pev->view_ofs;	// eye position
	Vector to = from + maxRange * dir;

	TraceResult result;
	UTIL_TraceLine( from, to, ignore_monsters, ignore_glass, ENT( player->pev ), &result );

	if (result.flFraction != 1.0f)
	{
		// draw cursor
		Vector cursor = result.vecEndPos;
		float cursorSize = 10.0f;

		if (doDraw)
		{
			UTIL_DrawBeamPoints( cursor + Vector( 0, 0, cursorSize ), cursor, beamTime, 255, 255, 255 );
			UTIL_DrawBeamPoints( cursor + Vector( cursorSize, 0, 0 ), cursor + Vector( -cursorSize, 0, 0 ), beamTime, 255, 255, 255 );
			UTIL_DrawBeamPoints( cursor + Vector( 0, cursorSize, 0 ), cursor + Vector( 0, -cursorSize, 0 ), beamTime, 255, 255, 255 );
			
			// show surface normal
			// UTIL_DrawBeamPoints( cursor + 50.0f * result.vecPlaneNormal, cursor, beamTime, 255, 0, 255 );

		}

		if (isCreatingNavArea)
		{
			if (isAnchored)
			{
				// show drag rectangle
				if (doDraw)
				{
					float z = anchor.z + 2.0f;
					UTIL_DrawBeamPoints( Vector( cursor.x, cursor.y, z ), Vector( anchor.x, cursor.y, z ), beamTime, 0, 255, 255 );
					UTIL_DrawBeamPoints( Vector( anchor.x, anchor.y, z ), Vector( anchor.x, cursor.y, z ), beamTime, 0, 255, 255 );
					UTIL_DrawBeamPoints( Vector( anchor.x, anchor.y, z ), Vector( cursor.x, anchor.y, z ), beamTime, 0, 255, 255 );
					UTIL_DrawBeamPoints( Vector( cursor.x, cursor.y, z ), Vector( cursor.x, anchor.y, z ), beamTime, 0, 255, 255 );
				}
			}
			else
			{
				// anchor starting corner
				anchor = cursor;
				isAnchored = true;
			}
		}

		// find the area the player is pointing at
		CNavArea *area = TheNavAreaGrid.GetNearestNavArea( &result.vecEndPos );

		if (area)
		{
			// if area changed, print its ID
			if (area != lastSelectedArea)
			{
				lastSelectedArea = area;

				char buffer[80];
				char attrib[80];
				char locName[80];

				if (area->GetPlace())
				{
					const char *name = TheBotPhrases->IDToName( area->GetPlace() );
					if (name)
						strcpy( locName, name );
					else
						strcpy( locName, "ERROR" );
				}
				else
				{
					locName[0] = '\000';
				}

				if (isPlaceMode)
				{
					attrib[0] = '\000';
				}
				else
				{
					sprintf( attrib, "%s%s%s%s", 
										(area->GetAttributes() & NAV_CROUCH) ? "CROUCH " : "",
										(area->GetAttributes() & NAV_JUMP) ? "JUMP " : "",
										(area->GetAttributes() & NAV_PRECISE) ? "PRECISE " : "",
										(area->GetAttributes() & NAV_NO_JUMP) ? "NO_JUMP " : "" );
				}

				sprintf( buffer, "Area #%d %s %s\n", area->GetID(), locName, attrib );

				UTIL_SayTextAll( buffer, player );

				// do "place painting"
				if (isPlacePainting)
				{				
					if (area->GetPlace() != ctrl->GetNavPlace())
					{
						area->SetPlace( ctrl->GetNavPlace() );
						EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/lightswitch2.wav", 1, ATTN_NORM, 0, 100 ); 
					}
				}
			}

			if (isPlaceMode)
			{
				area->DrawConnectedAreas();

				switch( cmd )
				{
					case EDIT_TOGGLE_PLACE_MODE:
						EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
						isPlaceMode = false;
						return;

					case EDIT_TOGGLE_PLACE_PAINTING:
					{
						if (isPlacePainting)
						{
							isPlacePainting = false;
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/latchunlocked2.wav", 1, ATTN_NORM, 0, 100 ); 
						}
						else
						{
							isPlacePainting = true;

							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/lightswitch2.wav", 1, ATTN_NORM, 0, 100 ); 

							// paint the initial area
							area->SetPlace( ctrl->GetNavPlace() );
						}
						break;
					}

					case EDIT_PLACE_PICK:
						EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
						ctrl->SetNavPlace( area->GetPlace() );
						break;

					case EDIT_PLACE_FLOODFILL:
						PlaceFloodFillFunctor pff( area );
						SearchSurroundingAreas( area, area->GetCenter(), pff );
						break;
				}
			}
			else	// normal editing mode
			{
				// draw the "marked" area
				if (markedArea && doDraw)
				{
					markedArea->Draw( 0, 255, 255, beamTime );
					if ( markedCorner != NUM_CORNERS )
						markedArea->DrawMarkedCorner( markedCorner, 0, 0, 255, beamTime );

					if (cv_bot_traceview.value == 11)
					{
						// draw areas connected to the marked area
						markedArea->DrawConnectedAreas();
					}
				}


				// draw split line
				const Extent *extent = area->GetExtent();

				float yaw = player->pev->v_angle.y;
				while( yaw > 360.0f )
					yaw -= 360.0f;

				while( yaw < 0.0f )
					yaw += 360.0f;

				float splitEdge;
				bool splitAlongX;

				if ((yaw < 45.0f || yaw > 315.0f) || (yaw > 135.0f && yaw < 225.0f))
				{
					splitEdge = GenerationStepSize * (int)(result.vecEndPos.y/GenerationStepSize);

					from.x = extent->lo.x;
					from.y = splitEdge;
					from.z = area->GetZ( &from ) + cv_bot_nav_zdraw.value;

					to.x = extent->hi.x;
					to.y = splitEdge;
					to.z = area->GetZ( &to ) + cv_bot_nav_zdraw.value;

					splitAlongX = true;
				}
				else
				{
					splitEdge = GenerationStepSize * (int)(result.vecEndPos.x/GenerationStepSize);

					from.x = splitEdge;
					from.y = extent->lo.y;
					from.z = area->GetZ( &from ) + cv_bot_nav_zdraw.value;

					to.x = splitEdge;
					to.y = extent->hi.y;
					to.z = area->GetZ( &to ) + cv_bot_nav_zdraw.value;

					splitAlongX = false;
				}

				if (doDraw)
					UTIL_DrawBeamPoints( from, to, beamTime, 255, 255, 255 );

				// draw the area we are pointing at and all connected areas
				if (doDraw && (cv_bot_traceview.value != 11 || markedArea == NULL))
					area->DrawConnectedAreas();


				// do area-dependant edit commands, if any
				switch( cmd )
				{
					case EDIT_TOGGLE_PLACE_MODE:
						EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
						isPlaceMode = true;
						return;

					case EDIT_DELETE:
						EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
						TheNavAreaList.remove( area );
						delete area;
						return;

					case EDIT_ATTRIB_CROUCH:
						EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/bell1.wav", 1, ATTN_NORM, 0, 100 ); 
						area->SetAttributes( area->GetAttributes() ^ NAV_CROUCH );
						break;

					case EDIT_ATTRIB_JUMP:
						EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/bell1.wav", 1, ATTN_NORM, 0, 100 ); 
						area->SetAttributes( area->GetAttributes() ^ NAV_JUMP );
						break;

					case EDIT_ATTRIB_PRECISE:
						EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/bell1.wav", 1, ATTN_NORM, 0, 100 ); 
						area->SetAttributes( area->GetAttributes() ^ NAV_PRECISE );
						break;

					case EDIT_ATTRIB_NO_JUMP:
						EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/bell1.wav", 1, ATTN_NORM, 0, 100 ); 
						area->SetAttributes( area->GetAttributes() ^ NAV_NO_JUMP );
						break;

					case EDIT_SPLIT:
						if (area->SplitEdit( splitAlongX, splitEdge ))
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "weapons/knife_hitwall1.wav", 1, ATTN_NORM, 0, 100 ); 
						else
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 );
						break;

					case EDIT_MERGE:
						if (markedArea)
						{
							if (area->MergeEdit( markedArea ))
								EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
							else
								EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 );
						}
						else
						{
							HintMessageToAllPlayers( "To merge, mark an area, highlight a second area, then invoke the merge command" );
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 ); 
						}
						break;

					case EDIT_MARK:
						if (markedArea)
						{
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
							markedArea = NULL;
						}
						else
						{
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip2.wav", 1, ATTN_NORM, 0, 100 ); 
							markedArea = area;

							int connected = 0;
							connected += markedArea->GetAdjacentCount( NORTH );
							connected += markedArea->GetAdjacentCount( SOUTH );
							connected += markedArea->GetAdjacentCount( EAST );
							connected += markedArea->GetAdjacentCount( WEST );

							char buffer[80];
							sprintf( buffer, "Marked Area is connected to %d other Areas\n", connected );
							UTIL_SayTextAll( buffer, player );
						}
						break;

					case EDIT_MARK_UNNAMED:
						if (markedArea)
						{
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
							markedArea = NULL;
						}
						else
						{
							markedArea = NULL;
							for( NavAreaList::iterator iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
							{
								CNavArea *area = *iter;
								if ( area->GetPlace() == 0 )
								{
									markedArea = area;
									break;
								}
							}
							if ( !markedArea )
							{
								EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
							}
							else
							{
								EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip2.wav", 1, ATTN_NORM, 0, 100 ); 

								int connected = 0;
								connected += markedArea->GetAdjacentCount( NORTH );
								connected += markedArea->GetAdjacentCount( SOUTH );
								connected += markedArea->GetAdjacentCount( EAST );
								connected += markedArea->GetAdjacentCount( WEST );

								int totalUnnamedAreas = 0;
								for( NavAreaList::iterator iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
								{
									CNavArea *area = *iter;
									if ( area->GetPlace() == 0 )
									{
										++totalUnnamedAreas;
									}
								}

								char buffer[80];
								sprintf( buffer, "Marked Area is connected to %d other Areas - there are %d total unnamed areas\n", connected, totalUnnamedAreas );
								UTIL_SayTextAll( buffer, player );
							}
						}
						break;

					case EDIT_WARP_TO_MARK:
						if (markedArea)
						{
							CBasePlayer *pLocalPlayer = UTIL_GetLocalPlayer();
							if ( pLocalPlayer && pLocalPlayer->m_iTeam == SPECTATOR && pLocalPlayer->pev->iuser1 == OBS_ROAMING )
							{
								Vector origin = *markedArea->GetCenter() + Vector( 0, 0, 0.75f * HumanHeight );
								UTIL_SetOrigin( pLocalPlayer->pev, origin );
							}
						}
						else
						{
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 ); 
						}
						break;

					case EDIT_CONNECT:
						if (markedArea)
						{
							NavDirType dir = markedArea->ComputeDirection( &cursor );
							if (dir == NUM_DIRECTIONS)
							{
								EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 ); 
							}
							else
							{
								markedArea->ConnectTo( area, dir );
								EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
							}
						}
						else
						{
							HintMessageToAllPlayers( "To connect areas, mark an area, highlight a second area, then invoke the connect command. Make sure the cursor is directly north, south, east, or west of the marked area." );
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 ); 
						}
						break;

					case EDIT_DISCONNECT:
						if (markedArea)
						{
							markedArea->Disconnect( area );
							area->Disconnect( markedArea );
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
						}
						else
						{
							HintMessageToAllPlayers( "To disconnect areas, mark an area, highlight a second area, then invoke the disconnect command. This will remove all connections between the two areas." );
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 ); 
						}
						break;

					case EDIT_SPLICE:
						if (markedArea)
						{
							if (area->SpliceEdit( markedArea ))
								EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
							else
								EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 );
						}
						else
						{
							HintMessageToAllPlayers( "To splice, mark an area, highlight a second area, then invoke the splice command to create an area between them" );
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 ); 
						}
						break;

					case EDIT_SELECT_CORNER:
						if (markedArea)
						{
							int corner = (markedCorner + 1) % (NUM_CORNERS + 1);
							markedCorner = (NavCornerType)corner;
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
						}
						else
						{
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 ); 
						}
						break;

					case EDIT_RAISE_CORNER:
						if (markedArea)
						{
							markedArea->RaiseCorner( markedCorner, 1 );
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
						}
						else
						{
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 ); 
						}
						break;

					case EDIT_LOWER_CORNER:
						if (markedArea)
						{
							markedArea->RaiseCorner( markedCorner, -1 );
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 
						}
						else
						{
							EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 ); 
						}
						break;
				}
			}
		}

		// do area-independant edit commands, if any
		switch( cmd )
		{
			case EDIT_BEGIN_AREA:
			{
				if (isCreatingNavArea)
				{
					isCreatingNavArea = false;
					EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 );
				}
				else
				{
					EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip2.wav", 1, ATTN_NORM, 0, 100 ); 
					isCreatingNavArea = true;
					isAnchored = false;
				}
				break;
			}

			case EDIT_END_AREA:
			{
				if (isCreatingNavArea)
				{
					// create the new nav area
					CNavArea *newArea = new CNavArea( &anchor, &cursor );
					TheNavAreaList.push_back( newArea );
					TheNavAreaGrid.AddNavArea( newArea );
					EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/blip1.wav", 1, ATTN_NORM, 0, 100 ); 

					// if we have a marked area, inter-connect the two
					if (markedArea)
					{
						const Extent *extent = markedArea->GetExtent();

						if (anchor.x > extent->hi.x && cursor.x > extent->hi.x)
						{
							markedArea->ConnectTo( newArea, EAST );
							newArea->ConnectTo( markedArea, WEST );
						}
						else if (anchor.x < extent->lo.x && cursor.x < extent->lo.x)
						{
							markedArea->ConnectTo( newArea, WEST );
							newArea->ConnectTo( markedArea, EAST );
						}
						else if (anchor.y > extent->hi.y && cursor.y > extent->hi.y)
						{
							markedArea->ConnectTo( newArea, SOUTH );
							newArea->ConnectTo( markedArea, NORTH );
						}
						else if (anchor.y < extent->lo.y && cursor.y < extent->lo.y)
						{
							markedArea->ConnectTo( newArea, NORTH );
							newArea->ConnectTo( markedArea, SOUTH );
						}

						// propogate marked area to new area
						markedArea = newArea;
					}

					isCreatingNavArea = false;
				}
				else
				{
					EMIT_SOUND_DYN( ENT(UTIL_GetLocalPlayer()->pev), CHAN_ITEM, "buttons/button11.wav", 1, ATTN_NORM, 0, 100 );
				}
				break;
			}
		}
	}


	// if our last command was not mark (or no command), clear the mark area
	if (cmd != EDIT_MARK && cmd != EDIT_BEGIN_AREA && cmd != EDIT_END_AREA &&
		cmd != EDIT_MARK_UNNAMED && cmd != EDIT_WARP_TO_MARK &&
		cmd != EDIT_SELECT_CORNER && cmd != EDIT_RAISE_CORNER && cmd != EDIT_LOWER_CORNER &&
		cmd != EDIT_NONE)
		markedArea = NULL;

	// if our last command was not affecting the corner, clear the corner selection
	if (cmd != EDIT_SELECT_CORNER && cmd != EDIT_RAISE_CORNER && cmd != EDIT_LOWER_CORNER && cmd != EDIT_NONE)
		markedCorner = NUM_CORNERS;


	if (isCreatingNavArea && cmd != EDIT_BEGIN_AREA && cmd != EDIT_END_AREA && cmd != EDIT_NONE)
		isCreatingNavArea = false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the ground height below this point in "height".
 * Return false if position is invalid (outside of map, in a solid area, etc).
 */
bool GetGroundHeight( const Vector *pos, float *height, Vector *normal )
{
	Vector to;
	to.x = pos->x;
	to.y = pos->y;
	to.z = pos->z - 9999.9f;

	float offset;
	Vector from;
	TraceResult result;
	edict_t *ignore = NULL;
	float ground = 0.0f;

	const float maxOffset = 100.0f;
	const float inc = 10.0f;

	#define MAX_GROUND_LAYERS 16
	struct GroundLayerInfo
	{
		float ground;
		Vector normal;
	}
	layer[ MAX_GROUND_LAYERS ];
	int layerCount = 0;

	for( offset = 1.0f; offset < maxOffset; offset += inc )
	{
		from = *pos + Vector( 0, 0, offset );

		UTIL_TraceLine( from, to, ignore_monsters, dont_ignore_glass, ignore, &result );

		// if the trace came down thru a door, ignore the door and try again
		// also ignore breakable floors
		if (result.pHit)
		{
			if (FClassnameIs( VARS( result.pHit ), "func_door" ) ||
				FClassnameIs( VARS( result.pHit ), "func_door_rotating" ) ||
				(FClassnameIs( VARS( result.pHit ), "func_breakable" ) && VARS( result.pHit )->takedamage == DAMAGE_YES))
			{
				ignore = result.pHit;
				// keep incrementing to avoid infinite loop if more than one entity is along the traceline...
				/// @todo Deal with multiple ignore entities in a single TraceLine()
				//offset -= inc;
				continue;
			}
		}

		if (result.fStartSolid == false)
		{
			// if we didnt start inside a solid area, the trace hit a ground layer

			// if this is a new ground layer, add it to the set
			if (layerCount == 0 || result.vecEndPos.z > layer[ layerCount-1 ].ground)
			{
				layer[ layerCount ].ground = result.vecEndPos.z;
				layer[ layerCount ].normal = result.vecPlaneNormal;
				++layerCount;
						
				if (layerCount == MAX_GROUND_LAYERS)
					break;
			}
		}
	}

	if (layerCount == 0)
		return false;

	// find the lowest layer that allows a player to stand or crouch upon it
	int i;
	for( i=0; i<layerCount-1; ++i )
	{
		if (layer[i+1].ground - layer[i].ground >= HalfHumanHeight)
			break;		
	}

	*height = layer[ i ].ground;

	if (normal)
		*normal = layer[ i ].normal;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the "simple" ground height below this point in "height".
 * This function is much faster, but less tolerant. Make sure the give position is "well behaved".
 * Return false if position is invalid (outside of map, in a solid area, etc).
 */
bool GetSimpleGroundHeight( const Vector *pos, float *height, Vector *normal )
{
	Vector to;
	to.x = pos->x;
	to.y = pos->y;
	to.z = pos->z - 9999.9f;

	TraceResult result;

	UTIL_TraceLine( *pos, to, ignore_monsters, dont_ignore_glass, NULL, &result );

	if (result.fStartSolid)
		return false;

	*height = result.vecEndPos.z;

	if (normal)
		*normal = result.vecPlaneNormal;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
enum { MAX_BLOCKED_AREAS = 256 };
static unsigned int BlockedID[ MAX_BLOCKED_AREAS ];
static int BlockedIDCount = 0;

/**
 * Shortest path cost, paying attention to "blocked" areas
 */
class ApproachAreaCost
{
public:
	float operator() ( CNavArea *area, CNavArea *fromArea, const CNavLadder *ladder )
	{
		// check if this area is "blocked"
		for( int i=0; i<BlockedIDCount; ++i )
			if (area->GetID() == BlockedID[i])
				return -1.0f;

		if (fromArea == NULL)
		{
			// first area in path, no cost
			return 0.0f;
		}
		else
		{
			// compute distance travelled along path so far
			float dist;

			if (ladder)
				dist = ladder->m_length;
			else
				dist = (*area->GetCenter() - *fromArea->GetCenter()).Length();

			float cost = dist + fromArea->GetCostSoFar();

			return cost;
		}
	}
};

/**
 * Can we see this area?
 * For now, if we can see any corner, we can see the area
 * @todo Need to check LOS to more than the corners for large and/or long areas
 */
inline bool IsAreaVisible( const Vector *pos, const CNavArea *area )
{
	Vector corner;
	TraceResult result;

	for( int c=0; c<NUM_CORNERS; ++c )
	{
		corner = *area->GetCorner( (NavCornerType)c );
		corner.z += 0.75f * HumanHeight;

		UTIL_TraceLine( *pos, corner, ignore_monsters, NULL, &result );
		if (result.flFraction == 1.0f)
		{
			// we can see this area
			return true;
		}
	}

	return false;
}

/**
 * Determine the set of "approach areas".
 * An approach area is an area representing a place where players 
 * move into/out of our local neighborhood of areas.
 */
void CNavArea::ComputeApproachAreas( void )
{
	m_approachCount = 0;

	if (cv_bot_quicksave.value > 0.0f)
		return;

	// use the center of the nav area as the "view" point
	Vector eye = m_center;
	if (GetGroundHeight( &eye, &eye.z ) == false)
		return;

	// approximate eye position
	if (GetAttributes() & NAV_CROUCH)
		eye.z += 0.9f * HalfHumanHeight;
	else
		eye.z += 0.9f * HumanHeight;

	enum { MAX_PATH_LENGTH = 256 };
	CNavArea *path[ MAX_PATH_LENGTH ];

	//
	// In order to enumerate all of the approach areas, we need to
	// run the algorithm many times, once for each "far away" area
	// and keep the union of the approach area sets
	//
	NavAreaList::iterator iter;
	for( iter = goodSizedAreaList.begin(); iter != goodSizedAreaList.end(); ++iter )
	{
		CNavArea *farArea = *iter;

		BlockedIDCount = 0;

		// if we can see 'farArea', try again - the whole point is to go "around the bend", so to speak
		if (IsAreaVisible( &eye, farArea ))
			continue;
	
		// make first path to far away area
		ApproachAreaCost cost;
		if (NavAreaBuildPath( this, farArea, NULL, cost ) == false)
			continue;

		//
		// Keep building paths to farArea and blocking them off until we
		// cant path there any more.
		// As areas are blocked off, all exits will be enumerated.
		//
		while( m_approachCount < MAX_APPROACH_AREAS )
		{
			// find number of areas on path
			int count = 0;
			CNavArea *area;
			for( area = farArea; area; area = area->GetParent() )
				++count;

			if (count > MAX_PATH_LENGTH)
				count = MAX_PATH_LENGTH;

			// build path in correct order - from eye outwards
			int i = count;
			for( area = farArea; i && area; area = area->GetParent() )
				path[ --i ] = area;

			// traverse path to find first area we cannot see (skip the first area)
			for( i=1; i<count; ++i )
			{
				// if we see this area, continue on
				if (IsAreaVisible( &eye, path[i] ))
					continue;

				// we can't see this area.
				// mark this area as "blocked" and unusable by subsequent approach paths
				if (BlockedIDCount == MAX_BLOCKED_AREAS)
				{
					CONSOLE_ECHO( "Overflow computing approach areas for area #%d.\n", m_id );
					return;
				}

				// if the area to be blocked is actually farArea, block the one just prior
				// (blocking farArea will cause all subsequent pathfinds to fail)
				int block = (path[i] == farArea) ? i-1 : i;

				BlockedID[ BlockedIDCount++ ] = path[ block ]->GetID();

				if (block == 0)
					break;

				// store new approach area if not already in set
				int a;
				for( a=0; a<m_approachCount; ++a )
					if (m_approach[a].here.area == path[block-1])
						break;

				if (a == m_approachCount)
				{
					m_approach[ m_approachCount ].prev.area = (block >= 2) ? path[block-2] : NULL;

					m_approach[ m_approachCount ].here.area = path[block-1];
					m_approach[ m_approachCount ].prevToHereHow = path[block-1]->GetParentHow();

					m_approach[ m_approachCount ].next.area = path[block];
					m_approach[ m_approachCount ].hereToNextHow = path[block]->GetParentHow();

					++m_approachCount;
				}

				// we are done with this path
				break;
			}

			// find another path to 'farArea'
			ApproachAreaCost cost;
			if (NavAreaBuildPath( this, farArea, NULL, cost ) == false)
			{
				// can't find a path to 'farArea' means all exits have been already tested and blocked
				break;
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------------------

/**
 * The singleton for accessing the grid
 */
CNavAreaGrid TheNavAreaGrid;


CNavAreaGrid::CNavAreaGrid( void ) : m_cellSize( 300.0f )
{
	m_grid = NULL;
	Reset();
}

CNavAreaGrid::~CNavAreaGrid()
{
	delete [] m_grid;
	m_grid = NULL;
}

/**
 * Clear the grid
 */
void CNavAreaGrid::Reset( void )
{
	if (m_grid)
		delete [] m_grid;

	m_grid = NULL;
	m_gridSizeX = 0;
	m_gridSizeY = 0;

	// clear the hash table
	for( int i=0; i<HASH_TABLE_SIZE; ++i )
		m_hashTable[i] = NULL;

	m_areaCount = 0;

	EditNavAreasReset(); // reset static vars
}

/**
 * Allocate the grid and define its extents
 */
void CNavAreaGrid::Initialize( float minX, float maxX, float minY, float maxY )
{
	if (m_grid)
		Reset();

	m_minX = minX;
	m_minY = minY;

	m_gridSizeX = ((maxX - minX) / m_cellSize) + 1;
	m_gridSizeY = ((maxY - minY) / m_cellSize) + 1;

	m_grid = new NavAreaList[ m_gridSizeX * m_gridSizeY ];
}

/**
 * Add an area to the grid
 */
void CNavAreaGrid::AddNavArea( CNavArea *area )
{
	// add to grid
	const Extent *extent = area->GetExtent();

	int loX = WorldToGridX( extent->lo.x );
	int loY = WorldToGridY( extent->lo.y );
	int hiX = WorldToGridX( extent->hi.x );
	int hiY = WorldToGridY( extent->hi.y );

	for( int y = loY; y <= hiY; ++y )
		for( int x = loX; x <= hiX; ++x )
			m_grid[ x + y*m_gridSizeX ].push_back( const_cast<CNavArea *>( area ) );

	// add to hash table
	int key = ComputeHashKey( area->GetID() );

	if (m_hashTable[key])
	{
		// add to head of list in this slot
		area->m_prevHash = NULL;
		area->m_nextHash = m_hashTable[key];
		m_hashTable[key]->m_prevHash = area;
		m_hashTable[key] = area;
	}
	else
	{
		// first entry in this slot
		m_hashTable[key] = area;
		area->m_nextHash = NULL;
		area->m_prevHash = NULL;
	}

	++m_areaCount;
}

/**
 * Remove an area from the grid
 */
void CNavAreaGrid::RemoveNavArea( CNavArea *area )
{
	// add to grid
	const Extent *extent = area->GetExtent();

	int loX = WorldToGridX( extent->lo.x );
	int loY = WorldToGridY( extent->lo.y );
	int hiX = WorldToGridX( extent->hi.x );
	int hiY = WorldToGridY( extent->hi.y );

	for( int y = loY; y <= hiY; ++y )
		for( int x = loX; x <= hiX; ++x )
			m_grid[ x + y*m_gridSizeX ].remove( area );

	// remove from hash table
	int key = ComputeHashKey( area->GetID() );

	if (area->m_prevHash)
	{
		area->m_prevHash->m_nextHash = area->m_nextHash;
	}
	else
	{
		// area was at start of list
		m_hashTable[key] = area->m_nextHash;

		if (m_hashTable[key])
			m_hashTable[key]->m_prevHash = NULL;
	}

	if (area->m_nextHash)
	{
		area->m_nextHash->m_prevHash = area->m_prevHash;
	}

	--m_areaCount;
}

/**
 * Given a position, return the nav area that IsOverlapping and is *immediately* beneath it
 */
CNavArea *CNavAreaGrid::GetNavArea( const Vector *pos, float beneathLimit ) const
{
	if (m_grid == NULL)
		return NULL;

	// get list in cell that contains position
	int x = WorldToGridX( pos->x );
	int y = WorldToGridY( pos->y );
	NavAreaList *list = &m_grid[ x + y*m_gridSizeX ];


	// search cell list to find correct area
	CNavArea *use = NULL;
	float useZ = -99999999.9f;
	Vector testPos = *pos + Vector( 0, 0, 5 );

	for( NavAreaList::iterator iter = list->begin(); iter != list->end(); ++iter )
	{
		CNavArea *area = *iter;

		// check if position is within 2D boundaries of this area
		if (area->IsOverlapping( &testPos ))
		{
			// project position onto area to get Z
			float z = area->GetZ( &testPos );

			// if area is above us, skip it
			if (z > testPos.z)
				continue;

			// if area is too far below us, skip it
			if (z < pos->z - beneathLimit)
				continue;

			// if area is higher than the one we have, use this instead
			if (z > useZ)
			{
				use = area;
				useZ = z;
			}
		}
	}

	return use;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Given a position in the world, return the nav area that is closest
 * and at the same height, or beneath it.
 * Used to find initial area if we start off of the mesh.
 */
CNavArea *CNavAreaGrid::GetNearestNavArea( const Vector *pos, bool anyZ ) const
{
	if (m_grid == NULL)
		return NULL;

	CNavArea *close = NULL;
	float closeDistSq = 99999999.9f;

	// quick check
	close = GetNavArea( pos );
	if (close)
		return close;

	// ensure source position is well behaved
	Vector source;
	source.x = pos->x;
	source.y = pos->y;
	if (GetGroundHeight( pos, &source.z ) == false)
		return NULL;

	source.z += HalfHumanHeight;

	/// @todo Step incrementally using grid for speed

	// find closest nav area
	for( NavAreaList::iterator iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
	{
		CNavArea *area = *iter;

		Vector areaPos;
		area->GetClosestPointOnArea( &source, &areaPos );

		float distSq = (areaPos - source).LengthSquared();

		// keep the closest area
		if (distSq < closeDistSq)
		{
			// check LOS to area
			if (!anyZ)
			{
				TraceResult result;
				UTIL_TraceLine( source, areaPos + Vector( 0, 0, HalfHumanHeight ), ignore_monsters, ignore_glass, NULL, &result );
				if (result.flFraction != 1.0f)
					continue;
			}
					
			closeDistSq = distSq;
			close = area;
		}
	}

	return close;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Given an ID, return the associated area
 */
CNavArea *CNavAreaGrid::GetNavAreaByID( unsigned int id ) const
{
	if (id == 0)
		return NULL;

	int key = ComputeHashKey( id );

	for( CNavArea *area = m_hashTable[key]; area; area = area->m_nextHash )
		if (area->GetID() == id)
			return area;

	return NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return radio chatter place for given coordinate
 */
unsigned int CNavAreaGrid::GetPlace( const Vector *pos ) const
{
	CNavArea *area = GetNearestNavArea( pos, true );

	if (area)
		return area->GetPlace();

	return UNDEFINED_PLACE;
}


