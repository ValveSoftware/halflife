// nav_node.cpp
// AI Navigation Nodes
// Author: Michael S. Booth (mike@turtlerockstudios.com), January 2003

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "bot_util.h"
#include "nav_node.h"

NavDirType Opposite[ NUM_DIRECTIONS ] = { SOUTH, WEST, NORTH, EAST };

CNavNode *CNavNode::m_list = NULL;
unsigned int CNavNode::m_listLength = 0;

Extent NodeMapExtent;


//--------------------------------------------------------------------------------------------------------------
/**
 * Constructor
 */
CNavNode::CNavNode( const Vector *pos, const Vector *normal, CNavNode *parent )
{
	m_pos = *pos;
	m_normal = *normal;

	static unsigned int nextID = 1;
	m_id = nextID++;

	for( int i=0; i<NUM_DIRECTIONS; i++ )
		m_to[ i ] = NULL;

	m_visited = 0;
	m_parent = parent;

	m_next = m_list;
	m_list = this;
	m_listLength++;

	m_isCovered = false;
	m_area = NULL;

	m_attributeFlags = 0;

	//CONSOLE_ECHO( "  Created node #%d ( %g, %g, %g )\n", m_id, pos->x, pos->y, pos->z );
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Create a connection FROM this node TO the given node, in the given direction
 */
void CNavNode::ConnectTo( CNavNode *node, NavDirType dir )
{
	m_to[ dir ] = node;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return node at given position.
 * @todo Need a hash table to make this lookup fast
 */
const CNavNode *CNavNode::GetNode( const Vector *pos )
{
	const float tolerance = 0.45f * GenerationStepSize;			// 1.0f

	for( const CNavNode *node = m_list; node; node = node->m_next )
	{
		float dx = ABS( node->m_pos.x - pos->x );
		float dy = ABS( node->m_pos.y - pos->y );
		float dz = ABS( node->m_pos.z - pos->z );

		if (dx < tolerance && dy < tolerance && dz < tolerance)
			return node;
	}

	return NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if this node is bidirectionally linked to 
 * another node in the given direction
 */
BOOL CNavNode::IsBiLinked( NavDirType dir ) const
{
	if (m_to[ dir ] && 
			m_to[ dir ]->m_to[ Opposite[dir] ] == this)
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if this node is the NW corner of a quad of nodes
 * that are all bidirectionally linked.
 */
BOOL CNavNode::IsClosedCell( void ) const
{
	if (IsBiLinked( SOUTH ) &&
			IsBiLinked( EAST ) &&
			m_to[ EAST ]->IsBiLinked( SOUTH ) &&
			m_to[ SOUTH ]->IsBiLinked( EAST ) &&
			m_to[ EAST ]->m_to[ SOUTH ] == m_to[ SOUTH ]->m_to[ EAST ])
		return true;

	return false;
}

