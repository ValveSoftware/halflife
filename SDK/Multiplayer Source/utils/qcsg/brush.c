/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

// brush.c

#include "csg.h"

plane_t		mapplanes[MAX_MAP_PLANES];
int			nummapplanes;

/*
=============================================================================

PLANE FINDING

=============================================================================
*/

void FindGCD (int *v)
{
	int		i, j, smallest;
	int		rem[3];
	int		val[3];

	for (i=0 ; i<3 ; i++)
		val[i] = abs(v[i]);

	while (1)
	{
		smallest = 1<<30;
		for (i=0 ; i<3 ; i++)
		{
			j = abs(val[i]);
			if (j && j<smallest)
				smallest = j;
		}

		for (i=0 ; i<3 ; i++)
			rem[i] = val[i]%smallest;
		if (rem[0] + rem[1] + rem[2] == 0)
			break;		// smallest == gcd
		for (i=0 ; i<3 ; i++)
			if (!rem[i])
				val[i] = smallest;
			else
				val[i] = rem[i];
	}

	for (i=0 ; i<3 ; i++)
		v[i] /= smallest;
}

int	PlaneTypeForNormal (vec3_t normal)
{
	vec_t	ax, ay, az;
	
// NOTE: should these have an epsilon around 1.0?		
	if (normal[0] == 1.0 || normal[0] == -1.0)
		return PLANE_X;
	if (normal[1] == 1.0 || normal[1] == -1.0)
		return PLANE_Y;
	if (normal[2] == 1.0 || normal[2] == -1.0)
		return PLANE_Z;
		
	ax = fabs(normal[0]);
	ay = fabs(normal[1]);
	az = fabs(normal[2]);
	
	if (ax >= ay && ax >= az)
		return PLANE_ANYX;
	if (ay >= ax && ay >= az)
		return PLANE_ANYY;
	return PLANE_ANYZ;
}

/*
=============
FindIntPlane

Returns which plane number to use for a given integer defined plane.

=============
*/
int		FindIntPlane (int *inormal, int *iorigin)
{
	int		i, j;
	plane_t	*p, temp;
	int		t;
	vec3_t	origin;
	qboolean	locked;

	FindGCD (inormal);

	p = mapplanes;
	locked = false;
	i = 0;

	while (1)
	{
		if (i == nummapplanes)
		{
			if (!locked)
			{
				locked = true;
				ThreadLock ();	// make sure we don't race
			}
			if (i == nummapplanes)
				break;		// we didn't race
		}

		// see if origin is on plane
		t = 0;
		for (j=0 ; j<3 ; j++)
			t += (iorigin[j] - p->iorigin[j]) * inormal[j];
		if (!t)
		{	// on plane

			// see if the normal is forward, backwards, or off
			for (j=0 ; j<3 ; j++)
				if (inormal[j] != p->inormal[j])
					break;
			if (j == 3)
			{
				if (locked)
					ThreadUnlock ();
				return i;
			}
		}

		i++;
		p++;
	}

	if (!locked)
		Error ("not locked");

	// create a new plane
	for (j=0 ; j<3 ; j++)
	{
		p->inormal[j] = inormal[j];
		(p+1)->inormal[j] = -inormal[j];
		p->iorigin[j] = iorigin[j];
		(p+1)->iorigin[j] = iorigin[j];

		p->normal[j] = inormal[j];
		origin[j] = iorigin[j];
	}

	if (nummapplanes >= MAX_MAP_PLANES)
		Error ("MAX_MAP_PLANES");

	VectorNormalize (p->normal);

	p->type = (p+1)->type = PlaneTypeForNormal (p->normal);

	p->dist = DotProduct (origin, p->normal); 
	VectorSubtract (vec3_origin, p->normal, (p+1)->normal);
	(p+1)->dist = -p->dist;

	// allways put axial planes facing positive first
	if (p->type < 3)
	{
		if (inormal[0] < 0 || inormal[1] < 0 || inormal[2] < 0)
		{
			// flip order
			temp = *p;
			*p = *(p+1);
			*(p+1) = temp;
			nummapplanes += 2;
			ThreadUnlock ();
			return i + 1;
		}
	}

	nummapplanes += 2;
	ThreadUnlock ();
	return i;
}

int PlaneFromPoints (int *p0, int *p1, int *p2)
{
	int		j;
	int		t1[3], t2[3];
	int		normal[3];

	// convert to a vector / dist plane
	for (j=0 ; j<3 ; j++)
	{
		t1[j] = p0[j] - p1[j];
		t2[j] = p2[j] - p1[j];
	}

	FindGCD (t1);
	FindGCD (t2);

	normal[0] = t1[1]*t2[2] - t1[2]*t2[1];
	normal[1] = t1[2]*t2[0] - t1[0]*t2[2];
	normal[2] = t1[0]*t2[1] - t1[1]*t2[0];

	if (!normal[0] && !normal[1] && !normal[2])
		return -1;

	return FindIntPlane (normal, p0);
}

/*
=============================================================================

			TURN BRUSHES INTO GROUPS OF FACES

=============================================================================
*/


void ScaleUpIVector (int *iv, int min)
{
	int		i;
	int		largest, scale;

	largest = 0;
	for (i=0 ; i<3 ; i++)
	{
		if (abs(iv[i]) > largest)
			largest = abs(iv[i]);
	}

	scale = (min + largest - 1)/largest;
	for (i=0 ; i<3 ; i++)
		iv[i] *= scale;
}

/*
=================
BaseWindingForIPlane
=================
*/
winding_t *BaseWindingForIPlane (plane_t *p)
{
	int		i, x;
	vec_t	max, v;
	winding_t	*w;
	int		org[3], vup[3], vright[3];

	VectorCopy (p->iorigin, org);

	VectorCopy (vec3_origin, vup);
	VectorCopy (vec3_origin, vright);
	if (!p->inormal[1] && !p->inormal[2])
	{
		vup[2] = 8192;
		vright[1] = 8192*p->normal[0];
	}
	else if (!p->inormal[0] && !p->inormal[2])
	{
		vup[2] = 8192;
		vright[0] = -8192*p->normal[1];
	}
	else if (!p->inormal[0] && !p->inormal[1])
	{
		vup[1] = 8192;
		vright[0] = 8192*p->normal[2];
	}
	else
	{
		vup[0] = -2*p->inormal[1]*p->inormal[2];
		vup[1] = p->inormal[0]*p->inormal[2];
		vup[2] = p->inormal[0]*p->inormal[1];

		FindGCD (vup);

		vright[0] = vup[1]*p->inormal[2] - vup[2]*p->inormal[1];
		vright[1] = vup[2]*p->inormal[0] - vup[0]*p->inormal[2];
		vright[2] = vup[0]*p->inormal[1] - vup[1]*p->inormal[0];

		FindGCD (vright);

		ScaleUpIVector (vup, 8192);
		ScaleUpIVector (vright, 8192);
	}

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
==============================================================================

BEVELED CLIPPING HULL GENERATION

This is done by brute force, and could easily get a lot faster if anyone cares.
==============================================================================
*/

#if 0
vec3_t	hull_size[NUM_HULLS][2] = {
{ {0, 0, 0}, {0, 0, 0} },
{ {-16,-16,-32}, {16,16,24} },
{ {-32,-32,-64}, {32,32,24} }
};
#endif
#if 1
vec3_t	hull_size[NUM_HULLS][2] = {
{ {0, 0, 0}, {0, 0, 0} },
{ {-16,-16,-36}, {16,16,36} },// 32x32x72
{ {-32,-32,-32}, {32,32,32} }, // 64x64x64
{ {-16,-16,-18}, {16,16,18} } // 32x32x36
};
#endif

#define	MAX_HULL_POINTS	32
#define	MAX_HULL_EDGES	64

typedef struct
{
	brush_t	*b;
	int		hullnum;
	int		num_hull_points;
	vec3_t	hull_points[MAX_HULL_POINTS];
	vec3_t	hull_corners[MAX_HULL_POINTS*8];
	int		num_hull_edges;
	int		hull_edges[MAX_HULL_EDGES][2];
} expand_t;

/*
=============
IPlaneEquiv

=============
*/
qboolean	IPlaneEquiv (plane_t *p1, plane_t *p2)
{
	int		t;
	int		j;

	// see if origin is on plane
	t = 0;
	for (j=0 ; j<3 ; j++)
		t += (p2->iorigin[j] - p1->iorigin[j]) * p2->inormal[j];
	if (t)
		return false;

	// see if the normal is forward, backwards, or off
	for (j=0 ; j<3 ; j++)
		if (p2->inormal[j] != p1->inormal[j])
			break;
	if (j == 3)
		return true;

	for (j=0 ; j<3 ; j++)
		if (p2->inormal[j] != -p1->inormal[j])
			break;
	if (j == 3)
		return true;

	return false;
}


/*
============
AddBrushPlane
=============
*/
void AddBrushPlane (expand_t *ex, plane_t *plane)
{
	int		i;
	plane_t	*pl;
	bface_t	*f, *nf;
	brushhull_t	*h;
	
	h = &ex->b->hulls[ex->hullnum];
// see if the plane has allready been added
	for (f=h->faces ; f ; f=f->next)
	{
		pl = f->plane;
		if (IPlaneEquiv (plane, pl))
			return;
	}

	nf = malloc(sizeof(*nf));
	memset (nf, 0, sizeof(*nf));
	nf->planenum = FindIntPlane (plane->inormal, plane->iorigin);
	nf->plane = &mapplanes[nf->planenum];
	nf->next = h->faces;
	nf->contents = CONTENTS_EMPTY;
	h->faces = nf;
	nf->texinfo = 0;	// all clip hulls have same texture
}


/*
============
TestAddPlane

Adds the given plane to the brush description if all of the original brush
vertexes can be put on the front side
=============
*/
void TestAddPlane (expand_t *ex, plane_t *plane)
{
	int		i, j, c, t;
	vec_t	d;
	vec_t	*corner;
	plane_t	flip;
	vec3_t	inv;
	int		counts[3];
	plane_t	*pl;
	bface_t	*f, *nf;
	brushhull_t	*h;

// see if the plane has allready been added
	h = &ex->b->hulls[ex->hullnum];
	for (f=h->faces ; f ; f=f->next)
	{
		pl = f->plane;
		if (IPlaneEquiv (plane, pl))
			return;
	}
	
// check all the corner points
	counts[0] = counts[1] = counts[2] = 0;
	c = ex->num_hull_points * 8;
	
	corner = ex->hull_corners[0];
	for (i=0 ; i<c ; i++, corner += 3)
	{
		t = 0;
		for (j=0 ; j<3 ; j++)
			t += (corner[j] - plane->iorigin[j]) * plane->inormal[j];
		if (t < 0)
		{
			if (counts[0])
				return;
			counts[1]++;
		}
		else if (t > 0)
		{
			if (counts[1])
				return;
			counts[0]++;
		}
		else
			counts[2]++;
	}
	
// the plane is a seperator

	if (counts[0])
	{
		VectorSubtract (vec3_origin, plane->inormal, flip.inormal);
		VectorCopy (plane->iorigin, flip.iorigin);
		plane = &flip;
	}

	
	nf = malloc(sizeof(*nf));
	memset (nf, 0, sizeof(*nf));
	nf->planenum = FindIntPlane (plane->inormal, plane->iorigin);
	nf->plane = &mapplanes[nf->planenum];
	nf->next = h->faces;
	nf->contents = CONTENTS_EMPTY;
	h->faces = nf;
	nf->texinfo = 0;	// all clip hulls have same texture
}

/*
============
AddHullPoint

Doesn't add if duplicated
=============
*/
int AddHullPoint (expand_t *ex, vec3_t p)
{
	int		i, j;
	vec_t	*c;
	int		x,y,z;
	vec3_t	r;
	
	for (i=0 ; i<3 ; i++)
		r[i] = floor (p[i]+0.5);

	for (i=0 ; i<ex->num_hull_points ; i++)
	{
		for (j=0 ; j<3 ; j++)
			if (r[j] != ex->hull_points[i][j])
				break;
		if (j == 3)
			return i;	// allready added
	}
	
	if (ex->num_hull_points == MAX_HULL_POINTS)
		Error ("MAX_HULL_POINTS");
	ex->num_hull_points++;

	VectorCopy (r, ex->hull_points[ex->num_hull_points]);
	
	c = ex->hull_corners[i*8];
	
	for (x=0 ; x<2 ; x++)
		for (y=0 ; y<2 ; y++)
			for (z=0; z<2 ; z++)
			{
				c[0] = r[0] + hull_size[ex->hullnum][x][0];
				c[1] = r[1] + hull_size[ex->hullnum][y][1];
				c[2] = r[2] + hull_size[ex->hullnum][z][2];
				c += 3;
			}
		
	return i;
}


/*
============
AddHullEdge

Creates all of the hull planes around the given edge, if not done allready
=============
*/
//#define	ANGLEEPSILON	0.00001

#define ANGLEEPSILON	ON_EPSILON

void AddHullEdge (expand_t *ex, vec3_t p1, vec3_t p2)
{
	int		pt1, pt2;
	int		i;
	int		a, b, c, d, e;
	vec3_t	edgevec, planeorg, planevec;
	plane_t	plane;
	vec_t	l;
	
	pt1 = AddHullPoint (ex, p1);
	pt2 = AddHullPoint (ex, p2);

	// now use the rounded values
	p1 = ex->hull_points[pt1];
	p2 = ex->hull_points[pt2];

	for (i=0 ; i<ex->num_hull_edges ; i++)
		if ( (ex->hull_edges[i][0] == pt1 && ex->hull_edges[i][1] == pt2)
		|| (ex->hull_edges[i][0] == pt2 && ex->hull_edges[i][1] == pt1) )
			return;	// allread added
		
	if (ex->num_hull_edges == MAX_HULL_EDGES)
		Error ("MAX_HULL_EDGES");

	ex->hull_edges[i][0] = pt1;
	ex->hull_edges[i][1] = pt2;
	ex->num_hull_edges++;
		
	VectorSubtract (p1, p2, edgevec);
	VectorNormalize (edgevec);
	
	for (a=0 ; a<3 ; a++)
	{
		b = (a+1)%3;
		c = (a+2)%3;
		for (d=0 ; d<=1 ; d++)
			for (e=0 ; e<=1 ; e++)
			{
				VectorCopy (p1, plane.iorigin);
				plane.iorigin[b] += hull_size[ex->hullnum][d][b];
				plane.iorigin[c] += hull_size[ex->hullnum][e][c];
				
				VectorCopy (vec3_origin, planevec);
				planevec[a] = 1;
				
				plane.inormal[0] = planevec[1]*edgevec[2] - planevec[2]*edgevec[1];
				plane.inormal[1] = planevec[2]*edgevec[0] - planevec[0]*edgevec[2];
				plane.inormal[2] = planevec[0]*edgevec[1] - planevec[1]*edgevec[0];

				if (!plane.inormal[0] && !plane.inormal[1] && !plane.inormal[2])
					continue;	// degenerate
				TestAddPlane (ex, &plane);
			}
	}
	

}		


/*
============
ExpandBrush
=============
*/
void ExpandBrush (brush_t *b, int hullnum)
{
	int		i, x, s;
	int		corner;
	bface_t	*brush_faces, *f, *nf;
	plane_t	*p, plane;
	int		iorigin[3], inormal[3];
	expand_t	ex;
	brushhull_t	*h;
	qboolean	axial;

	brush_faces = b->hulls[0].faces;
	h = &b->hulls[hullnum];

	ex.b = b;
	ex.hullnum = hullnum;
	ex.num_hull_points = 0;
	ex.num_hull_edges = 0;

// expand all of the planes
	axial = true;
	for (f=brush_faces ; f ; f=f->next)
	{
		p = f->plane;
		if (p->type > PLANE_Z)
			axial = false;	// not an xyz axial plane

		VectorCopy (p->iorigin, iorigin);
		VectorCopy (p->inormal, inormal);

		for (x=0 ; x<3 ; x++)
		{
			if (p->normal[x] > 0)
				corner = hull_size[hullnum][1][x];
			else if (p->normal[x] < 0)
				corner = - hull_size[hullnum][0][x];
			else
				corner = 0;
			iorigin[x] += p->normal[x]*corner;
		}
		nf = malloc(sizeof(*nf));
		memset (nf, 0, sizeof(*nf));

		nf->planenum = FindIntPlane (inormal, iorigin);
		nf->plane = &mapplanes[nf->planenum];
		nf->next = h->faces;
		nf->contents = CONTENTS_EMPTY;
		h->faces = nf;
		nf->texinfo = 0;	// all clip hulls have same texture
	}

	// if this was an axial brush, we are done
	if (axial)
		return;

#if 1
// add any axis planes not contained in the brush to bevel off corners
	for (x=0 ; x<3 ; x++)
		for (s=-1 ; s<=1 ; s+=2)
		{
		// add the plane
			VectorCopy (vec3_origin, plane.inormal);
			plane.inormal[x] = s;
			if (s == -1)
			{
				VectorAdd (b->hulls[0].mins, hull_size[hullnum][0], plane.iorigin);
			}
			else
			{
				VectorAdd (b->hulls[0].maxs, hull_size[hullnum][1], plane.iorigin);
			}
			AddBrushPlane (&ex, &plane);
		}
#endif

#if 0
// create all the hull points
	for (f=brush_faces ; f ; f=f->next)
		for (i=0 ; i<f->w->numpoints ; i++)
			AddHullPoint (&ex, f->w->p[i]);

// add all of the edge bevels
	for (f=brush_faces ; f ; f=f->next)
		for (i=0 ; i<f->w->numpoints ; i++)
			AddHullEdge (&ex, f->w->p[i], f->w->p[(i+1)%f->w->numpoints]);
#endif
}

//============================================================================

/*
===========
MakeHullFaces
===========
*/
void MakeHullFaces (brush_t *b, brushhull_t *h)
{
	bface_t	*f, *f2;
	winding_t	*w;
	plane_t		*p;
	int			i, j;
	vec_t		v;
	vec_t		area;

restart:
	h->mins[0] = h->mins[1] = h->mins[2] = 9999;
	h->maxs[0] = h->maxs[1] = h->maxs[2] = -9999;

	for (f = h->faces ; f ; f=f->next)
	{
//		w = BaseWindingForIPlane (f->plane);
		w = BaseWindingForPlane (f->plane->normal, f->plane->dist);
		for (f2 = h->faces ; f2 && w ; f2=f2->next)
		{
			if (f == f2)
				continue;
			p = &mapplanes[f2->planenum ^ 1];

			w = ChopWinding (w, p->normal, p->dist);
		}
		area = w ? WindingArea(w) : 0;
		if (area < 0.1)
		{
			qprintf ("Entity %i, Brush %i: plane with area %4.2f\n"
				, b->entitynum, b->brushnum, area);
			// remove the face and regenerate the hull
			if (h->faces == f)
				h->faces = f->next;
			else
			{
				for (f2=h->faces ; f2->next != f ; f2=f2->next)
				;
				f2->next = f->next;
			}
			goto restart;
		}
		f->w = w;
		f->contents = CONTENTS_EMPTY;
		if (w)
		{
			for (i=0 ; i<w->numpoints ; i++)
			{
				for (j=0 ; j<3 ; j++)
				{
					v = w->p[i][j];
//					w->p[i][j] = floor (v+0.5);	// round to int
					if (v<h->mins[j])
						h->mins[j] = v;
					if (v>h->maxs[j])
						h->maxs[j] = v;
				}
			}
		}
	}

	for (i=0 ; i<3 ; i++)
	{
		if (h->mins[i] < -BOGUS_RANGE/2
		|| h->maxs[i] > BOGUS_RANGE/2)
		{
			vec3_t eorigin = { 0, 0, 0};
			char *pszClass = "Unknown Class";
			if ( b->entitynum )
			{
				entity_t	*e = entities + b->entitynum;
				pszClass = ValueForKey(e, "classname" );
				GetVectorForKey( e, "origin", eorigin );
			}
			
			printf( "Entity %i, Brush %i: A '%s' @(%.0f,%.0f,%.0f)\n",
					b->entitynum, b->brushnum, pszClass, eorigin[0], eorigin[1], eorigin[2] );
			printf( "\toutside world(+/-%d): (%.0f, %.0f, %.0f)-(%.0f,%.0f,%.0f)\n",
					BOGUS_RANGE/2, h->mins[0], h->mins[1], h->mins[2], h->maxs[0], h->maxs[1], h->maxs[2] );
			break;
		}
	}
}

/*
===========
MakeBrushPlanes
===========
*/
qboolean MakeBrushPlanes (brush_t *b)
{
	int		i, j;
	int		planenum;
	side_t	*s;
	int		contents;
	bface_t	*f;
	vec3_t	origin;

	//
	// if the origin key is set (by an origin brush), offset all of the values
	//
	GetVectorForKey (&entities[b->entitynum], "origin", origin);

	//
	// convert to mapplanes
	//
	for (i=0 ; i<b->numsides ; i++)
	{
		s = &brushsides[b->firstside + i];
		for (j=0 ; j<3 ; j++)
		{
			VectorSubtract (s->planepts[j], origin, s->planepts[j]);
		}
		planenum = PlaneFromPoints (s->planepts[0], s->planepts[1], s->planepts[2]);
		if (planenum == -1)
		{
			printf ("Entity %i, Brush %i: plane with no normal\n"
				, b->entitynum, b->brushnum);
			continue;
		}

		//
		// see if the plane has been used already
		//
		for (f=b->hulls[0].faces ; f ; f=f->next)
		{
			if (f->planenum == planenum || f->planenum == (planenum^1) )
			{
				char *pszClass = "Unknown Class";
				if ( b->entitynum )
				{
					entity_t	*e = entities + b->entitynum;
					pszClass = ValueForKey(e, "classname" );
				}
				
				printf( "Entity %i, Brush %i: A '%s' @(%.0f,%.0f,%.0f) has a coplanar plane at (%.0f, %.0f, %.0f), texture %s\n",
						b->entitynum, b->brushnum, pszClass, origin[0], origin[1], origin[2], s->planepts[0][0]+origin[0], s->planepts[0][1]+origin[1], s->planepts[0][2]+origin[2], s->td.name );
				return false;
			}
		}

		f = malloc(sizeof(*f));
		memset (f, 0, sizeof(*f));

		f->planenum = planenum;
		f->plane = &mapplanes[planenum];
		f->next = b->hulls[0].faces;
		b->hulls[0].faces = f;
		f->texinfo = onlyents ? 0 : TexinfoForBrushTexture (f->plane, &s->td, origin);
	}

	return true;
}

/*
===========
TextureContents
===========
*/
int TextureContents (char *name)
{
	if (!Q_strncasecmp (name, "sky",3))
		return CONTENTS_SKY;

	if (!Q_strncasecmp(name+1,"!lava",5))
		return CONTENTS_LAVA;

	if (!Q_strncasecmp(name+1,"!slime",6))
		return CONTENTS_SLIME;

	if (!Q_strncasecmp (name, "!cur_90",7))
		return CONTENTS_CURRENT_90;
	if (!Q_strncasecmp (name, "!cur_0",6))
		return CONTENTS_CURRENT_0;
	if (!Q_strncasecmp (name, "!cur_270",8))
		return CONTENTS_CURRENT_270;
	if (!Q_strncasecmp (name, "!cur_180",8))
		return CONTENTS_CURRENT_180;
	if (!Q_strncasecmp (name, "!cur_up",7))
		return CONTENTS_CURRENT_UP;
	if (!Q_strncasecmp (name, "!cur_dwn",8))
		return CONTENTS_CURRENT_DOWN;

	if (name[0] == '!')
		return CONTENTS_WATER;

	if (!Q_strncasecmp (name, "origin",6))
		return CONTENTS_ORIGIN;

	if (!Q_strncasecmp (name, "clip",4))
		return CONTENTS_CLIP;

	if( !Q_strncasecmp( name, "translucent", 11 ) )
		return CONTENTS_TRANSLUCENT;

	if( name[0] == '@' )
		return CONTENTS_TRANSLUCENT;

	return CONTENTS_SOLID;
}

/*
===========
BrushContents
===========
*/
int	BrushContents (brush_t *b)
{
	char		*name;
	int			contents;
	bface_t		*f;
	side_t		*s;
	int			i;

	s = &brushsides[b->firstside];
	contents = TextureContents (s->td.name);
	for (i=1 ; i<b->numsides ; i++, s++)
	{
		if (TextureContents(s->td.name) != contents)
		{
			printf ("Entity %i, Brush %i: mixed face contents"
				, b->entitynum, b->brushnum);
			break;
		}
	}

	return contents;
}

/*
===========
CreateBrush
===========
*/
void CreateBrush (int brushnum)
{
	brush_t		*b;
	int			contents;
	int			h;

	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);

	b = &mapbrushes[brushnum];

	contents = b->contents;
	if (contents == CONTENTS_ORIGIN)
		return;

	//
	// convert brush sides to planes
	//
	MakeBrushPlanes (b);
	MakeHullFaces (b, &b->hulls[0]);

	// water brushes are not solid, so are not represented in
	// the clipping hull
	if (contents == CONTENTS_LAVA
	|| contents == CONTENTS_SLIME
	|| contents == CONTENTS_WATER 
	|| contents == CONTENTS_TRANSLUCENT )
		return;

	if (!noclip)
	{
		for (h=1 ; h<NUM_HULLS ; h++)
		{
			ExpandBrush (b, h);
			MakeHullFaces (b, &b->hulls[h]);
		}
	}

	// clip brushes don't stay in the drawing hull
	if (contents == CONTENTS_CLIP)
	{
		b->hulls[0].faces = NULL;
		b->contents = CONTENTS_SOLID;
	}
}



