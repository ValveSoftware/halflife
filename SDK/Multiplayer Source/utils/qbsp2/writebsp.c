/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/


#include "bsp5.h"

//===========================================================================

/*
==================
WriteClipNodes_r

==================
*/
int WriteClipNodes_r (node_t *node)
{
	int			i, c;
	dclipnode_t	*cn;
	int			num;
	
	if (node->planenum == -1)
	{
		num = node->contents;
		free (node->markfaces);
		free (node);
		return num;
	}
	
// emit a clipnode
	if (numclipnodes == MAX_MAP_CLIPNODES)
		Error ("MAX_MAP_CLIPNODES");
	c = numclipnodes;
	cn = &dclipnodes[numclipnodes];
	numclipnodes++;
	if (node->planenum & 1)
		Error ("WriteClipNodes_r: odd planenum");
	cn->planenum = node->planenum;
	for (i=0 ; i<2 ; i++)
		cn->children[i] = WriteClipNodes_r(node->children[i]);
	
	free (node);
	return c;
}

/*
==================
WriteClipNodes

Called after the clipping hull is completed.  Generates a disk format
representation and frees the original memory.
==================
*/
void WriteClipNodes (node_t *nodes)
{
	WriteClipNodes_r (nodes);
}

//===========================================================================

/*
==================
WriteDrawLeaf
==================
*/
void WriteDrawLeaf (node_t *node)
{
	face_t		**fp, *f;
	dleaf_t		*leaf_p;
		
// emit a leaf
	leaf_p = &dleafs[numleafs];
	numleafs++;

	leaf_p->contents = node->contents;

//
// write bounding box info
//	
	VectorCopy (node->mins, leaf_p->mins);
	VectorCopy (node->maxs, leaf_p->maxs);
	
	leaf_p->visofs = -1;	// no vis info yet
	
//
// write the marksurfaces
//
	leaf_p->firstmarksurface = nummarksurfaces;
	
	for (fp=node->markfaces ; *fp ; fp++)
	{
	// emit a marksurface
		f = *fp;
		do
		{
			dmarksurfaces[nummarksurfaces] =  f->outputnumber;
			if (nummarksurfaces >= MAX_MAP_MARKSURFACES)
				Error ("nummarksurfaces == MAX_MAP_MARKSURFACES");
			nummarksurfaces++;
			f=f->original;		// grab tjunction split faces
		} while (f);
	}
	free (node->markfaces);
	
	leaf_p->nummarksurfaces = nummarksurfaces - leaf_p->firstmarksurface;
}

/*
==================
WriteFace
==================
*/
void WriteFace (face_t *f)
{
	dface_t	*df;
	int		i;
	int		e;

	f->outputnumber = numfaces;

	df = &dfaces[numfaces];
	if (numfaces >= MAX_MAP_FACES)
		Error ("numfaces == MAX_MAP_FACES");
	numfaces++;

	df->planenum = f->planenum & (~1);
	df->side = f->planenum & 1;
	df->firstedge = numsurfedges;
	df->numedges = f->numpoints;
	df->texinfo = f->texturenum;
	for (i=0 ; i<f->numpoints ; i++)
	{
		e = GetEdge (f->pts[i], f->pts[(i+1)%f->numpoints], f);
		if (numsurfedges >= MAX_MAP_SURFEDGES)
			Error ("numsurfedges == MAX_MAP_SURFEDGES");
		dsurfedges[numsurfedges] = e;
		numsurfedges++;
	}
}

/*
==================
WriteDrawNodes_r
==================
*/
void WriteDrawNodes_r (node_t *node)
{
	dnode_t	*n;
	int		i;
	face_t	*f, *next;

// emit a node	
	if (numnodes == MAX_MAP_NODES)
		Error ("numnodes == MAX_MAP_NODES");
	n = &dnodes[numnodes];
	numnodes++;

	VectorCopy (node->mins, n->mins);
	VectorCopy (node->maxs, n->maxs);

	if (node->planenum & 1)
		Error ("WriteDrawNodes_r: odd planenum");
	n->planenum = node->planenum;
	n->firstface = numfaces;

	for (f=node->faces ; f ; f=f->next)
		WriteFace (f);

	n->numfaces = numfaces - n->firstface;

	//
	// recursively output the other nodes
	//	
	for (i=0 ; i<2 ; i++)
	{
		if (node->children[i]->planenum == -1)
		{
			if (node->children[i]->contents == CONTENTS_SOLID)
				n->children[i] = -1;
			else
			{
				n->children[i] = -(numleafs + 1);
				WriteDrawLeaf (node->children[i]);
			}
		}
		else
		{
			n->children[i] = numnodes;	
			WriteDrawNodes_r (node->children[i]);
		}
	}
}

/*
===========
FreeDrawNodes_r
===========
*/
void FreeDrawNodes_r (node_t *node)
{
	int		i;
	face_t	*f, *next;

	for (i=0 ; i<2 ; i++)
		if (node->children[i]->planenum != -1)
			FreeDrawNodes_r (node->children[i]);

	//
	// free the faces on the node
	//
	for (f=node->faces ; f ; f=next)
	{
		next = f->next;
		FreeFace (f);
	}

	free (node);
}

/*
==================
WriteDrawNodes

Called after a drawing hull is completed
Frees all nodes and faces.
==================
*/
void WriteDrawNodes (node_t *headnode)
{
	if (headnode->contents < 0)	
		WriteDrawLeaf (headnode);
	else
	{
		WriteDrawNodes_r (headnode);
		FreeDrawNodes_r (headnode);
	}
}


//===========================================================================

/*
==================
BeginBSPFile
==================
*/
void BeginBSPFile (void)
{
	// these values may actually be initialized
	// if the file existed when loaded, so clear them explicitly
	nummodels = 0;
	numfaces = 0;
	numnodes = 0;
	numclipnodes = 0;
	numvertexes = 0;
	nummarksurfaces = 0;
	numsurfedges = 0;

	// edge 0 is not used, because 0 can't be negated
	numedges = 1;

// leaf 0 is common solid with no faces
	numleafs = 1;
	dleafs[0].contents = CONTENTS_SOLID;
}


/*
==================
FinishBSPFile
==================
*/
void FinishBSPFile (void)
{
	int		i;

	qprintf ("--- FinishBSPFile ---\n");

	if (verbose)
		PrintBSPFileSizes ();
	WriteBSPFile (bspfilename);
}

