/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/


#include "cmdlib.h"
#include "mathlib.h"
#include "polylib.h"

int	c_active_windings;
int	c_peak_windings;
int	c_winding_allocs;
int	c_winding_points;

#define	BOGUS_RANGE	8192

void pw(winding_t *w)
{
	int		i;
	for (i=0 ; i<w->numpoints ; i++)
		printf ("(%5.1f, %5.1f, %5.1f)\n",w->p[i][0], w->p[i][1],w->p[i][2]);
}


/*
=============
AllocWinding
=============
*/
winding_t	*AllocWinding (int points)
{
	winding_t	*w;
	int			s;

	c_winding_allocs++;
	c_winding_points += points;
	c_active_windings++;
	if (c_active_windings > c_peak_windings)
		c_peak_windings = c_active_windings;
	s = sizeof(vec_t)*3*points + sizeof(int);
	s += sizeof(vec_t) - sizeof(w->numpoints);		// padding

	w = malloc (s);
	memset (w, 0, s); 

	return w;
}

void FreeWinding (winding_t *w)
{
	c_active_windings--;
	free (w);
}

/*
============
RemoveColinearPoints
============
*/
int	c_removed;

void	RemoveColinearPoints (winding_t *w)
{
	int		i, j, k;
	vec3_t	v1, v2;
	int		nump;
	vec3_t	p[MAX_POINTS_ON_WINDING];

	nump = 0;
	for (i=0 ; i<w->numpoints ; i++)
	{
		j = (i+1)%w->numpoints;
		k = (i+w->numpoints-1)%w->numpoints;
		VectorSubtract (w->p[j], w->p[i], v1);
		VectorSubtract (w->p[i], w->p[k], v2);
		VectorNormalize(v1);
		VectorNormalize(v2);
		if (DotProduct(v1, v2) < 1.0-ON_EPSILON)
		{
			VectorCopy (w->p[i], p[nump]);
			nump++;
		}
	}

	if (nump == w->numpoints)
		return;

	c_removed += w->numpoints - nump;
	w->numpoints = nump;
	memcpy (w->p, p, nump*sizeof(p[0]));
}

/*
============
WindingPlane
============
*/
void WindingPlane (winding_t *w, vec3_t normal, vec_t *dist)
{
	vec3_t	v1, v2;

	VectorSubtract (w->p[1], w->p[0], v1);
	VectorSubtract (w->p[2], w->p[0], v2);
	CrossProduct (v2, v1, normal);
	VectorNormalize (normal);
	*dist = DotProduct (w->p[0], normal);

}

/*
=============
WindingArea
=============
*/
vec_t	WindingArea (winding_t *w)
{
	int		i;
	vec3_t	d1, d2, cross;
	vec_t	total;

	total = 0;
	for (i=2 ; i<w->numpoints ; i++)
	{
		VectorSubtract (w->p[i-1], w->p[0], d1);
		VectorSubtract (w->p[i], w->p[0], d2);
		CrossProduct (d1, d2, cross);
		total += 0.5 * VectorLength ( cross );
	}
	return total;
}

void	WindingBounds (winding_t *w, vec3_t mins, vec3_t maxs)
{
	vec_t	v;
	int		i,j;

	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;

	for (i=0 ; i<w->numpoints ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			v = w->p[i][j];
			if (v < mins[j])
				mins[j] = v;
			if (v > maxs[j])
				maxs[j] = v;
		}
	}
}

/*
=============
WindingCenter
=============
*/
void	WindingCenter (winding_t *w, vec3_t center)
{
	int		i;
	vec3_t	d1, d2, cross;
	float	scale;

	VectorCopy (vec3_origin, center);
	for (i=0 ; i<w->numpoints ; i++)
		VectorAdd (w->p[i], center, center);

	scale = 1.0/w->numpoints;
	VectorScale (center, scale, center);
}

/*
=================
BaseWindingForPlane
=================
*/
winding_t *BaseWindingForPlane (vec3_t normal, float dist)
{
	int		i, x;
	vec_t	max, v;
	vec3_t	org, vright, vup;
	winding_t	*w;
	
// find the major axis

	max = -BOGUS_RANGE;
	x = -1;
	for (i=0 ; i<3; i++)
	{
		v = fabs(normal[i]);
		if (v > max)
		{
			x = i;
			max = v;
		}
	}
	if (x==-1)
		Error ("BaseWindingForPlane: no axis found");
		
	VectorCopy (vec3_origin, vup);	
	switch (x)
	{
	case 0:
	case 1:
		vup[2] = 1;
		break;		
	case 2:
		vup[0] = 1;
		break;		
	}

	v = DotProduct (vup, normal);
	VectorMA (vup, -v, normal, vup);
	VectorNormalize (vup);
		
	VectorScale (normal, dist, org);
	
	CrossProduct (vup, normal, vright);
	
	VectorScale (vup, 9000, vup);
	VectorScale (vright, 9000, vright);

// project a really big	axis aligned box onto the plane
	w = AllocWinding (4);
	
	VectorSubtract (org, vright, w->p[0]);
	VectorAdd (w->p[0], vup, w->p[0]);
	
	VectorAdd (org, vright, w->p[1]);
	VectorAdd (w->p[1], vup, w->p[1]);
	
	VectorAdd (org, vright, w->p[2]);
	VectorSubtract (w->p[2], vup, w->p[2]);
	
	VectorSubtract (org, vright, w->p[3]);
	VectorSubtract (w->p[3], vup, w->p[3]);

	w->numpoints = 4;
	
	return w;	
}

/*
==================
CopyWinding
==================
*/
winding_t	*CopyWinding (winding_t *w)
{
	int			size;
	winding_t	*c;
	
	size = (int)((winding_t *)0)->p[w->numpoints];
	c = malloc (size);
	memcpy (c, w, size);
	return c;
}


/*
=============
ClipWinding
=============
*/
void	ClipWinding (winding_t *in, vec3_t normal, vec_t dist,
					 winding_t **front, winding_t **back)
{
	vec_t	dists[MAX_POINTS_ON_WINDING+4];
	int		sides[MAX_POINTS_ON_WINDING+4];
	int		counts[3];
	vec_t	dot;
	int		i, j;
	vec_t	*p1, *p2;
	vec3_t	mid;
	winding_t	*f, *b;
	int		maxpts;
	
	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	for (i=0 ; i<in->numpoints ; i++)
	{
		dot = DotProduct (in->p[i], normal);
		dot -= dist;
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
	sides[i] = sides[0];
	dists[i] = dists[0];
	
	*front = *back = NULL;

	if (!counts[0])
	{
		*back = CopyWinding (in);
		return;
	}
	if (!counts[1])
	{
		*front = CopyWinding (in);
		return;
	}

	maxpts = in->numpoints+4;	// can't use counts[0]+2 because
								// of fp grouping errors

	*front = f = AllocWinding (maxpts);
	*back = b = AllocWinding (maxpts);
		
	for (i=0 ; i<in->numpoints ; i++)
	{
		p1 = in->p[i];
		
		if (sides[i] == SIDE_ON)
		{
			VectorCopy (p1, f->p[f->numpoints]);
			f->numpoints++;
			VectorCopy (p1, b->p[b->numpoints]);
			b->numpoints++;
			continue;
		}
	
		if (sides[i] == SIDE_FRONT)
		{
			VectorCopy (p1, f->p[f->numpoints]);
			f->numpoints++;
		}
		if (sides[i] == SIDE_BACK)
		{
			VectorCopy (p1, b->p[b->numpoints]);
			b->numpoints++;
		}

		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;
			
	// generate a split point
		p2 = in->p[(i+1)%in->numpoints];
		
		dot = dists[i] / (dists[i]-dists[i+1]);
		for (j=0 ; j<3 ; j++)
		{	// avoid round off error when possible
			if (normal[j] == 1)
				mid[j] = dist;
			else if (normal[j] == -1)
				mid[j] = -dist;
			else
				mid[j] = p1[j] + dot*(p2[j]-p1[j]);
		}
			
		VectorCopy (mid, f->p[f->numpoints]);
		f->numpoints++;
		VectorCopy (mid, b->p[b->numpoints]);
		b->numpoints++;
	}
	
	if (f->numpoints > maxpts || b->numpoints > maxpts)
		Error ("ClipWinding: points exceeded estimate");
	if (f->numpoints > MAX_POINTS_ON_WINDING || b->numpoints > MAX_POINTS_ON_WINDING)
		Error ("ClipWinding: MAX_POINTS_ON_WINDING");
}



/*
=============
ClipWindingNoCopy
=============
*/
void	ClipWindingNoCopy (winding_t *in, vec3_t normal, vec_t dist,
					 winding_t **front, winding_t **back)
{
	vec_t	dists[MAX_POINTS_ON_WINDING+4];
	int		sides[MAX_POINTS_ON_WINDING+4];
	int		counts[3];
	vec_t	dot;
	int		i, j;
	vec_t	*p1, *p2;
	vec3_t	mid;
	winding_t	*f, *b;
	int		maxpts;
	
	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	for (i=0 ; i<in->numpoints ; i++)
	{
		dot = DotProduct (in->p[i], normal);
		dot -= dist;
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
	sides[i] = sides[0];
	dists[i] = dists[0];
	
	*front = *back = NULL;

	if (!counts[0])
	{
		*back = in;
		return;
	}
	if (!counts[1])
	{
		*front = in;
		return;
	}

	maxpts = in->numpoints+4;	// can't use counts[0]+2 because
								// of fp grouping errors

	*front = f = AllocWinding (maxpts);
	*back = b = AllocWinding (maxpts);
		
	for (i=0 ; i<in->numpoints ; i++)
	{
		p1 = in->p[i];
		
		if (sides[i] == SIDE_ON)
		{
			VectorCopy (p1, f->p[f->numpoints]);
			f->numpoints++;
			VectorCopy (p1, b->p[b->numpoints]);
			b->numpoints++;
			continue;
		}
	
		if (sides[i] == SIDE_FRONT)
		{
			VectorCopy (p1, f->p[f->numpoints]);
			f->numpoints++;
		}
		if (sides[i] == SIDE_BACK)
		{
			VectorCopy (p1, b->p[b->numpoints]);
			b->numpoints++;
		}

		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;
			
	// generate a split point
		p2 = in->p[(i+1)%in->numpoints];
		
		dot = dists[i] / (dists[i]-dists[i+1]);
		for (j=0 ; j<3 ; j++)
		{	// avoid round off error when possible
			if (normal[j] == 1)
				mid[j] = dist;
			else if (normal[j] == -1)
				mid[j] = -dist;
			else
				mid[j] = p1[j] + dot*(p2[j]-p1[j]);
		}
			
		VectorCopy (mid, f->p[f->numpoints]);
		f->numpoints++;
		VectorCopy (mid, b->p[b->numpoints]);
		b->numpoints++;
	}
	
	if (f->numpoints > maxpts || b->numpoints > maxpts)
		Error ("ClipWinding: points exceeded estimate");
	if (f->numpoints > MAX_POINTS_ON_WINDING || b->numpoints > MAX_POINTS_ON_WINDING)
		Error ("ClipWinding: MAX_POINTS_ON_WINDING");
}


/*
=============
ChopWindingNoFree
=============
*/
winding_t	*ChopWindingNoFree (winding_t *in, vec3_t normal, vec_t dist)
{
	vec_t	dists[MAX_POINTS_ON_WINDING+4];
	int		sides[MAX_POINTS_ON_WINDING+4];
	int		counts[3];
	vec_t	dot;
	int		i, j;
	vec_t	*p1, *p2;
	vec3_t	mid;
	winding_t	*f;
	int		maxpts;

	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	for (i=0 ; i<in->numpoints ; i++)
	{
		dot = DotProduct (in->p[i], normal);
		dot -= dist;
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
	sides[i] = sides[0];
	dists[i] = dists[0];
	
	if (!counts[0])
		return NULL;
	if (!counts[1])
		return in;

	maxpts = in->numpoints+4;	// can't use counts[0]+2 because
								// of fp grouping errors

	f = AllocWinding (maxpts);
		
	for (i=0 ; i<in->numpoints ; i++)
	{
		p1 = in->p[i];
		
		if (sides[i] == SIDE_ON)
		{
			VectorCopy (p1, f->p[f->numpoints]);
			f->numpoints++;
			continue;
		}
	
		if (sides[i] == SIDE_FRONT)
		{
			VectorCopy (p1, f->p[f->numpoints]);
			f->numpoints++;
		}

		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;
			
	// generate a split point
		p2 = in->p[(i+1)%in->numpoints];
		
		dot = dists[i] / (dists[i]-dists[i+1]);
		for (j=0 ; j<3 ; j++)
		{	// avoid round off error when possible
			if (normal[j] == 1)
				mid[j] = dist;
			else if (normal[j] == -1)
				mid[j] = -dist;
			else
				mid[j] = p1[j] + dot*(p2[j]-p1[j]);
		}
			
		VectorCopy (mid, f->p[f->numpoints]);
		f->numpoints++;
	}
	
	if (f->numpoints > maxpts)
		Error ("ClipWinding: points exceeded estimate");
	if (f->numpoints > MAX_POINTS_ON_WINDING)
		Error ("ClipWinding: MAX_POINTS_ON_WINDING");

	return f;
}


/*
=================
ChopWinding

Returns the fragment of in that is on the front side
of the cliping plane.  The original is freed.
=================
*/
winding_t	*ChopWinding (winding_t *in, vec3_t normal, vec_t dist)
{
	winding_t	*f, *b;

	ClipWinding (in, normal, dist, &f, &b);
	FreeWinding (in);
	if (b)
		FreeWinding (b);
	return f;
}


/*
=================
CheckWinding

=================
*/
void CheckWinding (winding_t *w)
{
	int		i, j;
	vec_t	*p1, *p2;
	vec_t	d, edgedist;
	vec3_t	dir, edgenormal, facenormal;
	vec_t	area;
	vec_t	facedist;

	if (w->numpoints < 3)
		Error ("CheckWinding: %i points",w->numpoints);
	
	area = WindingArea(w);
	if (area < 1)
		Error ("CheckWinding: %f area", area);

	WindingPlane (w, facenormal, &facedist);
	
	for (i=0 ; i<w->numpoints ; i++)
	{
		p1 = w->p[i];

		for (j=0 ; j<3 ; j++)
			if (p1[j] > BOGUS_RANGE || p1[j] < -BOGUS_RANGE)
				Error ("CheckFace: BUGUS_RANGE: %f",p1[j]);

		j = i+1 == w->numpoints ? 0 : i+1;
		
	// check the point is on the face plane
		d = DotProduct (p1, facenormal) - facedist;
		if (d < -ON_EPSILON || d > ON_EPSILON)
			Error ("CheckWinding: point off plane");
	
	// check the edge isn't degenerate
		p2 = w->p[j];
		VectorSubtract (p2, p1, dir);
		
		if (VectorLength (dir) < ON_EPSILON)
			Error ("CheckWinding: degenerate edge");
			
		CrossProduct (facenormal, dir, edgenormal);
		VectorNormalize (edgenormal);
		edgedist = DotProduct (p1, edgenormal);
		edgedist += ON_EPSILON;
		
	// all other points must be on front side
		for (j=0 ; j<w->numpoints ; j++)
		{
			if (j == i)
				continue;
			d = DotProduct (w->p[j], edgenormal);
			if (d > edgedist)
				Error ("CheckWinding: non-convex");
		}
	}
}


/*
============
WindingOnPlaneSide
============
*/
int		WindingOnPlaneSide (winding_t *w, vec3_t normal, vec_t dist)
{
	qboolean	front, back;
	int			i;
	vec_t		d;

	front = false;
	back = false;
	for (i=0 ; i<w->numpoints ; i++)
	{
		d = DotProduct (w->p[i], normal) - dist;
		if (d < -ON_EPSILON)
		{
			if (front)
				return SIDE_CROSS;
			back = true;
			continue;
		}
		if (d > ON_EPSILON)
		{
			if (back)
				return SIDE_CROSS;
			front = true;
			continue;
		}
	}

	if (back)
		return SIDE_BACK;
	if (front)
		return SIDE_FRONT;
	return SIDE_ON;
}

