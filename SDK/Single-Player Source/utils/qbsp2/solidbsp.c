/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

// solidbsp.c

#include "bsp5.h"

/*

  Each node or leaf will have a set of portals that completely enclose
  the volume of the node and pass into an adjacent node.

*/

int		c_leaffaces;
int		c_nodefaces;
int		c_splitnodes;

//============================================================================

/*
==================
FaceSide

For BSP hueristic
==================
*/
int FaceSide (face_t *in, dplane_t *split)
{
	int		frontcount, backcount;
	vec_t	dot;
	int		i;
	vec_t	*p;
	
	
	frontcount = backcount = 0;
	
// axial planes are fast
	if (split->type < 3)
		for (i=0, p = in->pts[0]+split->type ; i<in->numpoints ; i++, p+=3)
		{
			if (*p > split->dist + ON_EPSILON)
			{
				if (backcount)
					return SIDE_ON;
				frontcount = 1;
			}
			else if (*p < split->dist - ON_EPSILON)
			{
				if (frontcount)
					return SIDE_ON;
				backcount = 1;
			}
		}
	else	
// sloping planes take longer
		for (i=0, p = in->pts[0] ; i<in->numpoints ; i++, p+=3)
		{
			dot = DotProduct (p, split->normal);
			dot -= split->dist;
			if (dot > ON_EPSILON)
			{
				if (backcount)
					return SIDE_ON;
				frontcount = 1;
			}
			else if (dot < -ON_EPSILON)
			{
				if (frontcount)
					return SIDE_ON;
				backcount = 1;
			}
		}
	
	if (!frontcount)
		return SIDE_BACK;
	if (!backcount)
		return SIDE_FRONT;
	
	return SIDE_ON;
}

/*
==================
ChooseMidPlaneFromList

When there are a huge number of planes, just choose one closest
to the middle.
==================
*/
surface_t *ChooseMidPlaneFromList (surface_t *surfaces, vec3_t mins, vec3_t maxs)
{
	int			j,l;
	surface_t	*p, *bestsurface;
	vec_t		bestvalue, value, dist;
	dplane_t		*plane;

//
// pick the plane that splits the least
//
	bestvalue = 6*8192*8192;
	bestsurface = NULL;
	
	for (p=surfaces ; p ; p=p->next)
	{
		if (p->onnode)
			continue;

		plane = &dplanes[p->planenum];
		
	// check for axis aligned surfaces
		l = plane->type;
		if (l > PLANE_Z)
			continue;

	//
	// calculate the split metric along axis l, smaller values are better
	//
		value = 0;

		dist = plane->dist * plane->normal[l];
		for (j=0 ; j<3 ; j++)
		{
			if (j == l)
			{
				value += (maxs[l]-dist)*(maxs[l]-dist);
				value += (dist-mins[l])*(dist-mins[l]);
			}
			else
				value += 2*(maxs[j]-mins[j])*(maxs[j]-mins[j]);
		}
		
		if (value > bestvalue)
			continue;
		
	//
	// currently the best!
	//
		bestvalue = value;
		bestsurface = p;
	}

	if (!bestsurface)
	{
		for (p=surfaces ; p ; p=p->next)
			if (!p->onnode)
				return p;		// first valid surface
		Error ("ChooseMidPlaneFromList: no valid planes");
	}
		
	return bestsurface;
}



/*
==================
ChoosePlaneFromList

Choose the plane that splits the least faces
==================
*/
surface_t *ChoosePlaneFromList (surface_t *surfaces, vec3_t mins, vec3_t maxs)
{
	int			j,k,l;
	surface_t	*p, *p2, *bestsurface;
	vec_t		bestvalue, bestdistribution, value, dist;
	dplane_t		*plane;
	face_t		*f;
	
//
// pick the plane that splits the least
//
	bestvalue = 99999;
	bestsurface = NULL;
	bestdistribution = 9e30;
	
	for (p=surfaces ; p ; p=p->next)
	{
		if (p->onnode)
			continue;

		plane = &dplanes[p->planenum];
		k = 0;

		for (p2=surfaces ; p2 ; p2=p2->next)
		{
			if (p2 == p)
				continue;
			if (p2->onnode)
				continue;
				
			for (f=p2->faces ; f ; f=f->next)
			{
				if (FaceSide (f, plane) == SIDE_ON)
				{
					k++;
					if (k >= bestvalue)
						break;
				}
				
			}
			if (k > bestvalue)
				break;
		}

		if (k > bestvalue)
			continue;
			
	// if equal numbers, axial planes win, then decide on spatial subdivision
	
		if (k < bestvalue || (k == bestvalue && plane->type < PLANE_ANYX) )
		{
		// check for axis aligned surfaces
			l = plane->type;
	
			if (l <= PLANE_Z)
			{	// axial aligned						
			//
			// calculate the split metric along axis l
			//
				value = 0;
		
				for (j=0 ; j<3 ; j++)
				{
					if (j == l)
					{
						dist = plane->dist * plane->normal[l];
						value += (maxs[l]-dist)*(maxs[l]-dist);
						value += (dist-mins[l])*(dist-mins[l]);
					}
					else
						value += 2*(maxs[j]-mins[j])*(maxs[j]-mins[j]);
				}
				
				if (value > bestdistribution && k == bestvalue)
					continue;
				bestdistribution = value;
			}
		//
		// currently the best!
		//
			bestvalue = k;
			bestsurface = p;
		}

	}


	return bestsurface;
}


/*
==================
SelectPartition

Selects a surface from a linked list of surfaces to split the group on
returns NULL if the surface list can not be divided any more (a leaf)
==================
*/
surface_t *SelectPartition (surface_t *surfaces, node_t *node, qboolean usemidsplit)
{
	int			i,j;
	surface_t	*p, *bestsurface;

	//
	// count surface choices
	//
	i = 0;
	bestsurface = NULL;
	for (p=surfaces ; p ; p=p->next)
		if (!p->onnode)
		{
			i++;
			bestsurface = p;
		}
		
	if (i==0)
		return NULL;		// this is a leafnode
		
	if (i==1)
		return bestsurface;	// this is a final split
	

	if (usemidsplit) // do fast way for clipping hull
		return ChooseMidPlaneFromList (surfaces, node->mins, node->maxs);
		
	// do slow way to save poly splits for drawing hull
	return ChoosePlaneFromList (surfaces, node->mins, node->maxs);
}

//============================================================================

/*
=================
CalcSurfaceInfo

Calculates the bounding box
=================
*/
void CalcSurfaceInfo (surface_t *surf)
{
	int		i,j;
	face_t	*f;
	
	if (!surf->faces)
		Error ("CalcSurfaceInfo: surface without a face");
		
//
// calculate a bounding box
//
	for (i=0 ; i<3 ; i++)
	{
		surf->mins[i] = 99999;
		surf->maxs[i] = -99999;
	}

	for (f=surf->faces ; f ; f=f->next)
	{
		if (f->contents >= 0)
			Error ("Bad contents");
		for (i=0 ; i<f->numpoints ; i++)
			for (j=0 ; j<3 ; j++)
			{
				if (f->pts[i][j] < surf->mins[j])
					surf->mins[j] = f->pts[i][j];
				if (f->pts[i][j] > surf->maxs[j])
					surf->maxs[j] = f->pts[i][j];
			}
	}
}



/*
==================
DivideSurface
==================
*/
void DivideSurface (surface_t *in, dplane_t *split, surface_t **front, surface_t **back)
{
	face_t		*facet, *next;
	face_t		*frontlist, *backlist;
	face_t		*frontfrag, *backfrag;
	surface_t	*news;
	dplane_t	*inplane;	

	inplane = &dplanes[in->planenum];
	
	// parallel case is easy

	if (inplane->normal[0] == split->normal[0]
	&& inplane->normal[1] == split->normal[1]
	&& inplane->normal[2] == split->normal[2])
	{
		if (inplane->dist > split->dist)
		{
			*front = in;
			*back = NULL;
		}
		else if (inplane->dist < split->dist)
		{
			*front = NULL;
			*back = in;
		}
		else
		{	// split the surface into front and back
			frontlist = NULL;
			backlist = NULL;
			for (facet = in->faces ; facet ; facet = next)
			{
				next = facet->next;
				if (facet->planenum & 1)
				{
					facet->next = backlist;
					backlist = facet;
				}
				else
				{
					facet->next = frontlist;
					frontlist = facet;
				}
			}
			goto makesurfs;
		}
		return;
	}
	
// do a real split.  may still end up entirely on one side
// OPTIMIZE: use bounding box for fast test
	frontlist = NULL;
	backlist = NULL;
	
	for (facet = in->faces ; facet ; facet = next)
	{
		next = facet->next;
		SplitFace (facet, split, &frontfrag, &backfrag);
		if (frontfrag)
		{
			frontfrag->next = frontlist;
			frontlist = frontfrag;
		}
		if (backfrag)
		{
			backfrag->next = backlist;
			backlist = backfrag;
		}
	}

// if nothing actually got split, just move the in plane
makesurfs:
	if (frontlist == NULL)
	{
		*front = NULL;
		*back = in;
		in->faces = backlist;
		return;
	}

	if (backlist == NULL)
	{
		*front = in;
		*back = NULL;
		in->faces = frontlist;
		return;
	}
	

// stuff got split, so allocate one new surface and reuse in
	news = AllocSurface ();
	*news = *in;
	news->faces = backlist;
	*back = news;
	
	in->faces = frontlist;
	*front = in;
	
// recalc bboxes and flags
	CalcSurfaceInfo (news);
	CalcSurfaceInfo (in);	
}

/*
=============
SplitNodeSurfaces
=============
*/
void SplitNodeSurfaces (surface_t *surfaces, node_t *node)
{
	surface_t	*p, *next;
	surface_t	*frontlist, *backlist;
	surface_t	*frontfrag, *backfrag;
	dplane_t	*splitplane;

	splitplane = &dplanes[node->planenum];

	frontlist = NULL;
	backlist = NULL;
	
	for (p=surfaces ; p ; p=next)
	{
		next = p->next;
		DivideSurface (p, splitplane, &frontfrag, &backfrag);

		if (frontfrag)
		{
			if (!frontfrag->faces)
				Error ("surface with no faces");
			frontfrag->next = frontlist;
			frontlist = frontfrag;
		}
		if (backfrag)
		{
			if (!backfrag->faces)
				Error ("surface with no faces");
			backfrag->next = backlist;
			backlist = backfrag;
		}
	}

	node->children[0]->surfaces = frontlist;
	node->children[1]->surfaces = backlist;
}


int RankForContents (int contents)
{
	switch (contents)
	{
	case CONTENTS_EMPTY:		return 0;
	case CONTENTS_WATER:		return 1;
	case CONTENTS_TRANSLUCENT:	return 2;
	case CONTENTS_CURRENT_0:	return 3;
	case CONTENTS_CURRENT_90:	return 4;
	case CONTENTS_CURRENT_180:	return 5;
	case CONTENTS_CURRENT_270:	return 6;
	case CONTENTS_CURRENT_UP:	return 7;
	case CONTENTS_CURRENT_DOWN:	return 8;
	case CONTENTS_SLIME:		return 9;
	case CONTENTS_LAVA :		return 10;
	case CONTENTS_SKY  :		return 11;
	case CONTENTS_SOLID:		return 12;
	default:
		Error ("RankForContents: bad contents %i", contents);
	}
	return -1;
}

int ContentsForRank (int rank)
{
	switch (rank)
	{
	case -1:	return CONTENTS_SOLID;	// no faces at all
	case 0:		return CONTENTS_EMPTY;
	case 1:		return CONTENTS_WATER;
	case 2:		return CONTENTS_TRANSLUCENT;
	case 3:		return CONTENTS_CURRENT_0;
	case 4:		return CONTENTS_CURRENT_90;
	case 5:		return CONTENTS_CURRENT_180;
	case 6:		return CONTENTS_CURRENT_270;
	case 7:		return CONTENTS_CURRENT_UP;
	case 8:		return CONTENTS_CURRENT_DOWN;
	case 9:		return CONTENTS_SLIME;
	case 10:	return CONTENTS_LAVA;
	case 11:	return CONTENTS_SKY;
	case 12:	return CONTENTS_SOLID;
	default:
		Error ("ContentsForRank: bad rank %i", rank);
	}
	return -1;
}

/*
=============
FreeLeafSurfs
=============
*/
void FreeLeafSurfs (node_t *leaf)
{
	surface_t *surf, *snext;
	face_t		*f, *fnext;

	for (surf = leaf->surfaces ; surf ; surf=snext)
	{
		snext = surf->next;
		for (f=surf->faces ; f ; f=fnext)
		{
			fnext = f->next;
			FreeFace (f);
		}
		FreeSurface (surf);
	}

	leaf->surfaces = NULL;
}

/*
==================
LinkLeafFaces

Determines the contents of the leaf and creates the final list of
original faces that have some fragment inside this leaf
==================
*/
#define	MAX_LEAF_FACES	1024
void LinkLeafFaces (surface_t *planelist, node_t *leafnode)
{
	face_t		*f;
	surface_t	*surf;
	int			rank, r;
	int			nummarkfaces;
	face_t		*markfaces[MAX_LEAF_FACES];

	leafnode->faces = NULL;
	leafnode->planenum = -1;

	rank = -1;
	for ( surf = planelist ; surf ; surf = surf->next)
	{
		for (f = surf->faces ; f ; f=f->next)
		{
			r = RankForContents (f->contents);
			if (r > rank)
				rank = r;
		}
	}

	leafnode->contents = ContentsForRank (rank);

	if (leafnode->contents != CONTENTS_SOLID)
	{
		nummarkfaces = 0;
		for (surf = leafnode->surfaces ; surf ; surf=surf->next)
		{
			for (f=surf->faces ; f ; f=f->next)
			{
				if (nummarkfaces == MAX_LEAF_FACES)
					Error ("nummarkfaces == MAX_LEAF_FACES");

				markfaces[nummarkfaces++] = f->original;
			}
		}

		c_leaffaces += nummarkfaces;
		markfaces[nummarkfaces] = NULL;	// end marker
		nummarkfaces++;

		leafnode->markfaces = malloc(nummarkfaces * sizeof(*leafnode->markfaces));
		memcpy (leafnode->markfaces, markfaces, nummarkfaces * sizeof(*leafnode->markfaces));
	}

	FreeLeafSurfs (leafnode);
	leafnode->surfaces = NULL;
}


/*
==================
MakeNodePortal

create the new portal by taking the full plane winding for the cutting plane
and clipping it by all of the planes from the other portals.

Each portal tracks the node that created it, so unused nodes
can be removed later.
==================
*/
void MakeNodePortal (node_t *node)
{
	portal_t	*new_portal, *p;
	dplane_t	*plane;
	dplane_t	clipplane;
	winding_t	*w;
	int			side;

	plane = &dplanes[node->planenum];
	w = BaseWindingForPlane (plane);

	new_portal = AllocPortal ();
	new_portal->plane = *plane;
	new_portal->onnode = node;

	side = 0;	// shut up compiler warning
	for (p = node->portals ; p ; p = p->next[side])	
	{
		clipplane = p->plane;
		if (p->nodes[0] == node)
			side = 0;
		else if (p->nodes[1] == node)
		{
			clipplane.dist = -clipplane.dist;
			VectorSubtract (vec3_origin, clipplane.normal, clipplane.normal);
			side = 1;
		}
		else
			Error ("MakeNodePortal: mislinked portal");

		w = ClipWinding (w, &clipplane, true);
		if (!w)
		{
			printf ("WARNING: MakeNodePortal:new portal was clipped away from node@(%.0f,%.0f,%.0f)-(%.0f,%.0f,%.0f)\n",
					node->mins[0], node->mins[1], node->mins[2], 
					node->maxs[0], node->maxs[1], node->maxs[2]);
			FreePortal (new_portal);
			return;
		}
	}

	new_portal->winding = w;	
	AddPortalToNodes (new_portal, node->children[0], node->children[1]);
}

/*
==============
SplitNodePortals

Move or split the portals that bound node so that the node's
children have portals instead of node.
==============
*/
void SplitNodePortals (node_t *node)
{
	portal_t	*p, *next_portal, *new_portal;
	node_t		*f, *b, *other_node;
	int			side;
	dplane_t	*plane;
	winding_t	*frontwinding, *backwinding;

	plane = &dplanes[node->planenum];
	f = node->children[0];
	b = node->children[1];

	for (p = node->portals ; p ; p = next_portal)	
	{
		if (p->nodes[0] == node)
			side = 0;
		else if (p->nodes[1] == node)
			side = 1;
		else
			Error ("CutNodePortals_r: mislinked portal");
		next_portal = p->next[side];

		other_node = p->nodes[!side];
		RemovePortalFromNode (p, p->nodes[0]);
		RemovePortalFromNode (p, p->nodes[1]);

//
// cut the portal into two portals, one on each side of the cut plane
//
		DivideWinding (p->winding, plane, &frontwinding, &backwinding);
		
		if (!frontwinding)
		{
			if (side == 0)
				AddPortalToNodes (p, b, other_node);
			else
				AddPortalToNodes (p, other_node, b);
			continue;
		}
		if (!backwinding)
		{
			if (side == 0)
				AddPortalToNodes (p, f, other_node);
			else
				AddPortalToNodes (p, other_node, f);
			continue;
		}
		
	// the winding is split
		new_portal = AllocPortal ();
		*new_portal = *p;
		new_portal->winding = backwinding;
		FreeWinding (p->winding);
		p->winding = frontwinding;

		if (side == 0)
		{
			AddPortalToNodes (p, f, other_node);
			AddPortalToNodes (new_portal, b, other_node);
		}
		else
		{
			AddPortalToNodes (p, other_node, f);
			AddPortalToNodes (new_portal, other_node, b);
		}
	}

	node->portals = NULL;
}


/*
==================
CalcNodeBounds

Determines the boundaries of a node by
minmaxing all the portal points, whcih
completely enclose the node.

 Returns true if the node should be midsplit.(very large)
==================
*/
qboolean CalcNodeBounds (node_t *node)
{
	int		i, j;
	vec_t	v;
	portal_t	*p, *next_portal;
	int		side;

	node->mins[0] = node->mins[1] = node->mins[2] = 9999;
	node->maxs[0] = node->maxs[1] = node->maxs[2] = -9999;

	for (p = node->portals ; p ; p = next_portal)	
	{
		if (p->nodes[0] == node)
			side = 0;
		else if (p->nodes[1] == node)
			side = 1;
		else
			Error ("CutNodePortals_r: mislinked portal");
		next_portal = p->next[side];

		for (i=0 ; i<p->winding->numpoints ; i++)
		{
			for (j=0 ; j<3 ; j++)
			{
				v = p->winding->points[i][j];
				if (v < node->mins[j])
					node->mins[j] = v;
				if (v > node->maxs[j])
					node->maxs[j] = v;
			}
		}
	}

	for (i=0 ; i<3 ; i++)
		if (node->maxs[i] - node->mins[i] > 1024)
			return true;
	return false;
}

/*
==================
CopyFacesToNode

Do a final merge attempt, then subdivide the faces
to surface cache size if needed.

These are final faces that will be drawable in the game.
Copies of these faces are further chopped up into the leafs,
but they will reference these originals.
==================
*/
void CopyFacesToNode (node_t *node, surface_t *surf)
{
	face_t	**prevptr, *f, *newf;

	// merge as much as possible
	MergePlaneFaces (surf);

	// subdivide large faces
	prevptr = &surf->faces;
	while (1)
	{
		f = *prevptr;
		if (!f)
			break;
		SubdivideFace (f, prevptr);
		f = *prevptr;
		prevptr = &f->next;
	}

	// copy the faces to the node, and consider them the originals
	node->surfaces = NULL;
	node->faces = NULL;
	for (f=surf->faces ; f ; f=f->next)
	{
		if (f->contents != CONTENTS_SOLID)
		{
			newf = AllocFace ();
			*newf = *f;
			f->original = newf;
			newf->next = node->faces;
			node->faces = newf;
			c_nodefaces++;
		}
	}
}

/*
==================
DrawSurfaces
==================
*/
void DrawSurfaces (surface_t *surf)
{
	face_t	*f;

	Draw_ClearWindow ();
	for ( ; surf ; surf=surf->next)
	{
		for (f = surf->faces ; f ; f=f->next)
		{
			Draw_DrawFace (f);
		}
	}
}

/*
==================
BuildBspTree_r
==================
*/
void BuildBspTree_r (node_t *node)
{
	surface_t		*split;
	qboolean		midsplit;
	surface_t		*allsurfs;

	midsplit = CalcNodeBounds (node);

	DrawSurfaces (node->surfaces);

	split = SelectPartition (node->surfaces, node, midsplit);
	if (!split)
	{	// this is a leaf node
		node->planenum = PLANENUM_LEAF;
		LinkLeafFaces (node->surfaces, node);
		return;
	}

	//
	// these are final polygons
	//
	split->onnode = node;	// can't use again
	allsurfs = node->surfaces;
	node->planenum = split->planenum;
	node->faces = NULL;
	CopyFacesToNode (node, split);
	c_splitnodes++;

	node->children[0] = AllocNode ();
	node->children[1] = AllocNode ();

	//
	// split all the polysurfaces into front and back lists
	//
	SplitNodeSurfaces (allsurfs, node);

	//
	// create the portal that seperates the two children
	//
	MakeNodePortal (node);
	
	//
	// carve the portals on the boundaries of the node
	//
	SplitNodePortals (node);

	//
	// recursively do the children
	//
	BuildBspTree_r (node->children[0]);
	BuildBspTree_r (node->children[1]);
}

/*
==================
SolidBSP

Takes a chain of surfaces plus a split type, and
returns a bsp tree with faces off the nodes.

The original surface chain will be completely freed.
==================
*/
node_t *SolidBSP (surfchain_t *surfhead)
{
	int		i;
	node_t	*headnode;
	
	qprintf ("----- SolidBSP -----\n");

	headnode = AllocNode ();
	headnode->surfaces = surfhead->surfaces;
	
	Draw_ClearWindow ();
	c_splitnodes = 0;
	c_nodefaces = 0;
	c_leaffaces = 0;

	if (!surfhead->surfaces)
	{
		// nothing at all to build
		headnode->planenum = -1;
		headnode->contents = CONTENTS_EMPTY;
		return headnode;
	}

	//
	// generate six portals that enclose the entire world
	//
	MakeHeadnodePortals (headnode, surfhead->mins, surfhead->maxs);
	
	//
	// recursively partition everything
	//
	BuildBspTree_r (headnode);

	qprintf ("%5i split nodes\n", c_splitnodes);
	qprintf ("%5i node faces\n", c_nodefaces);
	qprintf ("%5i leaf faces\n", c_leaffaces);
	
	return headnode;
}

