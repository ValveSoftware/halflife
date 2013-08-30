/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//=========================================================
// nodes.h
//=========================================================

#ifndef	NODES_H
#define	NODES_H

#define	bits_NODE_GROUP_REALM	1

class CLink
{
public:
	entvars_t	*m_pLinkEnt;// the entity that blocks this connection (doors, etc)
};


class CGraph
{
public:
	BOOL	m_fGraphPresent;// is the graph in memory?
	BOOL	m_fGraphPointersSet;// are the entity pointers for the graph all set?

	int		m_cLinks;// total number of links
	CLink	*m_pLinkPool;// big list of all node connections

	void	InitGraph( void );
	int		AllocNodes ( void );
	
	int		CheckNODFile(char *szMapName);
	int		FLoadGraph(char *szMapName);
	int		FSetGraphPointers(void);
	void	ShowNodeConnections ( int iNode );
	int		FindNearestNode ( const Vector &vecOrigin, CBaseEntity *pEntity );
	int		FindNearestNode ( const Vector &vecOrigin, int afNodeTypes );

};

extern CGraph WorldGraph;

#endif	// NODES_H
