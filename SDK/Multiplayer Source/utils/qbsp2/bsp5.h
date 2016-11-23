/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

// bsp5.h

#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "threads.h"

//#define	ON_EPSILON	0.05
#define	BOGUS_RANGE	18000

// the exact bounding box of the brushes is expanded some for the headnode
// volume.  is this still needed?
#define	SIDESPACE	24

//============================================================================


typedef struct
{
	int		numpoints;
	vec3_t	points[8];			// variable sized
} winding_t;

#define MAX_POINTS_ON_WINDING	128

winding_t *BaseWindingForPlane (dplane_t *p);
winding_t	*NewWinding (int points);
void		FreeWinding (winding_t *w);
winding_t	*CopyWinding (winding_t *w);
winding_t	*ClipWinding (winding_t *in, dplane_t *split, qboolean keepon);
void	DivideWinding (winding_t *in, dplane_t *split, winding_t **front, winding_t **back);

//============================================================================
 
#define	MAXEDGES			48 // 32
#define	MAXPOINTS			28		// don't let a base face get past this
									// because it can be split more later

typedef struct face_s
{
	struct face_s	*next;
	
	int				planenum;
	int				texturenum;
	int				contents;	// contents in front of face

	struct face_s	*original;		// face on node
	int				outputnumber;		// only valid for original faces after
										// write surfaces
	int				numpoints;
	vec3_t			pts[MAXEDGES];		// FIXME: change to use winding_t
} face_t;


typedef struct surface_s
{
	struct surface_s	*next;
	int			planenum;
	vec3_t		mins, maxs;
	struct node_s	*onnode;		// true if surface has already been used
									// as a splitting node
	face_t		*faces;	// links to all the faces on either side of the surf
} surface_t;

typedef struct
{
	vec3_t		mins, maxs;
	surface_t	*surfaces;
} surfchain_t;

//
// there is a node_t structure for every node and leaf in the bsp tree
//
#define	PLANENUM_LEAF		-1

typedef struct node_s
{
	surface_t		*surfaces;

	vec3_t			mins,maxs;		// bounding volume of portals;

// information for decision nodes	
	int				planenum;		// -1 = leaf node	
	struct node_s	*children[2];	// only valid for decision nodes
	face_t			*faces;			// decision nodes only, list for both sides
	
// information for leafs
	int				contents;		// leaf nodes (0 for decision nodes)
	face_t			**markfaces;	// leaf nodes only, point to node faces
	struct portal_s	*portals;
	int				visleafnum;		// -1 = solid
	int				valid;			// for flood filling
	int				occupied;		// light number in leaf for outside filling
} node_t;


#define	NUM_HULLS		4	

face_t *NewFaceFromFace (face_t *in);
void SplitFace (face_t *in, dplane_t *split, face_t **front, face_t **back);

//=============================================================================

// solidbsp.c

void DivideFacet (face_t *in, dplane_t *split, face_t **front, face_t **back);
void CalcSurfaceInfo (surface_t *surf);
void SubdivideFace (face_t *f, face_t **prevptr);
node_t *SolidBSP (surfchain_t *surfhead);

//=============================================================================

// merge.c

void MergePlaneFaces (surface_t *plane);
face_t *MergeFaceToList (face_t *face, face_t *list);
face_t *FreeMergeListScraps (face_t *merged);
void MergeAll (surface_t *surfhead);

//=============================================================================

// surfaces.c

extern	int		c_cornerverts;
extern	int		c_tryedges;
extern	face_t		*edgefaces[MAX_MAP_EDGES][2];

extern	int		firstmodeledge;
extern	int		firstmodelface;

void SubdivideFaces (surface_t *surfhead);

surfchain_t *GatherNodeFaces (node_t *headnode);

void MakeFaceEdges (node_t *headnode);

//=============================================================================

// portals.c

typedef struct portal_s
{
	dplane_t	plane;
	node_t		*onnode;		// NULL = outside box
	node_t		*nodes[2];		// [0] = front side of plane
	struct portal_s	*next[2];	
	winding_t	*winding;
} portal_t;

extern	node_t	outside_node;		// portals outside the world face this

void AddPortalToNodes (portal_t *p, node_t *front, node_t *back);
void RemovePortalFromNode (portal_t *portal, node_t *l);
void MakeHeadnodePortals (node_t *node, vec3_t mins, vec3_t maxs);

void WritePortalfile (node_t *headnode);

//=============================================================================

// region.c

void GrowNodeRegions (node_t *headnode);

//=============================================================================

// tjunc.c

void tjunc (node_t *headnode);

//=============================================================================

// writebsp.c

void WriteNodePlanes (node_t *headnode);
void WriteClipNodes (node_t *headnode);
void WriteDrawNodes (node_t *headnode);

void BeginBSPFile (void);
void FinishBSPFile (void);

//=============================================================================

// draw.c

extern	vec3_t	draw_mins, draw_maxs;

void Draw_ClearBounds (void);
void Draw_AddToBounds (vec3_t v);
void Draw_DrawFace (face_t *f);
void Draw_ClearWindow (void);
void Draw_SetRed (void);
void Draw_SetGrey (void);
void Draw_SetBlack (void);
void DrawPoint (vec3_t v);

void Draw_SetColor (int c);
void SetColor (int c);
void DrawPortal (portal_t *p);
void DrawLeaf (node_t *l, int color);

void DrawWinding (winding_t *w);
void DrawTri (vec3_t p1, vec3_t p2, vec3_t p3);

//=============================================================================

// outside.c

node_t *FillOutside (node_t *node, qboolean leakfile);

//=============================================================================

extern	qboolean	drawflag;
extern	qboolean	nofill;
extern	qboolean	notjunc;
extern	qboolean	verbose;
extern	qboolean	nogfx;
extern	qboolean	leakonly;
extern	qboolean	watervis;

extern	int		subdivide_size;

extern	int		hullnum;

void qprintf (char *fmt, ...);	// only prints if verbose

extern	int		valid;

extern	char	portfilename[1024];
extern	char	bspfilename[1024];
extern	char	pointfilename[1024];

extern	qboolean	worldmodel;

extern	face_t	*validfaces[MAX_MAP_PLANES];

surfchain_t *SurflistFromValidFaces (void);


// misc functions

face_t *AllocFace (void);
void FreeFace (face_t *f);

struct portal_s *AllocPortal (void);
void FreePortal (struct portal_s *p);

surface_t *AllocSurface (void);
void FreeSurface (surface_t *s);

node_t *AllocNode (void);

//=============================================================================

// cull.c

void CullStuff (void);

