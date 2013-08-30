/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#include "vis.h"

int		c_fullskip;
int		c_chains;
int		c_portalskip, c_leafskip;
int		c_vistest, c_mighttest;

int		active;

void CheckStack (leaf_t *leaf, threaddata_t *thread)
{
	pstack_t	*p;

	for (p=thread->pstack_head.next ; p ; p=p->next)
	{
//		printf ("=");
		if (p->leaf == leaf)
			Error ("CheckStack: leaf recursion");
	}
//	printf ("\n");
}


winding_t *AllocStackWinding (pstack_t *stack)
{
	int		i;

	for (i=0 ; i<3 ; i++)
	{
		if (stack->freewindings[i])
		{
			stack->freewindings[i] = 0;
			return &stack->windings[i];
		}
	}

	Error ("AllocStackWinding: failed");

	return NULL;
}

void FreeStackWinding (winding_t *w, pstack_t *stack)
{
	int		i;

	i = w - stack->windings;

	if (i<0 || i>2)
		return;		// not from local

	if (stack->freewindings[i])
		Error ("FreeStackWinding: allready free");
	stack->freewindings[i] = 1;
}

/*
==============
ChopWinding

==============
*/
winding_t	*ChopWinding (winding_t *in, pstack_t *stack, plane_t *split)
{
	vec_t	dists[128];
	int		sides[128];
	int		counts[3];
	vec_t	dot;
	int		i, j;
	vec_t	*p1, *p2;
	vec3_t	mid;
	winding_t	*neww;
	int		maxpts;

	counts[0] = counts[1] = counts[2] = 0;

	if ( in->numpoints > (sizeof(sides)/sizeof(*sides)) )
		Error("Winding with too many sides!");

// determine sides for each point
	for (i=0 ; i<in->numpoints ; i++)
	{
		dot = DotProduct (in->points[i], split->normal);
		dot -= split->dist;
		dists[i] = dot;
		if (dot > ON_EPSILON)
			sides[i] = SIDE_FRONT;
		else if (dot < -ON_EPSILON)
			sides[i] = SIDE_BACK;
		else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}

	if (!counts[1])
		return in;		// completely on front side
	
	if (!counts[0])
	{
		FreeStackWinding (in, stack);
		return NULL;
	}

	sides[i] = sides[0];
	dists[i] = dists[0];
	
	neww = AllocStackWinding (stack);

	neww->numpoints = 0;

	for (i=0 ; i<in->numpoints ; i++)
	{
		p1 = in->points[i];

		if (neww->numpoints == MAX_POINTS_ON_FIXED_WINDING)
		{
			FreeStackWinding (neww, stack);
			return in;		// can't chop -- fall back to original
		}

		if (sides[i] == SIDE_ON)
		{
			VectorCopy (p1, neww->points[neww->numpoints]);
			neww->numpoints++;
			continue;
		}
	
		if (sides[i] == SIDE_FRONT)
		{
			VectorCopy (p1, neww->points[neww->numpoints]);
			neww->numpoints++;
		}
		
		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;
			
		if (neww->numpoints == MAX_POINTS_ON_FIXED_WINDING)
		{
			FreeStackWinding (neww, stack);
			return in;		// can't chop -- fall back to original
		}

	// generate a split point
		p2 = in->points[(i+1)%in->numpoints];
		
		dot = dists[i] / (dists[i]-dists[i+1]);
		for (j=0 ; j<3 ; j++)
		{	// avoid round off error when possible
			if (split->normal[j] == 1)
				mid[j] = split->dist;
			else if (split->normal[j] == -1)
				mid[j] = -split->dist;
			else
				mid[j] = p1[j] + dot*(p2[j]-p1[j]);
		}
			
		VectorCopy (mid, neww->points[neww->numpoints]);
		neww->numpoints++;
	}
	
// free the original winding
	FreeStackWinding (in, stack);
	
	return neww;
}

/*
==============
InTheBallpark

Build a bounding box using the start and end windings
then verify that the clip winding bounding box touches
the start/end bounding box.
  
==============
*/

int
InTheBallpark( winding_t *start, winding_t *clip, winding_t *end )
{
	int			d,p;
	vec3_t		bmin = {9999,9999,9999}, bmax = {-9999,-9999,-9999};
	vec3_t		cmin = {9999,9999,9999}, cmax = {-9999,-9999,-9999};
	vec3_t		bcenter, bsize;
	vec3_t		ccenter, csize;


	for(d=0; d<3; d++)
	{
		// Establish a bounding box based on start winding
		for (p=0; p<start->numpoints; p++)
		{
			if (start->points[p][d] < bmin[d])
				bmin[d] = start->points[p][d];
			if (start->points[p][d] > bmax[d])
				bmax[d] = start->points[p][d];
		}
		// Extend this bounding box based on end winding
		for (p=0; p<end->numpoints; p++)
		{
			if (end->points[p][d] < bmin[d])
				bmin[d] = end->points[p][d];
			if (end->points[p][d] > bmax[d])
				bmax[d] = end->points[p][d];
		}
		// Establish a second box based on clip winding
		for (p=0; p<clip->numpoints; p++)
		{
			if (clip->points[p][d] < cmin[d])
				cmin[d] = clip->points[p][d];
			if (clip->points[p][d] > cmax[d])
				cmax[d] = clip->points[p][d];
		}
		// Calculate the center of each bounding box
		bcenter[d] = (bmax[d]+bmin[d]); // Optimized out /2;
		ccenter[d] = (cmax[d]+cmin[d]); // Optimized out /2;

		// Calculate the distances from center to the edges
		bsize[d] = (bmax[d] - bmin[d]); // Optimized out /2;
		csize[d] = (cmax[d] - cmin[d]); // Optimized out /2;

		// Are the centers further apart than the distance to the edges
		if ( fabs(bcenter[d]-ccenter[d]) > bsize[d]+csize[d]+ON_EPSILON )
			return 0;
	}
	return 1;
}

/*
==============
ClipToSeperators

Source, pass, and target are an ordering of portals.

Generates seperating planes canidates by taking two points from source and one
point from pass, and clips target by them.

If target is totally clipped away, that portal can not be seen through.

Normal clip keeps target on the same side as pass, which is correct if the
order goes source, pass, target.  If the order goes pass, source, target then
flipclip should be set.
==============
*/
winding_t	*ClipToSeperators (winding_t *source, winding_t *pass, winding_t *target, qboolean flipclip, pstack_t *stack)
{
	int			i, j, k, l;
	plane_t		plane;
	vec3_t		v1, v2;
	float		d;
	vec_t		length;
	int			counts[3];
	qboolean		fliptest;

	// check all combinations	
	for (i=0 ; i<source->numpoints ; i++)
	{
		l = (i+1)%source->numpoints;
		VectorSubtract (source->points[l] , source->points[i], v1);

	// fing a vertex of pass that makes a plane that puts all of the
	// vertexes of pass on the front side and all of the vertexes of
	// source on the back side
		for (j=0 ; j<pass->numpoints ; j++)
		{
			VectorSubtract (pass->points[j], source->points[i], v2);

			plane.normal[0] = v1[1]*v2[2] - v1[2]*v2[1];
			plane.normal[1] = v1[2]*v2[0] - v1[0]*v2[2];
			plane.normal[2] = v1[0]*v2[1] - v1[1]*v2[0];
			
		// if points don't make a valid plane, skip it

			length = plane.normal[0] * plane.normal[0]
			+ plane.normal[1] * plane.normal[1]
			+ plane.normal[2] * plane.normal[2];
			
			if (length < ON_EPSILON)
				continue;

			length = 1/sqrt(length);
			
			plane.normal[0] *= length;
			plane.normal[1] *= length;
			plane.normal[2] *= length;

			plane.dist = DotProduct (pass->points[j], plane.normal);

		//
		// find out which side of the generated seperating plane has the
		// source portal
		//
#if 1
			fliptest = false;
			for (k=0 ; k<source->numpoints ; k++)
			{
				if (k == i || k == l)
					continue;
				d = DotProduct (source->points[k], plane.normal) - plane.dist;
				if (d < -ON_EPSILON)
				{	// source is on the negative side, so we want all
					// pass and target on the positive side
					fliptest = false;
					break;
				}
				else if (d > ON_EPSILON)
				{	// source is on the positive side, so we want all
					// pass and target on the negative side
					fliptest = true;
					break;
				}
			}
			if (k == source->numpoints)
				continue;		// planar with source portal
#else
			fliptest = flipclip;
#endif
		//
		// flip the normal if the source portal is backwards
		//
			if (fliptest)
			{
				VectorSubtract (vec3_origin, plane.normal, plane.normal);
				plane.dist = -plane.dist;
			}
#if 1
		//
		// if all of the pass portal points are now on the positive side,
		// this is the seperating plane
		//
			counts[0] = counts[1] = counts[2] = 0;
			for (k=0 ; k<pass->numpoints ; k++)
			{
				if (k==j)
					continue;
				d = DotProduct (pass->points[k], plane.normal) - plane.dist;
				if (d < -ON_EPSILON)
					break;
				else if (d > ON_EPSILON)
					counts[0]++;
				else
					counts[2]++;
			}
			if (k != pass->numpoints)
				continue;	// points on negative side, not a seperating plane
				
			if (!counts[0])
				continue;	// planar with seperating plane
#else
			k = (j+1)%pass->numpoints;
			d = DotProduct (pass->points[k], plane.normal) - plane.dist;
			if (d < -ON_EPSILON)
				continue;
			k = (j+pass->numpoints-1)%pass->numpoints;
			d = DotProduct (pass->points[k], plane.normal) - plane.dist;
			if (d < -ON_EPSILON)
				continue;			
#endif
		//
		// flip the normal if we want the back side
		//
			if (flipclip)
			{
				VectorSubtract (vec3_origin, plane.normal, plane.normal);
				plane.dist = -plane.dist;
			}
			
		//
		// clip target by the seperating plane
		//
			target = ChopWinding (target, stack, &plane);
			if (!target)
				return NULL;		// target is not visible
		}
	}
	
	return target;
}



/*
==================
RecursiveLeafFlow

Flood fill through the leafs
If src_portal is NULL, this is the originating leaf
==================
*/
void RecursiveLeafFlow (int leafnum, threaddata_t *thread, pstack_t *prevstack)
{
	pstack_t	stack;
	portal_t	*p;
	plane_t		backplane;
	leaf_t 		*leaf;
	int			i, j;
	long		*test, *might, *vis, more;
	int			pnum;

	c_chains++;

	leaf = &leafs[leafnum];
//	CheckStack (leaf, thread);
	
// mark the leaf as visible
	if (! (thread->leafvis[leafnum>>3] & (1<<(leafnum&7)) ) )
	{
		thread->leafvis[leafnum>>3] |= 1<<(leafnum&7);
		thread->base->numcansee++;
	}
	
	prevstack->next = &stack;

	stack.next = NULL;
	stack.leaf = leaf;
	stack.portal = NULL;

	might = (long *)stack.mightsee;
	vis = (long *)thread->leafvis;
	
// check all portals for flowing into other leafs	
	for (i=0 ; i<leaf->numportals ; i++)
	{
		p = leaf->portals[i];

		if ( ! (prevstack->mightsee[p->leaf>>3] & (1<<(p->leaf&7)) ) )
		{
			c_leafskip++;
			continue;	// can't possibly see it
		}
#if 0
		pnum = p - portals;
		if ( (thread->fullportal[pnum>>3] & (1<<(pnum&7)) ) )
		{
			c_fullskip++;
			continue;	// allready have full vis info
		}
#endif
	// if the portal can't see anything we haven't allready seen, skip it
		if (p->status == stat_done)
		{
			c_vistest++;
			test = (long *)p->visbits;
		}
		else
		{
			c_mighttest++;
			test = (long *)p->mightsee;
		}

		more = 0;
		for (j=0 ; j<bitlongs ; j++)
		{
			might[j] = ((long *)prevstack->mightsee)[j] & test[j];
			more |= (might[j] & ~vis[j]);
		}
		
		if (!more)
		{	// can't see anything new
			c_portalskip++;
			continue;
		}

		// get plane of portal, point normal into the neighbor leaf
		stack.portalplane = p->plane;
		VectorSubtract (vec3_origin, p->plane.normal, backplane.normal);
		backplane.dist = -p->plane.dist;
			
		if (VectorCompare (prevstack->portalplane.normal, backplane.normal) )
			continue;	// can't go out a coplanar face
	
		c_portalcheck++;
		
		stack.portal = p;
		stack.next = NULL;
		stack.freewindings[0] = 1;
		stack.freewindings[1] = 1;
		stack.freewindings[2] = 1;

		stack.pass = ChopWinding (p->winding, &stack, &thread->pstack_head.portalplane);
		if (!stack.pass)
			continue;
			
		stack.source = ChopWinding (prevstack->source, &stack, &backplane);
		if (!stack.source)
			continue;

		if (!prevstack->pass)
		{	// the second leaf can only be blocked if coplanar
			RecursiveLeafFlow (p->leaf, thread, &stack);
			continue;
		}

		stack.pass = ChopWinding (stack.pass, &stack, &prevstack->portalplane);
		if (!stack.pass)
			continue;
		
		c_portaltest++;

#ifdef NOT_BROKEN
        if (!InTheBallpark(stack.source, prevstack->pass, stack.pass))
		{
			FreeStackWinding (stack.pass, &stack);
			stack.pass = NULL;
			continue;
		}
#endif
		stack.pass = ClipToSeperators (stack.source, prevstack->pass, stack.pass, false, &stack);
		if (!stack.pass)
			continue;
		
		stack.pass = ClipToSeperators (prevstack->pass, stack.source, stack.pass, true, &stack);
		if (!stack.pass)
			continue;

		c_portalpass++;
#if 0
		if (stack.pass == p->winding)
		{
			thread->fullportal[pnum>>3] |= (1<<(pnum&7));
			FreeStackWinding (stack.source, &stack);
			stack.source = ChopWinding (thread->base->winding, &stack, &backplane);
			for (j=0 ; j<bitlongs ; j++)
				might[j] = ((long *)thread->pstack_head.mightsee)[j] & test[j];
		}
#endif
	// flow through it for real
		RecursiveLeafFlow (p->leaf, thread, &stack);
	}	
}


/*
===============
PortalFlow

===============
*/
void PortalFlow (portal_t *p)
{
	threaddata_t	data;
	int				i;

	if (p->status != stat_working)
		Error ("PortalFlow: reflowed");
	p->status = stat_working;
	
	p->visbits = malloc (bitbytes);
	memset (p->visbits, 0, bitbytes);

	memset (&data, 0, sizeof(data));
	data.leafvis = p->visbits;
	data.base = p;
	
	data.pstack_head.portal = p;
	data.pstack_head.source = p->winding;
	data.pstack_head.portalplane = p->plane;
	for (i=0 ; i<bitlongs ; i++)
		((long *)data.pstack_head.mightsee)[i] = ((long *)p->mightsee)[i];
	RecursiveLeafFlow (p->leaf, &data, &data.pstack_head);

	p->status = stat_done;
}


/*
===============================================================================

This is a rough first-order aproximation that is used to trivially reject some
of the final calculations.

===============================================================================
*/

void SimpleFlood (portal_t *srcportal, int leafnum, byte *portalsee, int *c_leafsee)
{
	int		i;
	leaf_t	*leaf;
	portal_t	*p;
	
	if (srcportal->mightsee[leafnum>>3] & (1<<(leafnum&7)) )
		return;
	srcportal->mightsee[leafnum>>3] |= (1<<(leafnum&7));
	(*c_leafsee)++;
	
	leaf = &leafs[leafnum];
	
	for (i=0 ; i<leaf->numportals ; i++)
	{
		p = leaf->portals[i];
		if ( !portalsee[ p - portals ] )
			continue;
		SimpleFlood (srcportal, p->leaf, portalsee, c_leafsee);
	}
}


/*
==============
BasePortalVis
==============
*/
void BasePortalVis (int threadnum)
{
	int			i, j, k;
	portal_t	*tp, *p;
	float		d;
	winding_t	*w;
	byte	portalsee[MAX_PORTALS];
	int		c_leafsee;

	
	while (1)
	{
		i = GetThreadWork ();
		if (i == -1)
			break;
		p = portals+i;

		p->mightsee = malloc (bitbytes);
		memset (p->mightsee, 0, bitbytes);
		
		memset (portalsee, 0, numportals*2);

		for (j=0, tp = portals ; j<numportals*2 ; j++, tp++)
		{
			if (j == i)
				continue;
			w = tp->winding;
			for (k=0 ; k<w->numpoints ; k++)
			{
				d = DotProduct (w->points[k], p->plane.normal)
					- p->plane.dist;
				if (d > ON_EPSILON)
					break;
			}
			if (k == w->numpoints)
				continue;	// no points on front
				
			
			w = p->winding;
			for (k=0 ; k<w->numpoints ; k++)
			{
				d = DotProduct (w->points[k], tp->plane.normal)
					- tp->plane.dist;
				if (d < -ON_EPSILON)
					break;
			}
			if (k == w->numpoints)
				continue;	// no points on front

			portalsee[j] = 1;					
		}

		c_leafsee = 0;
		SimpleFlood (p, p->leaf, portalsee, &c_leafsee);
		p->nummightsee = c_leafsee;
//		printf ("portal:%4i  c_leafsee:%4i \n", i, c_leafsee);
	
	}

}














