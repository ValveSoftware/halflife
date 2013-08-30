// nav_path.cpp
// Encapsulation of a path through space
// Author: Michael S. Booth (mike@turtlerockstudios.com), November 2003

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "soundent.h"
#include "gamerules.h"
#include "player.h"
#include "client.h"
#include "cmd.h"

#include "nav.h"
#include "nav_path.h"
#include "bot_util.h"
#include "improv.h"

//--------------------------------------------------------------------------------------------------------------
/**
 * Determine actual path positions
 */
bool CNavPath::ComputePathPositions( void )
{
	if (m_segmentCount == 0)
		return false;

	// start in first area's center
	m_path[0].pos = *m_path[0].area->GetCenter();
	m_path[0].ladder = NULL;
	m_path[0].how = NUM_TRAVERSE_TYPES;

	for( int i=1; i<m_segmentCount; ++i )
	{
		const PathSegment *from = &m_path[ i-1 ];
		PathSegment *to = &m_path[ i ];

		if (to->how <= GO_WEST)		// walk along the floor to the next area
		{
			to->ladder = NULL;

			// compute next point, keeping path as straight as possible
			from->area->ComputeClosestPointInPortal( to->area, (NavDirType)to->how, &from->pos, &to->pos );

			// move goal position into the goal area a bit
			const float stepInDist = 5.0f;		// how far to "step into" an area - must be less than min area size
			AddDirectionVector( &to->pos, (NavDirType)to->how, stepInDist );

			// we need to walk out of "from" area, so keep Z where we can reach it
			to->pos.z = from->area->GetZ( &to->pos );

			// if this is a "jump down" connection, we must insert an additional point on the path
			if (to->area->IsConnected( from->area, NUM_DIRECTIONS ) == false)
			{
				// this is a "jump down" link

				// compute direction of path just prior to "jump down"
				Vector2D dir;
				DirectionToVector2D( (NavDirType)to->how, &dir );

				// shift top of "jump down" out a bit to "get over the ledge"
				const float pushDist = 25.0f;
				to->pos.x += pushDist * dir.x;
				to->pos.y += pushDist * dir.y;

				// insert a duplicate node to represent the bottom of the fall
				if (m_segmentCount < MAX_PATH_SEGMENTS-1)
				{
					// copy nodes down
					for( int j=m_segmentCount; j>i; --j )
						m_path[j] = m_path[j-1];

					// path is one node longer
					++m_segmentCount;

					// move index ahead into the new node we just duplicated
					++i;

					m_path[i].pos.x = to->pos.x + pushDist * dir.x;
					m_path[i].pos.y = to->pos.y + pushDist * dir.y;

					// put this one at the bottom of the fall
					m_path[i].pos.z = to->area->GetZ( &m_path[i].pos );
				}
			}
		}
		else if (to->how == GO_LADDER_UP)		// to get to next area, must go up a ladder
		{
			// find our ladder
			const NavLadderList *list = from->area->GetLadderList( LADDER_UP );
			NavLadderList::const_iterator iter;
			for( iter = list->begin(); iter != list->end(); ++iter )
			{
				CNavLadder *ladder = *iter;

				// can't use "behind" area when ascending...
				if (ladder->m_topForwardArea == to->area ||
						ladder->m_topLeftArea == to->area ||
						ladder->m_topRightArea == to->area)
				{
					to->ladder = ladder;
					to->pos = ladder->m_bottom;
					AddDirectionVector( &to->pos, ladder->m_dir, 2.0f*HalfHumanWidth );
					break;
				}
			}

			if (iter == list->end())
			{
				//PrintIfWatched( "ERROR: Can't find ladder in path\n" );
				return false;
			}
		}
		else if (to->how == GO_LADDER_DOWN)		// to get to next area, must go down a ladder
		{
			// find our ladder
			const NavLadderList *list = from->area->GetLadderList( LADDER_DOWN );
			NavLadderList::const_iterator iter;
			for( iter = list->begin(); iter != list->end(); ++iter )
			{
				CNavLadder *ladder = *iter;

				if (ladder->m_bottomArea == to->area)
				{
					to->ladder = ladder;
					to->pos = ladder->m_top;
					AddDirectionVector( &to->pos, OppositeDirection( ladder->m_dir ), 2.0f*HalfHumanWidth );
					break;
				}
			}

			if (iter == list->end())
			{
				//PrintIfWatched( "ERROR: Can't find ladder in path\n" );
				return false;
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if position is at the end of the path
 */
bool CNavPath::IsAtEnd( const Vector &pos ) const
{
	if (!IsValid())
		return false;

	const float epsilon = 20.0f;
	return (pos - GetEndpoint()).IsLengthLessThan( epsilon );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return length of path from start to finish
 */
float CNavPath::GetLength( void ) const
{
	float length = 0.0f;
	for( int i=1; i<GetSegmentCount(); ++i )
	{
		length += (m_path[i].pos - m_path[i-1].pos).Length();
	}

	return length;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return point a given distance along the path - if distance is out of path bounds, point is clamped to start/end
 * @todo Be careful of returning "positions" along one-way drops, ladders, etc.
 */
bool CNavPath::GetPointAlongPath( float distAlong, Vector *pointOnPath ) const
{
	if (!IsValid() || pointOnPath == NULL)
		return false;

	if (distAlong <= 0.0f)
	{
		*pointOnPath = m_path[0].pos;
		return true;
	}

	float lengthSoFar = 0.0f;
	float segmentLength;
	Vector dir;
	for( int i=1; i<GetSegmentCount(); ++i )
	{
		dir = m_path[i].pos - m_path[i-1].pos;
		segmentLength = dir.Length();

		if (segmentLength + lengthSoFar >= distAlong)
		{
			// desired point is on this segment of the path
			float delta = distAlong - lengthSoFar;
			float t = delta / segmentLength;

			*pointOnPath = m_path[i].pos + t * dir;

			return true;
		}

		lengthSoFar += segmentLength;
	}

	*pointOnPath = m_path[ GetSegmentCount()-1 ].pos;
	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the node index closest to the given distance along the path without going over - returns (-1) if error
 */
int CNavPath::GetSegmentIndexAlongPath( float distAlong ) const
{
	if (!IsValid())
		return -1;

	if (distAlong <= 0.0f)
	{
		return 0;
	}

	float lengthSoFar = 0.0f;
	Vector dir;
	for( int i=1; i<GetSegmentCount(); ++i )
	{
		lengthSoFar += (m_path[i].pos - m_path[i-1].pos).Length();

		if (lengthSoFar > distAlong)
		{
			return i-1;
		}
	}

	return GetSegmentCount()-1;
}



//--------------------------------------------------------------------------------------------------------------
/**
 * Compute closest point on path to given point
 * NOTE: This does not do line-of-sight tests, so closest point may be thru the floor, etc
 */
bool CNavPath::FindClosestPointOnPath( const Vector *worldPos, int startIndex, int endIndex, Vector *close ) const
{
	if (!IsValid() || close == NULL)
		return false;

	Vector along, toWorldPos;
	Vector pos;
	const Vector *from, *to;
	float length;
	float closeLength;
	float closeDistSq = 9999999999.9;
	float distSq;

	for( int i=startIndex; i<=endIndex; ++i )
	{
		from = &m_path[i-1].pos;
		to = &m_path[i].pos;

		// compute ray along this path segment
		along = *to - *from;

		// make it a unit vector along the path
		length = along.NormalizeInPlace();

		// compute vector from start of segment to our point
		toWorldPos = *worldPos - *from;

		// find distance of closest point on ray
		closeLength = DotProduct( toWorldPos, along );

		// constrain point to be on path segment
		if (closeLength <= 0.0f)
			pos = *from;
		else if (closeLength >= length)
			pos = *to;
		else
			pos = *from + closeLength * along;

		distSq = (pos - *worldPos).LengthSquared();

		// keep the closest point so far
		if (distSq < closeDistSq)
		{
			closeDistSq = distSq;
			*close = pos;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Build trivial path when start and goal are in the same nav area
 */
bool CNavPath::BuildTrivialPath( const Vector *start, const Vector *goal )
{
	m_segmentCount = 0;

	CNavArea *startArea = TheNavAreaGrid.GetNearestNavArea( start );
	if (startArea == NULL)
		return false;

	CNavArea *goalArea = TheNavAreaGrid.GetNearestNavArea( goal );
	if (goalArea == NULL)
		return false;

	m_segmentCount = 2;

	m_path[0].area = startArea;
	m_path[0].pos.x = start->x;
	m_path[0].pos.y = start->y;
	m_path[0].pos.z = startArea->GetZ( start );
	m_path[0].ladder = NULL;
	m_path[0].how = NUM_TRAVERSE_TYPES;

	m_path[1].area = goalArea;
	m_path[1].pos.x = goal->x;
	m_path[1].pos.y = goal->y;
	m_path[1].pos.z = goalArea->GetZ( goal );
	m_path[1].ladder = NULL;
	m_path[1].how = NUM_TRAVERSE_TYPES;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Draw the path for debugging.
 */
void CNavPath::Draw( void )
{
	if (!IsValid())
		return;

	for( int i=1; i<m_segmentCount; ++i )
		UTIL_DrawBeamPoints( m_path[i-1].pos + Vector( 0, 0, HalfHumanHeight ), 
												 m_path[i].pos + Vector( 0, 0, HalfHumanHeight ), 2, 255, 75, 0 );
}



//--------------------------------------------------------------------------------------------------------------
/**
 * Check line of sight from 'anchor' node on path to subsequent nodes until
 * we find a node that can't been seen from 'anchor'.
 */
int CNavPath::FindNextOccludedNode( int anchor )
{
	int lastVisible = anchor;
	for( int i=anchor+1; i<m_segmentCount; ++i )
	{
		// don't remove ladder nodes
		if (m_path[i].ladder)
			return i;

		if (!IsWalkableTraceLineClear( m_path[ anchor ].pos, m_path[ i ].pos ))
		{
			// cant see this node from anchor node
			return i;
		}

		Vector anchorPlusHalf =  m_path[ anchor ].pos + Vector( 0, 0, HalfHumanHeight );
		Vector iPlusHalf =  m_path[ i ].pos +Vector( 0, 0, HalfHumanHeight );
		if (!IsWalkableTraceLineClear( anchorPlusHalf, iPlusHalf) )
		{
			// cant see this node from anchor node
			return i;
		}

		Vector anchorPlusFull =  m_path[ anchor ].pos + Vector( 0, 0, HumanHeight );
		Vector iPlusFull = m_path[ i ].pos + Vector( 0, 0, HumanHeight );
		if (!IsWalkableTraceLineClear( anchorPlusFull, iPlusFull ))
		{
			// cant see this node from anchor node
			return i;
		}
	}

	return m_segmentCount;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Smooth out path, removing redundant nodes
 */
void CNavPath::Optimize( void )
{
// DONT USE THIS: Optimizing the path results in cutting thru obstacles
return;

	if (m_segmentCount < 3)
		return;

	int anchor = 0;

	while( anchor < m_segmentCount )
	{
		int occluded = FindNextOccludedNode( anchor );
		int nextAnchor = occluded-1;

		if (nextAnchor > anchor)
		{
			// remove redundant nodes between anchor and nextAnchor
			int removeCount = nextAnchor - anchor - 1;
			if (removeCount > 0)
			{
				for( int i=nextAnchor; i<m_segmentCount; ++i )
				{
					m_path[i-removeCount] = m_path[i];
				}
				m_segmentCount -= removeCount;
			}
		}

		++anchor;
	}
}


//--------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------

/**
 * Constructor
 */
CNavPathFollower::CNavPathFollower( void )
{
	m_improv = NULL;
	m_path = NULL;

	m_segmentIndex = 0;
	m_isLadderStarted = false;

	m_isDebug = false;
}

void CNavPathFollower::Reset( void )
{
	m_segmentIndex = 1;
	m_isLadderStarted = false;

	m_stuckMonitor.Reset();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Move improv along path
 */
void CNavPathFollower::Update( float deltaT, bool avoidObstacles )
{
	if (m_path == NULL || m_path->IsValid() == false)
		return;

	const CNavPath::PathSegment *node = (*m_path)[ m_segmentIndex ];

	if (node == NULL)
	{
		m_improv->OnMoveToFailure( m_path->GetEndpoint(), IImprovEvent::FAIL_INVALID_PATH );
		m_path->Invalidate();
		return;
	}

	// handle ladders
	if (node->ladder)
	{
		const Vector *approachPos = NULL;
		const Vector *departPos = NULL;

		if (m_segmentIndex)
			approachPos = &(*m_path)[ m_segmentIndex-1 ]->pos;

		if (m_segmentIndex < m_path->GetSegmentCount()-1)
			departPos = &(*m_path)[ m_segmentIndex+1 ]->pos;

		if (!m_isLadderStarted)
		{
			// set up ladder movement
			m_improv->StartLadder( node->ladder, node->how, approachPos, departPos );
			m_isLadderStarted = true;
		}

		// move improv along ladder
		if (m_improv->TraverseLadder( node->ladder, node->how, approachPos, departPos, deltaT ))
		{
			// completed ladder
			++m_segmentIndex;
		}
		return;
	}

	// reset ladder init flag
	m_isLadderStarted = false;

	//
	// Check if we reached the end of the path
	//
	const float closeRange = 20.0f;
	if ((m_improv->GetFeet() - node->pos).IsLengthLessThan( closeRange ))
	{
		++m_segmentIndex;

		if (m_segmentIndex >= m_path->GetSegmentCount())
		{
			m_improv->OnMoveToSuccess( m_path->GetEndpoint() );
			m_path->Invalidate();
			return;
		}
	}


	m_goal = node->pos;

	const float aheadRange = 300.0f;
	m_segmentIndex = FindPathPoint( aheadRange, &m_goal, &m_behindIndex );
	if (m_segmentIndex >= m_path->GetSegmentCount())
		m_segmentIndex = m_path->GetSegmentCount()-1;


	bool isApproachingJumpArea = false;

	//
	// Crouching
	//
	if (!m_improv->IsUsingLadder())
	{
		// because hostage crouching is not really supported by the engine,
		// if we are standing in a crouch area, we must crouch to avoid collisions
		if (m_improv->GetLastKnownArea() && 
			m_improv->GetLastKnownArea()->GetAttributes() & NAV_CROUCH && 
			!(m_improv->GetLastKnownArea()->GetAttributes() & NAV_JUMP))
		{
			m_improv->Crouch();
		}

		// if we are approaching a crouch area, crouch
		// if there are no crouch areas coming up, stand
		const float crouchRange = 50.0f;
		bool didCrouch = false;
		for( int i=m_segmentIndex; i<m_path->GetSegmentCount(); ++i )
		{
			const CNavArea *to = (*m_path)[i]->area;

			// if there is a jump area on the way to the crouch area, don't crouch as it messes up the jump
			if (to->GetAttributes() & NAV_JUMP)
			{
				isApproachingJumpArea = true;
				break;
			}

			Vector close;
			to->GetClosestPointOnArea( &m_improv->GetCentroid(), &close );

			if ((close - m_improv->GetFeet()).Make2D().IsLengthGreaterThan( crouchRange ))
				break;

			if (to->GetAttributes() & NAV_CROUCH)
			{
				m_improv->Crouch();
				didCrouch = true;
				break;
			}

		}

		if (!didCrouch && !m_improv->IsJumping())
		{
			// no crouch areas coming up
			m_improv->StandUp();
		}

	}	// end crouching logic


	if (m_isDebug)
	{
		m_path->Draw();
		UTIL_DrawBeamPoints( m_improv->GetCentroid(), m_goal + Vector( 0, 0, StepHeight ), 1, 255, 0, 255 );
		UTIL_DrawBeamPoints( m_goal + Vector( 0, 0, StepHeight ), m_improv->GetCentroid(), 1, 255, 0, 255 );
	}

	// check if improv becomes stuck
	m_stuckMonitor.Update( m_improv );


	// if improv has been stuck for too long, give up
	const float giveUpTime = 2.0f;
	if (m_stuckMonitor.GetDuration() > giveUpTime)
	{
		m_improv->OnMoveToFailure( m_path->GetEndpoint(), IImprovEvent::FAIL_STUCK );
		m_path->Invalidate();
		return;
	}


	// if our goal is high above us, we must have fallen
	if (m_goal.z - m_improv->GetFeet().z > JumpCrouchHeight)
	{
		const float closeRange = 75.0f;
		Vector2D to( m_improv->GetFeet().x - m_goal.x, m_improv->GetFeet().y - m_goal.y );
		if (to.IsLengthLessThan( closeRange ))
		{
			// we can't reach the goal position
			// check if we can reach the next node, in case this was a "jump down" situation
			const CNavPath::PathSegment *nextNode = (*m_path)[ m_behindIndex+1 ];
			if (m_behindIndex >=0 && nextNode)
			{
				if (nextNode->pos.z - m_improv->GetFeet().z > JumpCrouchHeight)
				{
					// the next node is too high, too - we really did fall of the path
					m_improv->OnMoveToFailure( m_path->GetEndpoint(), IImprovEvent::FAIL_FELL_OFF );
					m_path->Invalidate();
					return;
				}
			}
			else
			{
				// fell trying to get to the last node in the path
				m_improv->OnMoveToFailure( m_path->GetEndpoint(), IImprovEvent::FAIL_FELL_OFF );
				m_path->Invalidate();
				return;
			}
		}
	}


	// avoid small obstacles
	if (avoidObstacles && !isApproachingJumpArea && !m_improv->IsJumping() && m_segmentIndex < m_path->GetSegmentCount()-1)
	{
		FeelerReflexAdjustment( &m_goal );

		// currently, this is only used for hostages, and their collision physics stinks
		// do more feeler checks to avoid short obstacles
		/*
		const float inc = 0.25f;
		for( float t = 0.5f; t < 1.0f; t += inc )
		{
			FeelerReflexAdjustment( &m_goal, t * StepHeight );
		}
		*/

	}

	// move improv along path
	m_improv->TrackPath( m_goal, deltaT );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the closest point to our current position on our current path
 * If "local" is true, only check the portion of the path surrounding m_pathIndex.
 */
int CNavPathFollower::FindOurPositionOnPath( Vector *close, bool local ) const
{
	if (!m_path->IsValid())
		return -1;

	Vector along, toFeet;
	Vector feet = m_improv->GetFeet();
	Vector eyes = m_improv->GetEyes();
	Vector pos;
	const Vector *from, *to;
	float length;
	float closeLength;
	float closeDistSq = 9999999999.9;
	int closeIndex = -1;
	float distSq;

	int start, end;

	if (local)
	{
		start = m_segmentIndex - 3;
		if (start < 1)
			start = 1;

		end = m_segmentIndex + 3;
		if (end > m_path->GetSegmentCount())
			end = m_path->GetSegmentCount();
	}
	else
	{
		start = 1;
		end = m_path->GetSegmentCount();
	}

	for( int i=start; i<end; ++i )
	{
		from = &(*m_path)[i-1]->pos;
		to = &(*m_path)[i]->pos;

		// compute ray along this path segment
		along = *to - *from;

		// make it a unit vector along the path
		length = along.NormalizeInPlace();

		// compute vector from start of segment to our point
		toFeet = feet - *from;

		// find distance of closest point on ray
		closeLength = DotProduct( toFeet, along );

		// constrain point to be on path segment
		if (closeLength <= 0.0f)
			pos = *from;
		else if (closeLength >= length)
			pos = *to;
		else
			pos = *from + closeLength * along;

		distSq = (pos - feet).LengthSquared();

		// keep the closest point so far
		if (distSq < closeDistSq)
		{
			// don't use points we cant see
			Vector probe = pos + Vector( 0, 0, HalfHumanHeight );
			if (!IsWalkableTraceLineClear( eyes, probe, WALK_THRU_DOORS | WALK_THRU_BREAKABLES ))
				continue;

			// don't use points we cant reach
			//if (!IsStraightLinePathWalkable( &pos ))
			//	continue;

			closeDistSq = distSq;
			if (close)
				*close = pos;
			closeIndex = i-1;
		}
	}

	return closeIndex;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Compute a point a fixed distance ahead along our path.
 * Returns path index just after point.
 */
int CNavPathFollower::FindPathPoint( float aheadRange, Vector *point, int *prevIndex )
{
	// find path index just past aheadRange
	int afterIndex;

	// finds the closest point on local area of path, and returns the path index just prior to it
	Vector close;
	int startIndex = FindOurPositionOnPath( &close, true );

	if (prevIndex)
		*prevIndex = startIndex;

	if (startIndex <= 0)
	{
		// went off the end of the path
		// or next point in path is unwalkable (ie: jump-down)
		// keep same point
		return m_segmentIndex;
	}

	// if we are crouching, just follow the path exactly
	if (m_improv->IsCrouching())
	{
		// we want to move to the immediately next point along the path from where we are now
		int index = startIndex+1;
		if (index >= m_path->GetSegmentCount())
			index = m_path->GetSegmentCount()-1;

		*point = (*m_path)[ index ]->pos;

		// if we are very close to the next point in the path, skip ahead to the next one to avoid wiggling
		// we must do a 2D check here, in case the goal point is floating in space due to jump down, etc
		const float closeEpsilon = 20.0f;	// 10
		while ((*point - close).Make2D().IsLengthLessThan( closeEpsilon ))
		{
			++index;

			if (index >= m_path->GetSegmentCount())
			{
				index = m_path->GetSegmentCount()-1;
				break;
			}

			*point = (*m_path)[ index ]->pos;
		}

		return index;
	}

	// make sure we use a node a minimum distance ahead of us, to avoid wiggling 
	while (startIndex < m_path->GetSegmentCount()-1)
	{
		Vector pos = (*m_path)[ startIndex+1 ]->pos;

		// we must do a 2D check here, in case the goal point is floating in space due to jump down, etc
		const float closeEpsilon = 20.0f;
		if ((pos - close).Make2D().IsLengthLessThan( closeEpsilon ))
		{
			++startIndex;
		}
		else
		{
			break;
		}
	}

	// if we hit a ladder or jump area, must stop (dont use ladder behind us)
	if (startIndex > m_segmentIndex && startIndex < m_path->GetSegmentCount() && 
			((*m_path)[ startIndex ]->ladder || (*m_path)[ startIndex ]->area->GetAttributes() & NAV_JUMP))
	{
		*point = (*m_path)[ startIndex ]->pos;
		return startIndex;
	}

	// we need the point just *ahead* of us
	++startIndex;
	if (startIndex >= m_path->GetSegmentCount())
		startIndex = m_path->GetSegmentCount()-1;

	// if we hit a ladder or jump area, must stop
	if (startIndex < m_path->GetSegmentCount() && 
		((*m_path)[ startIndex ]->ladder || (*m_path)[ startIndex ]->area->GetAttributes() & NAV_JUMP))
	{
		*point = (*m_path)[ startIndex ]->pos;
		return startIndex;
	}

	// note direction of path segment we are standing on
	Vector initDir = (*m_path)[ startIndex ]->pos - (*m_path)[ startIndex-1 ]->pos;
	initDir.NormalizeInPlace();

	Vector feet = m_improv->GetFeet();
	Vector eyes = m_improv->GetEyes();
	float rangeSoFar = 0;

	// this flag is true if our ahead point is visible
	bool visible = true;

	Vector prevDir = initDir;

	// step along the path until we pass aheadRange
	bool isCorner = false;
	int i;
	for( i=startIndex; i<m_path->GetSegmentCount(); ++i )
	{
		Vector pos = (*m_path)[i]->pos;
		Vector to = pos - (*m_path)[i-1]->pos;
		Vector dir = to.Normalize();

		// don't allow path to double-back from our starting direction (going upstairs, down curved passages, etc)
		if (DotProduct( dir, initDir ) < 0.0f) // -0.25f
		{
			--i;
			break;
		}

		// if the path turns a corner, we want to move towards the corner, not into the wall/stairs/etc
		if (DotProduct( dir, prevDir ) < 0.5f)
		{
			isCorner = true;
			--i;
			break;
		}
		prevDir = dir;

		// don't use points we cant see
		Vector probe = pos + Vector( 0, 0, HalfHumanHeight );
		if (!IsWalkableTraceLineClear( eyes, probe, WALK_THRU_BREAKABLES ))
		{
			// presumably, the previous point is visible, so we will interpolate
			visible = false;
			break;
		}

		// if we encounter a ladder or jump area, we must stop
		if (i < m_path->GetSegmentCount() && 
			((*m_path)[ i ]->ladder || (*m_path)[ i ]->area->GetAttributes() & NAV_JUMP))
			break;

		// Check straight-line path from our current position to this position
		// Test for un-jumpable height change, or unrecoverable fall
		//if (!IsStraightLinePathWalkable( &pos ))
		//{
		//	--i;
		//	break;
		//}

		Vector along = (i == startIndex) ? (pos - feet) : (pos - (*m_path)[i-1]->pos);
		rangeSoFar += along.Length2D();

		// stop if we have gone farther than aheadRange
		if (rangeSoFar >= aheadRange)
			break;
	}

	if (i < startIndex)
		afterIndex = startIndex;
	else if (i < m_path->GetSegmentCount())
		afterIndex = i;
	else
		afterIndex = m_path->GetSegmentCount()-1;


	// compute point on the path at aheadRange
	if (afterIndex == 0)
	{
		*point = (*m_path)[0]->pos;
	}
	else
	{
		// interpolate point along path segment
		const Vector *afterPoint = &(*m_path)[ afterIndex ]->pos;
		const Vector *beforePoint = &(*m_path)[ afterIndex-1 ]->pos;

		Vector to = *afterPoint - *beforePoint;
		float length = to.Length2D();

		float t = 1.0f - ((rangeSoFar - aheadRange) / length);

		if (t < 0.0f)
			t = 0.0f;
		else if (t > 1.0f)
			t = 1.0f;

		*point = *beforePoint + t * to;

		// if afterPoint wasn't visible, slide point backwards towards beforePoint until it is
		if (!visible)
		{
			const float sightStepSize = 25.0f;
			float dt = sightStepSize / length;

			Vector probe = *point + Vector( 0, 0, HalfHumanHeight );
			while( t > 0.0f && !IsWalkableTraceLineClear( eyes,  probe, WALK_THRU_BREAKABLES ) )
			{
				t -= dt;
				*point = *beforePoint + t * to;
			}

			if (t <= 0.0f)
				*point = *beforePoint;
		}
	}

	// if position found is too close to us, or behind us, force it farther down the path so we don't stop and wiggle
	if (!isCorner)
	{
		const float epsilon = 50.0f;
		Vector2D toPoint;
		Vector2D centroid( m_improv->GetCentroid().x, m_improv->GetCentroid().y );
		
		toPoint.x = point->x - centroid.x;
		toPoint.y = point->y - centroid.y;

		if (DotProduct( toPoint, initDir.Make2D() ) < 0.0f || toPoint.IsLengthLessThan( epsilon ))
		{
			int i;
			for( i=startIndex; i<m_path->GetSegmentCount(); ++i )
			{
				toPoint.x = (*m_path)[i]->pos.x - centroid.x;
				toPoint.y = (*m_path)[i]->pos.y - centroid.y;
				if ((*m_path)[i]->ladder || (*m_path)[i]->area->GetAttributes() & NAV_JUMP || toPoint.IsLengthGreaterThan( epsilon ))
				{
					*point = (*m_path)[i]->pos;
					startIndex = i;
					break;
				}
			}

			if (i == m_path->GetSegmentCount())
			{
				*point = m_path->GetEndpoint();
				startIndex = m_path->GetSegmentCount()-1;
			}
		}
	}

	// m_pathIndex should always be the next point on the path, even if we're not moving directly towards it
	if (startIndex < m_path->GetSegmentCount())
		return startIndex;

	return m_path->GetSegmentCount()-1;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Do reflex avoidance movements if our "feelers" are touched
 * @todo Parameterize feeler spacing
 */
void CNavPathFollower::FeelerReflexAdjustment( Vector *goalPosition, float height )
{
	// if we are in a "precise" area, do not do feeler adjustments
	if (m_improv->GetLastKnownArea() && m_improv->GetLastKnownArea()->GetAttributes() & NAV_PRECISE)
		return;

	Vector dir( BotCOS( m_improv->GetMoveAngle() ), BotSIN( m_improv->GetMoveAngle() ), 0.0f );
	dir.z = 0.0f;
	dir.NormalizeInPlace();

	Vector lat( -dir.y, dir.x, 0.0f );

	const float feelerOffset = (m_improv->IsCrouching()) ? 20.0f : 25.0f;	// 15, 20
	const float feelerLengthRun = 50.0f;	// 100 - too long for tight hallways (cs_747)
	const float feelerLengthWalk = 30.0f;

	const float feelerHeight = (height > 0.0f) ? height : StepHeight + 0.1f;	// if obstacle is lower than StepHeight, we'll walk right over it

	float feelerLength = (m_improv->IsRunning()) ? feelerLengthRun : feelerLengthWalk;

	feelerLength = (m_improv->IsCrouching()) ? 20.0f : feelerLength;

	//
	// Feelers must follow floor slope
	//
	float ground;
	Vector normal;
	if (m_improv->GetSimpleGroundHeightWithFloor( &m_improv->GetEyes(), &ground, &normal ) == false)
		return;

	// get forward vector along floor
	dir = CrossProduct( lat, normal );

	// correct the sideways vector
	lat = CrossProduct( dir, normal );


	Vector feet = m_improv->GetFeet();
	feet.z += feelerHeight;

	Vector from = feet + feelerOffset * lat;
	Vector to = from + feelerLength * dir;

	bool leftClear = IsWalkableTraceLineClear( from, to, WALK_THRU_DOORS | WALK_THRU_BREAKABLES );

	// draw debug beams
	if (m_isDebug)
	{
		if (leftClear)
			UTIL_DrawBeamPoints( from, to, 1, 0, 255, 0 );
		else
			UTIL_DrawBeamPoints( from, to, 1, 255, 0, 0 );
	}

	from = feet - feelerOffset * lat;
	to = from + feelerLength * dir;

	bool rightClear = IsWalkableTraceLineClear( from, to, WALK_THRU_DOORS | WALK_THRU_BREAKABLES );

	// draw debug beams
	if (m_isDebug)
	{
		if (rightClear)
			UTIL_DrawBeamPoints( from, to, 1, 0, 255, 0 );
		else
			UTIL_DrawBeamPoints( from, to, 1, 255, 0, 0 );
	}



	const float avoidRange = (m_improv->IsCrouching()) ? 150.0f : 300.0f;

	if (!rightClear)
	{
		if (leftClear)
		{
			// right hit, left clear - veer left
			*goalPosition = *goalPosition + avoidRange * lat;
			//*goalPosition = m_improv->GetFeet() + avoidRange * lat;

			//m_improv->StrafeLeft();
		}
	}
	else if (!leftClear)
	{
		// right clear, left hit - veer right
		*goalPosition = *goalPosition - avoidRange * lat;
		//*goalPosition = m_improv->GetFeet() - avoidRange * lat;

		//m_improv->StrafeRight();
	}

}

//--------------------------------------------------------------------------------------------------------------
/**
 * Reset the stuck-checker.
 */
CStuckMonitor::CStuckMonitor( void )
{
	m_isStuck = false;
	m_avgVelIndex = 0;
	m_avgVelCount = 0;
}

/**
 * Reset the stuck-checker.
 */
void CStuckMonitor::Reset( void )
{
	m_isStuck = false;
	m_avgVelIndex = 0;
	m_avgVelCount = 0;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Test if the improv has become stuck
 */
void CStuckMonitor::Update( CImprov *improv )
{
	if (m_isStuck)
	{
		// improv is stuck - see if it has moved far enough to be considered unstuck
		const float unstuckRange = 75.0f;
		if ((improv->GetCentroid() - m_stuckSpot).IsLengthGreaterThan( unstuckRange ))
		{
			// no longer stuck
			Reset();
			//PrintIfWatched( "UN-STUCK\n" );
		}
	}
	else
	{
		// check if improv has become stuck

		// compute average velocity over a short period (for stuck check)
		Vector vel = improv->GetCentroid() - m_lastCentroid;

		// if we are jumping, ignore Z
		//if (improv->IsJumping())
		//	vel.z = 0.0f;

		// ignore Z unless we are on a ladder (which is only Z)
		if (!improv->IsUsingLadder())
			vel.z = 0.0f;

		// cannot be Length2D, or will break ladder movement (they are only Z)
		float moveDist = vel.Length();

		float deltaT = gpGlobals->time - m_lastTime;
		if (deltaT <= 0.0f)
			return;

		m_lastTime = gpGlobals->time;

		// compute current velocity
		m_avgVel[ m_avgVelIndex++ ] = moveDist/deltaT;

		if (m_avgVelIndex == MAX_VEL_SAMPLES)
			m_avgVelIndex = 0;

		if (m_avgVelCount < MAX_VEL_SAMPLES)
		{
			++m_avgVelCount;
		}
		else
		{
			// we have enough samples to know if we're stuck

			float avgVel = 0.0f;
			for( int t=0; t<m_avgVelCount; ++t )
				avgVel += m_avgVel[t];

			avgVel /= m_avgVelCount;

			// cannot make this velocity too high, or actors will get "stuck" when going down ladders
			float stuckVel = (improv->IsUsingLadder()) ? 10.0f : 20.0f;

			if (avgVel < stuckVel)
			{
				// note when and where we initially become stuck
				m_stuckTimer.Start();
				m_stuckSpot = improv->GetCentroid();
				m_isStuck = true;
			}
		}
	}

	// always need to track this
	m_lastCentroid = improv->GetCentroid();
}

