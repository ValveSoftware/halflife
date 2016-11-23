/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// nodes.h
//=========================================================

//=========================================================
// DEFINE
//=========================================================
#define MAX_STACK_NODES	    100
#define	NO_NODE				-1
#define MAX_NODE_HULLS		4

#define bits_NODE_LAND      ( 1 << 0 )  // Land node, so nudge if necessary.
#define bits_NODE_AIR       ( 1 << 1 )  // Air node, don't nudge.
#define bits_NODE_WATER     ( 1 << 2 )  // Water node, don't nudge.
#define bits_NODE_GROUP_REALM (bits_NODE_LAND | bits_NODE_AIR | bits_NODE_WATER)

//=========================================================
// Instance of a node.
//=========================================================
class CNode
{
public:
	Vector	m_vecOrigin;// location of this node in space
	Vector  m_vecOriginPeek; // location of this node (LAND nodes are NODE_HEIGHT higher).
	BYTE    m_Region[3]; // Which of 256 regions do each of the coordinate belong?
	int		m_afNodeInfo;// bits that tell us more about this location
	
	int		m_cNumLinks; // how many links this node has
	int		m_iFirstLink;// index of this node's first link in the link pool.

	// Where to start looking in the compressed routing table (offset into m_pRouteInfo).
	// (4 hull sizes -- smallest to largest + fly/swim), and secondly, door capability.
	//
	int		m_pNextBestNode[MAX_NODE_HULLS][2];

	// Used in finding the shortest path. m_fClosestSoFar is -1 if not visited.
	// Then it is the distance to the source. If another path uses this node
	// and has a closer distance, then m_iPreviousNode is also updated.
	//
	float   m_flClosestSoFar; // Used in finding the shortest path.
	int		m_iPreviousNode;

	short	m_sHintType;// there is something interesting in the world at this node's position
	short	m_sHintActivity;// there is something interesting in the world at this node's position
	float	m_flHintYaw;// monster on this node should face this yaw to face the hint.
};

//=========================================================
// CLink - A link between 2 nodes
//=========================================================
#define		bits_LINK_SMALL_HULL	( 1 << 0 )// headcrab box can fit through this connection
#define		bits_LINK_HUMAN_HULL	( 1 << 1 )// player box can fit through this connection
#define		bits_LINK_LARGE_HULL	( 1 << 2 )// big box can fit through this connection
#define		bits_LINK_FLY_HULL		( 1 << 3 )// a flying big box can fit through this connection
#define		bits_LINK_DISABLED		( 1 << 4 )// link is not valid when the set

#define		NODE_SMALL_HULL			0
#define		NODE_HUMAN_HULL			1
#define		NODE_LARGE_HULL			2
#define		NODE_FLY_HULL			3

class CLink
{
public:
	int		m_iSrcNode;// the node that 'owns' this link ( keeps us from having to make reverse lookups )
	int		m_iDestNode;// the node on the other end of the link. 
	
	entvars_t	*m_pLinkEnt;// the entity that blocks this connection (doors, etc)

	// m_szLinkEntModelname is not necessarily NULL terminated (so we can store it in a more alignment-friendly 4 bytes)
	char	m_szLinkEntModelname[ 4 ];// the unique name of the brush model that blocks the connection (this is kept for save/restore)

	int		m_afLinkInfo;// information about this link
	float	m_flWeight;// length of the link line segment
};


typedef struct
{
	int m_SortedBy[3];
	int m_CheckedEvent;
} DIST_INFO;

typedef struct
{
	Vector v;
	short n;		// Nearest node or -1 if no node found.
} CACHE_ENTRY;

//=========================================================
// CGraph 
//=========================================================
#define	GRAPH_VERSION	(int)16// !!!increment this whever graph/node/link classes change, to obsolesce older disk files.
class CGraph
{
public:

// the graph has two flags, and should not be accessed unless both flags are TRUE!
	BOOL	m_fGraphPresent;// is the graph in memory?
	BOOL	m_fGraphPointersSet;// are the entity pointers for the graph all set?
	BOOL    m_fRoutingComplete; // are the optimal routes computed, yet?

	CNode	*m_pNodes;// pointer to the memory block that contains all node info
	CLink	*m_pLinkPool;// big list of all node connections
	char    *m_pRouteInfo; // compressed routing information the nodes use.

	int		m_cNodes;// total number of nodes
	int		m_cLinks;// total number of links
	int     m_nRouteInfo; // size of m_pRouteInfo in bytes.

	// Tables for making nearest node lookup faster. SortedBy provided nodes in a
	// order of a particular coordinate. Instead of doing a binary search, RangeStart
	// and RangeEnd let you get to the part of SortedBy that you are interested in.
	//
	// Once you have a point of interest, the only way you'll find a closer point is
	// if at least one of the coordinates is closer than the ones you have now. So we
	// search each range. After the search is exhausted, we know we have the closest
	// node.
	//
#define CACHE_SIZE 128
#define NUM_RANGES 256
	DIST_INFO *m_di;	// This is m_cNodes long, but the entries don't correspond to CNode entries.
	int m_RangeStart[3][NUM_RANGES];
	int m_RangeEnd[3][NUM_RANGES];
	float m_flShortest;
	int m_iNearest;
	int m_minX, m_minY, m_minZ, m_maxX, m_maxY, m_maxZ;
	int m_minBoxX, m_minBoxY, m_minBoxZ, m_maxBoxX, m_maxBoxY, m_maxBoxZ;
	int m_CheckedCounter;
	float m_RegionMin[3], m_RegionMax[3]; // The range of nodes.
	CACHE_ENTRY m_Cache[CACHE_SIZE];


	int m_HashPrimes[16];
	short *m_pHashLinks;
	int m_nHashLinks;


	// kinda sleazy. In order to allow variety in active idles for monster groups in a room with more than one node, 
	// we keep track of the last node we searched from and store it here. Subsequent searches by other monsters will pick
	// up where the last search stopped.
	int		m_iLastActiveIdleSearch;

	// another such system used to track the search for cover nodes, helps greatly with two monsters trying to get to the same node.
	int		m_iLastCoverSearch;

	// functions to create the graph
	int		LinkVisibleNodes ( CLink *pLinkPool, FILE *file, int *piBadNode );
	int		RejectInlineLinks ( CLink *pLinkPool, FILE *file );
	int		FindShortestPath ( int *piPath, int iStart, int iDest, int iHull, int afCapMask);
	int		FindNearestNode ( const Vector &vecOrigin, CBaseEntity *pEntity );
	int		FindNearestNode ( const Vector &vecOrigin, int afNodeTypes );
	//int		FindNearestLink ( const Vector &vecTestPoint, int *piNearestLink, BOOL *pfAlongLine );
	float	PathLength( int iStart, int iDest, int iHull, int afCapMask );
	int		NextNodeInRoute( int iCurrentNode, int iDest, int iHull, int iCap );

	enum NODEQUERY { NODEGRAPH_DYNAMIC, NODEGRAPH_STATIC };
	// A static query means we're asking about the possiblity of handling this entity at ANY time
	// A dynamic query means we're asking about it RIGHT NOW.  So we should query the current state
	int		HandleLinkEnt ( int iNode, entvars_t *pevLinkEnt, int afCapMask, NODEQUERY queryType );
	entvars_t*	LinkEntForLink ( CLink *pLink, CNode *pNode );
	void	ShowNodeConnections ( int iNode );
	void	InitGraph( void );
	int		AllocNodes ( void );
	
	int		CheckNODFile(char *szMapName);
	int		FLoadGraph(char *szMapName);
	int		FSaveGraph(char *szMapName);
	int		FSetGraphPointers(void);
	void	CheckNode(Vector vecOrigin, int iNode);

	void    BuildRegionTables(void);
	void    ComputeStaticRoutingTables(void);
	void    TestRoutingTables(void);

	void	HashInsert(int iSrcNode, int iDestNode, int iKey);
	void    HashSearch(int iSrcNode, int iDestNode, int &iKey);
	void	HashChoosePrimes(int TableSize);
	void    BuildLinkLookups(void);

	void    SortNodes(void);

	int			HullIndex( const CBaseEntity *pEntity );	// what hull the monster uses
	int			NodeType( const CBaseEntity *pEntity );		// what node type the monster uses
	inline int	CapIndex( int afCapMask ) 
	{ 
		if (afCapMask & (bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE)) 
			return 1; 
		return 0; 
	}


	inline	CNode &Node( int i )
	{
#ifdef _DEBUG
		if ( !m_pNodes || i < 0 || i > m_cNodes )
			ALERT( at_error, "Bad Node!\n" );
#endif
		return m_pNodes[i];
	}

	inline	CLink &Link( int i )
	{
#ifdef _DEBUG
		if ( !m_pLinkPool || i < 0 || i > m_cLinks )
			ALERT( at_error, "Bad link!\n" );
#endif
		return m_pLinkPool[i];
	}
	
	inline CLink &NodeLink( int iNode, int iLink )
	{
		return Link( Node( iNode ).m_iFirstLink + iLink );
	}

	inline CLink &NodeLink( const CNode &node, int iLink )
	{
		return Link( node.m_iFirstLink + iLink );
	}

	inline  int	INodeLink ( int iNode, int iLink )
	{
		return NodeLink( iNode, iLink ).m_iDestNode;
	}

#if 0
	inline CNode &SourceNode( int iNode, int iLink )
	{
		return Node( NodeLink( iNode, iLink ).m_iSrcNode );
	}

	inline CNode &DestNode( int iNode, int iLink )
	{
		return Node( NodeLink( iNode, iLink ).m_iDestNode );
	}

	inline	CNode *PNodeLink ( int iNode, int iLink ) 
	{
		return &DestNode( iNode, iLink );
	}
#endif
};

//=========================================================
// Nodes start out as ents in the level. The node graph 
// is built, then these ents are discarded. 
//=========================================================
class CNodeEnt : public CBaseEntity
{
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	short m_sHintType;
	short m_sHintActivity;
};


//=========================================================
// CStack - last in, first out.
//=========================================================
class CStack 
{
public:
			CStack( void );
	void	Push( int value );
	int		Pop( void );
	int		Top( void );
	int		Empty( void ) { return m_level==0; }
	int		Size( void ) { return m_level; }
	void    CopyToArray ( int *piArray );

private:
	int		m_stack[ MAX_STACK_NODES ];
	int		m_level;
};


//=========================================================
// CQueue - first in, first out.
//=========================================================
class CQueue
{
public:

	CQueue( void );// constructor
	inline int Full ( void ) { return ( m_cSize == MAX_STACK_NODES ); }
	inline int Empty ( void ) { return ( m_cSize == 0 ); }
	//inline int Tail ( void ) { return ( m_queue[ m_tail ] ); }
	inline int Size ( void ) { return ( m_cSize ); }
	void Insert( int, float );
	int Remove( float & );

private:
	int	m_cSize;
    struct tag_QUEUE_NODE
    {
        int   Id;
        float Priority;
    } m_queue[ MAX_STACK_NODES ];
	int m_head;
	int m_tail;
};

//=========================================================
// CQueuePriority - Priority queue (smallest item out first).
//
//=========================================================
class CQueuePriority
{
public:

	CQueuePriority( void );// constructor
	inline int Full ( void ) { return ( m_cSize == MAX_STACK_NODES ); }
	inline int Empty ( void ) { return ( m_cSize == 0 ); }
	//inline int Tail ( float & ) { return ( m_queue[ m_tail ].Id ); }
	inline int Size ( void ) { return ( m_cSize ); }
	void Insert( int, float );
	int Remove( float &);

private:
	int	m_cSize;
    struct tag_HEAP_NODE
    {
        int   Id;
        float Priority;
    } m_heap[ MAX_STACK_NODES ];
	void Heap_SiftDown(int);
	void Heap_SiftUp(void);

};

//=========================================================
// hints - these MUST coincide with the HINTS listed under
// info_node in the FGD file!
//=========================================================
enum
{
	HINT_NONE = 0,
	HINT_WORLD_DOOR,
	HINT_WORLD_WINDOW,
	HINT_WORLD_BUTTON,
	HINT_WORLD_MACHINERY,
	HINT_WORLD_LEDGE,
	HINT_WORLD_LIGHT_SOURCE,
	HINT_WORLD_HEAT_SOURCE,
	HINT_WORLD_BLINKING_LIGHT,
	HINT_WORLD_BRIGHT_COLORS,
	HINT_WORLD_HUMAN_BLOOD,
	HINT_WORLD_ALIEN_BLOOD,

	HINT_TACTICAL_EXIT = 100,
	HINT_TACTICAL_VANTAGE,
	HINT_TACTICAL_AMBUSH,

	HINT_STUKA_PERCH = 300,
	HINT_STUKA_LANDING,
};

extern CGraph WorldGraph;
