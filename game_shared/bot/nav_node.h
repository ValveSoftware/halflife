// nav_node.h
// AI Navigation Nodes
// Author: Michael S. Booth (mike@turtlerockstudios.com), January 2003

#ifndef _NAV_NODE_H_
#define _NAV_NODE_H_

#include "nav.h"

//--------------------------------------------------------------------------------------------------------------
/**
 * Navigation Nodes.
 * These Nodes encapsulate world locations, and ways to get from one location to an adjacent one.
 * Note that these links are not necessarily commutative (falling off of a ledge, for example).
 */
class CNavNode
{
public:
	CNavNode( const Vector *pos, const Vector *normal, CNavNode *parent = NULL );

	static const CNavNode *GetNode( const Vector *pos );			///< return navigation node at the position, or NULL if none exists

	CNavNode *GetConnectedNode( NavDirType dir ) const;				///< get navigation node connected in given direction, or NULL if cant go that way
	const Vector *GetPosition( void ) const;
	const Vector *GetNormal( void ) const							{ return &m_normal; }
	unsigned int GetID( void ) const									{ return m_id; }

	static CNavNode *GetFirst( void )									{ return m_list; }
	static unsigned int GetListLength( void )					{ return m_listLength; }
	CNavNode *GetNext( void )													{ return m_next; }
	
	void ConnectTo( CNavNode *node, NavDirType dir );		///< create a connection FROM this node TO the given node, in the given direction
	CNavNode *GetParent( void ) const;

	void MarkAsVisited( NavDirType dir );					///< mark the given direction as having been visited
	BOOL HasVisited( NavDirType dir );						///< return TRUE if the given direction has already been searched
	BOOL IsBiLinked( NavDirType dir ) const;			///< node is bidirectionally linked to another node in the given direction
	BOOL IsClosedCell( void ) const;							///< node is the NW corner of a bi-linked quad of nodes

	void Cover( void )								{ m_isCovered = true; }	///< @todo Should pass in area that is covering
	BOOL IsCovered( void ) const			{ return m_isCovered; }	///< return true if this node has been covered by an area

	void AssignArea( CNavArea *area );						///< assign the given area to this node
	CNavArea *GetArea( void ) const;							///< return associated area

	void SetAttributes( unsigned char bits )		{ m_attributeFlags = bits; }
	unsigned char GetAttributes( void ) const		{ return m_attributeFlags; }

private:
	friend void DestroyNavigationMap( void );

	Vector m_pos;																	///< position of this node in the world
	Vector m_normal;															///< surface normal at this location
	CNavNode *m_to[ NUM_DIRECTIONS ];							///< links to north, south, east, and west. NULL if no link
	unsigned int m_id;														///< unique ID of this node
	unsigned char m_attributeFlags;								///< set of attribute bit flags (see NavAttributeType)

	static CNavNode *m_list;											///< the master list of all nodes for this map
	static unsigned int m_listLength;
	CNavNode *m_next;															///< next link in master list

	// below are only needed when generating
	unsigned char m_visited;											///< flags for automatic node generation. If direction bit is clear, that direction hasn't been explored yet.
	CNavNode *m_parent;														///< the node prior to this in the search, which we pop back to when this node's search is done (a stack)
	BOOL m_isCovered;															///< true when this node is "covered" by a CNavArea
	CNavArea *m_area;															///< the area this node is contained within
};

//--------------------------------------------------------------------------------------------------------------
//
// Inlines
//

inline CNavNode *CNavNode::GetConnectedNode( NavDirType dir ) const
{
	return m_to[ dir ];
}

inline const Vector *CNavNode::GetPosition( void ) const
{
	return &m_pos;
}

inline CNavNode *CNavNode::GetParent( void ) const
{
	return m_parent;
}

inline void CNavNode::MarkAsVisited( NavDirType dir )
{
	m_visited |= (1 << dir);
}

inline BOOL CNavNode::HasVisited( NavDirType dir )
{
	if (m_visited & (1 << dir))
		return true;

	return false;
}

inline void CNavNode::AssignArea( CNavArea *area )
{
	m_area = area;
}

inline CNavArea *CNavNode::GetArea( void ) const
{
	return m_area;
}


#endif // _NAV_NODE_H_
