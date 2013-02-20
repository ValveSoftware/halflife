/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

// merge.c

#include "bsp5.h"


// #define CONTINUOUS_EPSILON	0.001
#define CONTINUOUS_EPSILON	ON_EPSILON

/*
================
CheckColinear

================
*/
void CheckColinear (face_t *f)
{
	int			i, j;
	vec3_t		v1, v2;
	
	for (i=0 ; i<f->numpoints ;i++)
	{
// skip the point if the vector from the previous point is the same
// as the vector to the next point
		j = (i - 1 < 0) ? f->numpoints - 1 : i - 1;
		VectorSubtract (f->pts[i], f->pts[j], v1);
		VectorNormalize (v1);
		
		j = (i + 1 == f->numpoints) ? 0 : i + 1;
		VectorSubtract (f->pts[j], f->pts[i], v2);
		VectorNormalize (v2);
		
		if (VectorCompare (v1, v2))
			Error ("Colinear edge");			
	}
	
}


/*
=============
TryMerge

If two polygons share a common edge and the edges that meet at the
common points are both inside the other polygons, merge them

Returns NULL if the faces couldn't be merged, or the new face.
The originals will NOT be freed.
=============
*/
face_t *TryMerge (face_t *f1, face_t *f2)
{
	vec_t		*p1, *p2, *p3, *p4, *back;
	face_t		*newf;
	int			i, j, k, l;
	vec3_t		normal, delta, planenormal;
	vec_t		dot;
	dplane_t	*plane;
	qboolean		keep1, keep2;
	
	if (f1->numpoints == -1 || f2->numpoints == -1)
		return NULL;
	if (f1->texturenum != f2->texturenum)
		return NULL;
	if (f1->contents != f2->contents)
		return NULL;
		
//
// find a common edge
//	
	p1 = p2 = NULL;	// stop compiler warning
	j = 0;			// 
	
	for (i=0 ; i<f1->numpoints ; i++)
	{
		p1 = f1->pts[i];
		p2 = f1->pts[(i+1)%f1->numpoints];
		for (j=0 ; j<f2->numpoints ; j++)
		{
			p3 = f2->pts[j];
			p4 = f2->pts[(j+1)%f2->numpoints];
			for (k=0 ; k<3 ; k++)
			{
				if (fabs(p1[k] - p4[k]) > EQUAL_EPSILON)
					break;
				if (fabs(p2[k] - p3[k]) > EQUAL_EPSILON)
					break;
			}
			if (k==3)
				break;
		}
		if (j < f2->numpoints)
			break;
	}
	
	if (i == f1->numpoints)
		return NULL;			// no matching edges

//
// check slope of connected lines
// if the slopes are colinear, the point can be removed
//
	plane = &dplanes[f1->planenum];
	VectorCopy (plane->normal, planenormal);
		
	back = f1->pts[(i+f1->numpoints-1)%f1->numpoints];
	VectorSubtract (p1, back, delta);
	CrossProduct (planenormal, delta, normal);
	VectorNormalize (normal);
	
	back = f2->pts[(j+2)%f2->numpoints];
	VectorSubtract (back, p1, delta);
	dot = DotProduct (delta, normal);
	if (dot > CONTINUOUS_EPSILON)
		return NULL;			// not a convex polygon
	keep1 = dot < -CONTINUOUS_EPSILON;
	
	back = f1->pts[(i+2)%f1->numpoints];
	VectorSubtract (back, p2, delta);
	CrossProduct (planenormal, delta, normal);
	VectorNormalize (normal);

	back = f2->pts[(j+f2->numpoints-1)%f2->numpoints];
	VectorSubtract (back, p2, delta);
	dot = DotProduct (delta, normal);
	if (dot > CONTINUOUS_EPSILON)
		return NULL;			// not a convex polygon
	keep2 = dot < -CONTINUOUS_EPSILON;

//
// build the new polygon
//
	if (f1->numpoints + f2->numpoints > MAXEDGES)
	{
//		Error ("TryMerge: too many edges!");
		return NULL;
	}

	newf = NewFaceFromFace (f1);
	
// copy first polygon
	for (k=(i+1)%f1->numpoints ; k != i ; k=(k+1)%f1->numpoints)
	{
		if (k==(i+1)%f1->numpoints && !keep2)
			continue;
		
		VectorCopy (f1->pts[k], newf->pts[newf->numpoints]);
		newf->numpoints++;
	}
	
// copy second polygon
	for (l= (j+1)%f2->numpoints ; l != j ; l=(l+1)%f2->numpoints)
	{
		if (l==(j+1)%f2->numpoints && !keep1)
			continue;
		VectorCopy (f2->pts[l], newf->pts[newf->numpoints]);
		newf->numpoints++;
	}

	return newf;
}


/*
===============
MergeFaceToList
===============
*/
qboolean	mergedebug;
face_t *MergeFaceToList (face_t *face, face_t *list)
{	
	face_t	*newf, *f;
	
	for (f=list ; f ; f=f->next)
	{
//CheckColinear (f);		
if (mergedebug)
{
Draw_ClearWindow ();
Draw_DrawFace (face);
Draw_DrawFace (f);
Draw_SetBlack ();
}
		newf = TryMerge (face, f);
		if (!newf)
			continue;
		FreeFace (face);
		f->numpoints = -1;		// merged out
		return MergeFaceToList (newf, list);
	}
	
// didn't merge, so add at start
	face->next = list;
	return face;
}


/*
===============
FreeMergeListScraps
===============
*/
face_t *FreeMergeListScraps (face_t *merged)
{
	face_t	*head, *next;
	
	head = NULL;
	for ( ; merged ; merged = next)
	{
		next = merged->next;
		if (merged->numpoints == -1)
			FreeFace (merged);
		else
		{
			merged->next = head;
			head = merged;
		}
	}

	return head;
}


/*
===============
MergePlaneFaces
===============
*/
void MergePlaneFaces (surface_t *plane)
{
	face_t	*f1, *next;
	face_t	*merged;
	
	merged = NULL;
	
	for (f1 = plane->faces ; f1 ; f1 = next)
	{
		next = f1->next;
		merged = MergeFaceToList (f1, merged);
	}

// chain all of the non-empty faces to the plane
	plane->faces = FreeMergeListScraps (merged);
}


/*
============
MergeAll
============
*/
void MergeAll (surface_t *surfhead)
{
	surface_t       *surf;
	int                     mergefaces;
	face_t          *f;
	
	qprintf ("---- MergeAll ----\n");

	mergefaces = 0; 
	for (surf = surfhead ; surf ; surf=surf->next)
	{
		MergePlaneFaces (surf);
Draw_ClearWindow ();
		for (f=surf->faces ; f ; f=f->next)
		{
Draw_DrawFace (f);
			mergefaces++;
		}
	}
	
	qprintf ("%i mergefaces\n", mergefaces);
}
