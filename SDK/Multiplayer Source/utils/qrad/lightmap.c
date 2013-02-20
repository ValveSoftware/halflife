/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#include "qrad.h"

typedef struct
{
	dface_t		*faces[2];
	vec3_t		interface_normal;
	qboolean	coplanar;
} edgeshare_t;

edgeshare_t	edgeshare[MAX_MAP_EDGES];

vec3_t	face_centroids[MAX_MAP_EDGES];

#ifdef OBSOLETE_CODE

int			facelinks[MAX_MAP_FACES];
int			planelinks[2][MAX_MAP_PLANES];

/*
============
LinkPlaneFaces
============
*/
void LinkPlaneFaces (void)
{
	int		i;
	dface_t	*f;

	f = dfaces;
	for (i=0 ; i<numfaces ; i++, f++)
	{
		facelinks[i] = planelinks[f->side][f->planenum];
		planelinks[f->side][f->planenum] = i;
	}
}

#endif

/*
============
PairEdges
============
*/
void PairEdges (void)
{
	int		i, j, k, n;
	dface_t	*f;
	edgeshare_t	*e;

	f = dfaces;
	for (i=0 ; i<numfaces ; i++, f++)
	{
		for (j=0 ; j<f->numedges ; j++)
		{
			k = dsurfedges[f->firstedge + j];
			if (k < 0)
			{
				e = &edgeshare[-k];
				e->faces[1] = f;
			}
			else
			{
				e = &edgeshare[k];
				e->faces[0] = f;
			}

			if (e->faces[0] && e->faces[1])
			{
				// determine if coplanar
				if (e->faces[0]->planenum == e->faces[1]->planenum)
					e->coplanar = true;
				else if ( smoothing_threshold > 0 )
				{
					// see if they fall into a "smoothing group" based on angle of the normals
					vec3_t	normals[2];
					double	cos_normals_angle;
					for(n=0; n<2; n++)
					{
						VectorCopy( dplanes[e->faces[n]->planenum].normal, normals[n] );
						if ( e->faces[n]->side )
							VectorSubtract( vec3_origin, normals[n], normals[n] );
					}
					cos_normals_angle = DotProduct( normals[0], normals[1] );
					if ( cos_normals_angle >= smoothing_threshold ) 
					{
						VectorAdd( normals[0], normals[1], e->interface_normal );
						VectorNormalize( e->interface_normal );
					}
				}
			}
		}
	}
}

/*
=================================================================

  POINT TRIANGULATION

=================================================================
*/

typedef struct triedge_s
{
	int			p0, p1;
	vec3_t		normal;
	vec_t		dist;
	struct triangle_s	*tri;
} triedge_t;

typedef struct triangle_s
{
	triedge_t	*edges[3];
} triangle_t;

#define	MAX_TRI_POINTS		2048  // Was 1024 originally.
#define	MAX_TRI_EDGES		(MAX_TRI_POINTS*6)
#define	MAX_TRI_TRIS		(MAX_TRI_POINTS*2)

typedef struct
{
	int			numpoints;
	int			numedges;
	int			numtris;
	dplane_t	*plane;
	triedge_t	*edgematrix[MAX_TRI_POINTS][MAX_TRI_POINTS];
	patch_t		*points[MAX_TRI_POINTS];
	triedge_t	edges[MAX_TRI_EDGES];
	triangle_t	tris[MAX_TRI_TRIS];
} triangulation_t;

/*
===============
AllocTriangulation
===============
*/
triangulation_t	*AllocTriangulation (dplane_t *plane)
{
	triangulation_t	*t = NULL;


    HANDLE h;
	if ( h = GlobalAlloc( GMEM_FIXED | GMEM_ZEROINIT, sizeof(triangulation_t) ) )
	{
		t = GlobalLock( h );

		t->numpoints = 0;
		t->numedges = 0;
		t->numtris = 0;

		t->plane = plane;
	}
	else
		Error("Cannot alloc triangulation memory!");

	return t;
}

/*
===============
FreeTriangulation
===============
*/
void FreeTriangulation (triangulation_t *tr)
{
    HANDLE h = GlobalHandle(tr);
    
	if ( h )
	{
		GlobalUnlock(h);
		GlobalFree(h);
	}
	else
		Error("Cannot free triangulation memory!");
}


triedge_t	*FindEdge (triangulation_t *trian, int p0, int p1)
{
	triedge_t	*e, *be;
	vec3_t		v1;
	vec3_t		normal;
	vec_t		dist;

	if (trian->edgematrix[p0][p1])
		return trian->edgematrix[p0][p1];

	if (trian->numedges > MAX_TRI_EDGES-2)
		Error ("trian->numedges > MAX_TRI_EDGES-2");

	VectorSubtract (trian->points[p1]->origin, trian->points[p0]->origin, v1);
	VectorNormalize (v1);
	CrossProduct (v1, trian->plane->normal, normal);
	dist = DotProduct (trian->points[p0]->origin, normal);

	e = &trian->edges[trian->numedges];
	e->p0 = p0;
	e->p1 = p1;
	e->tri = NULL;
	VectorCopy (normal, e->normal);
	e->dist = dist;
	trian->numedges++;
	trian->edgematrix[p0][p1] = e;

	be = &trian->edges[trian->numedges];
	be->p0 = p1;
	be->p1 = p0;
	be->tri = NULL;
	VectorSubtract (vec3_origin, normal, be->normal);
	be->dist = -dist;
	trian->numedges++;
	trian->edgematrix[p1][p0] = be;

	return e;
}

triangle_t	*AllocTriangle (triangulation_t *trian)
{
	triangle_t	*t;

	if (trian->numtris >= MAX_TRI_TRIS)
		Error ("trian->numtris >= MAX_TRI_TRIS");

	t = &trian->tris[trian->numtris];
	trian->numtris++;

	return t;
}

/*
============
TriEdge_r
============
*/
void TriEdge_r (triangulation_t *trian, triedge_t *e)
{
	int		i, bestp;
	vec3_t	v1, v2;
	vec_t	*p0, *p1, *p;
	vec_t	best, ang;
	triangle_t	*nt;

	if (e->tri)
		return;		// allready connected by someone

	// find the point with the best angle
	p0 = trian->points[e->p0]->origin;
	p1 = trian->points[e->p1]->origin;
	best = 1.1f;
	for (i=0 ; i< trian->numpoints ; i++)
	{
		p = trian->points[i]->origin;
		// a 0 dist will form a degenerate triangle
		if (DotProduct(p, e->normal) - e->dist < 0)
			continue;	// behind edge
		VectorSubtract (p0, p, v1);
		VectorSubtract (p1, p, v2);
		if (!VectorNormalize (v1))
			continue;
		if (!VectorNormalize (v2))
			continue;
		ang = DotProduct (v1, v2);
		if (ang < best)
		{
			best = ang;
			bestp = i;
		}
	}
	if (best >= 1)
		return;		// edge doesn't match anything

	// make a new triangle
	nt = AllocTriangle (trian);
	nt->edges[0] = e;
	nt->edges[1] = FindEdge (trian, e->p1, bestp);
	nt->edges[2] = FindEdge (trian, bestp, e->p0);
	for (i=0 ; i<3 ; i++)
		nt->edges[i]->tri = nt;
	TriEdge_r (trian, FindEdge (trian, bestp, e->p1));
	TriEdge_r (trian, FindEdge (trian, e->p0, bestp));
}

/*
============
TriangulatePoints
============
*/
void TriangulatePoints (triangulation_t *trian)
{
	vec_t	d, bestd;
	vec3_t	v1;
	int		bp1, bp2, i, j;
	vec_t	*p1, *p2;
	triedge_t	*e, *e2;

	if (trian->numpoints < 2)
		return;

	// find the two closest points
	bestd = 9999;
	for (i=0 ; i<trian->numpoints ; i++)
	{
		p1 = trian->points[i]->origin;
		for (j=i+1 ; j<trian->numpoints ; j++)
		{
			p2 = trian->points[j]->origin;
			VectorSubtract (p2, p1, v1);
			d = (float)VectorLength (v1);
			if (d < bestd)
			{
				bestd = d;
				bp1 = i;
				bp2 = j;
			}
		}
	}

	e = FindEdge (trian, bp1, bp2);
	e2 = FindEdge (trian, bp2, bp1);
	TriEdge_r (trian, e);
	TriEdge_r (trian, e2);
}

/*
===============
AddPatchToTriangulation
===============
*/
void AddPatchToTriangulation (patch_t *patch, triangulation_t *trian)
{
	int			pnum;

	pnum = trian->numpoints;
	if (pnum == MAX_TRI_POINTS)
		Error ("trian->numpoints == MAX_TRI_POINTS");
	trian->points[pnum] = patch;
	trian->numpoints++;
}

/*
===============
LerpTriangle
===============
*/
void LerpTriangle (triangulation_t *trian, triangle_t *t, vec3_t point, vec3_t result)
{
	patch_t		*p1, *p2, *p3;
	vec3_t		base, d1, d2;
	vec_t		x, y, x1, y1, x2, y2;
	int			i;

	p1 = trian->points[t->edges[0]->p0];
	p2 = trian->points[t->edges[1]->p0];
	p3 = trian->points[t->edges[2]->p0];

	VectorCopy( p1->totallight, base );

	VectorSubtract( p2->totallight, base, d1 );
	VectorSubtract( p3->totallight, base, d2 );

	x = DotProduct (point, t->edges[0]->normal) - t->edges[0]->dist;
	y = DotProduct (point, t->edges[2]->normal) - t->edges[2]->dist;

	x1 = 0;
	y1 = DotProduct (p2->origin, t->edges[2]->normal) - t->edges[2]->dist;

	x2 = DotProduct (p3->origin, t->edges[0]->normal) - t->edges[0]->dist;
	y2 = 0;

#ifdef BROKEN_CODE
	if (fabs(y1)<ON_EPSILON || fabs(x2)<ON_EPSILON)
	{
		VectorCopy( base, result );
	}
	else
	{
		for( i=0; i<3; i++ )
			result[i] = base[i] + x*d2[i]/x2 + y*d1[i]/y1;
	}
#else
	VectorCopy( base, result );
	if ( fabs(x2) >= ON_EPSILON )
		for( i=0; i<3; i++)
			result[i] += x*d2[i]/x2;
	if ( fabs(y1) >= ON_EPSILON )
		for( i=0; i<3; i++)
			result[i] += y*d1[i]/y1;
#endif

}

qboolean PointInTriangle (vec3_t point, triangle_t *t)
{
	int		i;
	triedge_t	*e;
	vec_t	d;

	for (i=0 ; i<3 ; i++)
	{
		e = t->edges[i];
		d = DotProduct (e->normal, point) - e->dist;
		if (d < 0)
			return false;	// not inside
	}

	return true;
}

/*
===============
SampleTriangulation
===============
*/
void SampleTriangulation (vec3_t point, triangulation_t *trian, triangle_t **last_tri, vec3_t result)
{
	triangle_t	*t;
	triedge_t	*e;
	vec_t		d, best;
	patch_t		*p0, *p1;
	vec3_t		v1, v2;
	int			i, j;

	if (trian->numpoints == 0)
	{
		VectorFill( result, 0 );
		return;
	}

	if (trian->numpoints == 1)
	{
		VectorCopy( trian->points[0]->totallight, result );
		return;
	}

	// try the last one that worked first
	if (*last_tri)
	{
		if (PointInTriangle (point, *last_tri))
		{
			LerpTriangle (trian, *last_tri, point, result);
			return;
		}
	}

	// search for triangles
	for (t = trian->tris, j=0 ; j < trian->numtris ; t++, j++)
	{
		if (t == *last_tri)
			continue;
		
		if (!PointInTriangle (point, t))
			continue;

		// this is it
		*last_tri = t;
		LerpTriangle (trian, t, point, result);
		return;
	}

	// search for exterior edge
	for (e=trian->edges, j=0 ; j< trian->numedges ; e++, j++)
	{
		if (e->tri)
			continue;		// not an exterior edge

		d = DotProduct (point, e->normal) - e->dist;
		if (d < 0)
			continue;	// not in front of edge

		p0 = trian->points[e->p0];
		p1 = trian->points[e->p1];
	
		VectorSubtract (p1->origin, p0->origin, v1);
		VectorNormalize (v1);
		VectorSubtract (point, p0->origin, v2);
		d = DotProduct (v2, v1);
		if (d < 0)
			continue;
		if (d > 1)
			continue;

		for( i=0; i<3; i++ )
			result[i] = p0->totallight[i] + d * (p1->totallight[i] - p0->totallight[i]);

		return;
	}

	// search for nearest point
	best = 99999;
	p1 = NULL;
	for (j=0 ; j<trian->numpoints ; j++)
	{
		p0 = trian->points[j];
		VectorSubtract (point, p0->origin, v1);
		d = (float)VectorLength (v1);
		if (d < best)
		{
			best = d;
			p1 = p0;
		}
	}

	if (!p1)
		Error ("SampleTriangulation: no points");

	VectorCopy( p1->totallight, result );
}

/*
=================================================================

  LIGHTMAP SAMPLE GENERATION

=================================================================
*/


#define	SINGLEMAP	(18*18*4)

typedef struct
{
	vec3_t	lightmaps[MAXLIGHTMAPS][SINGLEMAP];
	int		numlightstyles;
	vec_t	*light;
	vec_t	facedist;
	vec3_t	facenormal;

	int		numsurfpt;
	vec3_t	surfpt[SINGLEMAP];
	vec3_t	facemid;		// world coordinates of center

	vec3_t	texorg;
	vec3_t	worldtotex[2];	// s = (world - texorg) . worldtotex[0]
	vec3_t	textoworld[2];	// world = texorg + s * textoworld[0]

	vec_t	exactmins[2], exactmaxs[2];
	
	int		texmins[2], texsize[2];
	int		lightstyles[256];
	int		surfnum;
	dface_t	*face;
} lightinfo_t;


/*
================
CalcFaceExtents

Fills in s->texmins[] and s->texsize[]
also sets exactmins[] and exactmaxs[]
================
*/
void CalcFaceExtents (lightinfo_t *l)
{
	dface_t *s;
	vec_t	mins[2], maxs[2], val;
	int		i,j, e;
	dvertex_t	*v;
	texinfo_t	*tex;
	
	s = l->face;

	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -99999;

	tex = &texinfo[s->texinfo];
	
	for (i=0 ; i<s->numedges ; i++)
	{
		e = dsurfedges[s->firstedge+i];
		if (e >= 0)
			v = dvertexes + dedges[e].v[0];
		else
			v = dvertexes + dedges[-e].v[1];
		
		for (j=0 ; j<2 ; j++)
		{
			val = v->point[0] * tex->vecs[j][0] + 
				v->point[1] * tex->vecs[j][1] +
				v->point[2] * tex->vecs[j][2] +
				tex->vecs[j][3];
			if (val < mins[j])
				mins[j] = val;
			if (val > maxs[j])
				maxs[j] = val;
		}
	}

	for (i=0 ; i<2 ; i++)
	{	
		l->exactmins[i] = mins[i];
		l->exactmaxs[i] = maxs[i];
		
		mins[i] = (float)floor(mins[i]/16);
		maxs[i] = (float)ceil(maxs[i]/16);

		l->texmins[i] = (int)mins[i];
		l->texsize[i] = (int)(maxs[i] - mins[i]);
		if (l->texsize[i] > 17)
			Error ("Bad surface extents");
	}
}

/*
================
CalcFaceVectors

Fills in texorg, worldtotex. and textoworld
================
*/
void CalcFaceVectors (lightinfo_t *l)
{
	texinfo_t	*tex;
	int			i, j;
	vec3_t	texnormal;
	vec_t	distscale;
	vec_t	dist, len;

	tex = &texinfo[l->face->texinfo];
	
// convert from float to double
	for (i=0 ; i<2 ; i++)
		for (j=0 ; j<3 ; j++)
			l->worldtotex[i][j] = tex->vecs[i][j];

// calculate a normal to the texture axis.  points can be moved along this
// without changing their S/T
	texnormal[0] = tex->vecs[1][1]*tex->vecs[0][2]
		- tex->vecs[1][2]*tex->vecs[0][1];
	texnormal[1] = tex->vecs[1][2]*tex->vecs[0][0]
		- tex->vecs[1][0]*tex->vecs[0][2];
	texnormal[2] = tex->vecs[1][0]*tex->vecs[0][1]
		- tex->vecs[1][1]*tex->vecs[0][0];
	VectorNormalize (texnormal);

// flip it towards plane normal
	distscale = DotProduct (texnormal, l->facenormal);
	if (!distscale)
		Error ("Texture axis perpendicular to face");
	if (distscale < 0)
	{
		distscale = -distscale;
		VectorSubtract (vec3_origin, texnormal, texnormal);
	}	

// distscale is the ratio of the distance along the texture normal to
// the distance along the plane normal
	distscale = 1/distscale;

	for (i=0 ; i<2 ; i++)
	{
		len = (float)VectorLength (l->worldtotex[i]);
		dist = DotProduct (l->worldtotex[i], l->facenormal);
		dist *= distscale;
		VectorMA (l->worldtotex[i], -dist, texnormal, l->textoworld[i]);
		VectorScale (l->textoworld[i], (1/len)*(1/len), l->textoworld[i]);
	}


// calculate texorg on the texture plane
	for (i=0 ; i<3 ; i++)
		l->texorg[i] = -tex->vecs[0][3]* l->textoworld[0][i] - tex->vecs[1][3] * l->textoworld[1][i];

// project back to the face plane
	dist = DotProduct (l->texorg, l->facenormal) - l->facedist - 1;
	dist *= distscale;
	VectorMA (l->texorg, -dist, texnormal, l->texorg);
	
}

/*
=================
CalcPoints

For each texture aligned grid point, back project onto the plane
to get the world xyz value of the sample point
=================
*/
void CalcPoints (lightinfo_t *l)
{
	int		i;
	int		s, t, j;
	int		w, h, step;
	vec_t	starts, startt, us, ut;
	vec_t	*surf;
	vec_t	mids, midt;
	vec3_t	origin;

	surf = l->surfpt[0];
	mids = (l->exactmaxs[0] + l->exactmins[0])/2;
	midt = (l->exactmaxs[1] + l->exactmins[1])/2;

	for (j=0 ; j<3 ; j++)
		l->facemid[j] = l->texorg[j] + l->textoworld[0][j]*mids + l->textoworld[1][j]*midt;

	h = l->texsize[1]+1;
	w = l->texsize[0]+1;
	starts = (float)l->texmins[0]*16;
	startt = (float)l->texmins[1]*16;
	step = 16;

	l->numsurfpt = w * h;

	// get the origin offset for rotating bmodels
	VectorCopy (face_offset[l->surfnum], origin);

	for (t=0 ; t<h ; t++)
	{
		for (s=0 ; s<w ; s++, surf+=3)
		{
			us = starts + s*step;
			ut = startt + t*step;


		// if a line can be traced from surf to facemid, the point is good
			for (i=0 ; i<64; i++)
			{
			// calculate texture point
				dleaf_t		*luxelleaf;

				for (j=0 ; j<3 ; j++)
					surf[j] = l->texorg[j] + l->textoworld[0][j]*us
					+ l->textoworld[1][j]*ut;
				VectorAdd (surf, origin, surf);

				luxelleaf = PointInLeaf(surf);

				// Make sure we are "in the world"(Not the zero leaf)
				if ( luxelleaf != dleafs )
				{
#if defined(BUGGY_TEST)
					if (TestLine_r (0, l->facemid, surf) == CONTENTS_EMPTY)
#endif
						break;	// got it
				}
				// nudge it
				if (i & 1)
				{
					if (us > mids)
					{
						us -= 8;
						if (us < mids)
							us = mids;
					}
					else
					{
						us += 8;
						if (us > mids)
							us = mids;
					}
				}
				else
				{
					if (ut > midt)
					{
						ut -= 8;
						if (ut < midt)
							ut = midt;
					}
					else
					{
						ut += 8;
						if (ut > midt)
							ut = midt;
					}
				}
			}
		}
	}
	
}


//==============================================================



typedef struct
{
	vec3_t		pos;
	vec3_t		light;
} sample_t;

typedef struct
{
	int			numsamples;
	sample_t	*samples[MAXLIGHTMAPS];
} facelight_t;

directlight_t	*directlights[MAX_MAP_LEAFS];
facelight_t		facelight[MAX_MAP_FACES];
int				numdlights;

/*
==================
FindTargetEntity
==================
*/
entity_t *FindTargetEntity (char *target)
{
	int		i;
	char	*n;

	for (i=0 ; i<num_entities ; i++)
	{
		n = ValueForKey (&entities[i], "targetname");
		if (!strcmp (n, target))
			return &entities[i];
	}

	return NULL;
}

/*
=============
CreateDirectLights
=============
*/
#define	DIRECT_SCALE	0.1f
void CreateDirectLights (void)
{
	unsigned i;
	patch_t	*p;
	directlight_t	*dl;
	dleaf_t	*leaf;
	int		leafnum;
	entity_t	*e, *e2;
	char	*name;
	char	*target;
	float	angle;
	vec3_t	dest;

	numdlights = 0;

	//
	// surfaces
	//
	for (i=0, p=patches ; i<num_patches ; i++, p++)
	{
		if( VectorAvg( p->totallight ) >= dlight_threshold )
			{
			numdlights++;
			dl = calloc(1, sizeof(directlight_t));

			VectorCopy (p->origin, dl->origin);

			leaf = PointInLeaf (dl->origin);
			leafnum = leaf - dleafs;

			dl->next = directlights[leafnum];
			directlights[leafnum] = dl;

			dl->type = emit_surface;
			VectorCopy (p->normal, dl->normal);
			VectorCopy( p->totallight, dl->intensity );
			VectorScale( dl->intensity, p->area, dl->intensity );
			VectorScale( dl->intensity, DIRECT_SCALE, dl->intensity );
			}

		VectorFill( p->totallight, 0 );		// all sent now  // BUGBUG for progressive refinement runs
	}

	//
	// entities
	//
	for (i=0 ; i<(unsigned)num_entities ; i++)
	{
		char *pLight;
		double r, g, b, scaler;
		float	l1;
		int argCnt;

		e = &entities[i];
		name = ValueForKey (e, "classname");
		if (strncmp (name, "light", 5))
			continue;

		numdlights++;
		dl = calloc(1, sizeof(directlight_t));

		GetVectorForKey (e, "origin", dl->origin);

		leaf = PointInLeaf (dl->origin);
		leafnum = leaf - dleafs;

		dl->next = directlights[leafnum];
		directlights[leafnum] = dl;

		dl->style = (int)FloatForKey (e, "style");

		pLight = ValueForKey( e, "_light" );
		// scanf into doubles, then assign, so it is vec_t size independent
		r = g = b = scaler = 0;
		argCnt = sscanf ( pLight, "%lf %lf %lf %lf", &r, &g, &b, &scaler );
		dl->intensity[0] = (float)r;
		if( argCnt == 1 )
		{
			// The R,G,B values are all equal.
			dl->intensity[1] = dl->intensity[2] = (float)r;
		}
		else if ( argCnt == 3 || argCnt == 4 )
		{
			// Save the other two G,B values.
			dl->intensity[1] = (float)g;
			dl->intensity[2] = (float)b;

			// Did we also get an "intensity" scaler value too?
			if ( argCnt == 4 )
			{
				// Scale the normalized 0-255 R,G,B values by the intensity scaler
				dl->intensity[0] = dl->intensity[0] / 255 * (float)scaler;
				dl->intensity[1] = dl->intensity[1] / 255 * (float)scaler;
				dl->intensity[2] = dl->intensity[2] / 255 * (float)scaler;
			}
		}
		else
		{
			printf( "entity at (%f,%f,%f) has bad '_light' value : '%s'\n", 
					dl->origin[0], dl->origin[1], dl->origin[2], pLight);
			continue;
		}

		target = ValueForKey (e, "target");

		if (!strcmp (name, "light_spot") || !strcmp(name, "light_environment") || target[0])
		{
			if (!VectorAvg( dl->intensity ))
				VectorFill( dl->intensity, 500 );
			dl->type = emit_spotlight;
			dl->stopdot = FloatForKey (e, "_cone");
			if (!dl->stopdot)
				dl->stopdot = 10;
			dl->stopdot2 = FloatForKey (e, "_cone2");
			if (!dl->stopdot2) 
				dl->stopdot2 = dl->stopdot;
			if (dl->stopdot2 < dl->stopdot)
				dl->stopdot2 = dl->stopdot;
			dl->stopdot2 = (float)cos(dl->stopdot2/180*Q_PI);
			dl->stopdot = (float)cos(dl->stopdot/180*Q_PI);

			if (target[0])
			{	// point towards target
				e2 = FindTargetEntity (target);
				if (!e2)
					printf ("WARNING: light at (%i %i %i) has missing target\n",
					(int)dl->origin[0], (int)dl->origin[1], (int)dl->origin[2]);
				else
				{
					GetVectorForKey (e2, "origin", dest);
					VectorSubtract (dest, dl->origin, dl->normal);
					VectorNormalize (dl->normal);
				}
			}
			else
			{	// point down angle
				vec3_t vAngles;
				GetVectorForKey( e, "angles", vAngles );

				angle = (float)FloatForKey (e, "angle");
				if (angle == ANGLE_UP)
				{
					dl->normal[0] = dl->normal[1] = 0;
					dl->normal[2] = 1;
				}
				else if (angle == ANGLE_DOWN)
				{
					dl->normal[0] = dl->normal[1] = 0;
					dl->normal[2] = -1;
				}
				else
				{
					// if we don't have a specific "angle" use the "angles" YAW
					if ( !angle )
					{
						angle = vAngles[1];
					}

					dl->normal[2] = 0;
					dl->normal[0] = (float)cos (angle/180*Q_PI);
					dl->normal[1] = (float)sin (angle/180*Q_PI);
				}

				angle = FloatForKey (e, "pitch");
				if ( !angle )
				{
					// if we don't have a specific "pitch" use the "angles" PITCH
					angle = vAngles[0];
				}

				dl->normal[2] = (float)sin(angle/180*Q_PI);
				dl->normal[0] *= (float)cos(angle/180*Q_PI);
				dl->normal[1] *= (float)cos(angle/180*Q_PI);
			}
			if (FloatForKey( e, "_sky" ) || !strcmp(name, "light_environment")) 
			{
				dl->type = emit_skylight;
				dl->stopdot2 = FloatForKey( e, "_sky" ); // hack stopdot2 to a sky key number
			}
		}
		else
		{
			if (!VectorAvg( dl->intensity ))
				VectorFill( dl->intensity, 300 );
			dl->type = emit_point;
		}

		if (dl->type != emit_skylight)
		{
			l1 = max( dl->intensity[0], max( dl->intensity[1], dl->intensity[2] ) );
			l1 = l1 * l1 / 10;

			dl->intensity[0] *= l1;
			dl->intensity[1] *= l1;
			dl->intensity[2] *= l1;
		}


	}

	qprintf ("%i direct lights\n", numdlights);
}

/*
=============
DeleteDirectLights
=============
*/

void DeleteDirectLights(void)
{
int					l;
directlight_t		*dl;

for ( l = 0; l < numleafs; l++ )
	while ( dl = directlights[l] )
	{
		directlights[l] = dl->next;
		free(dl);
	}
}

/*
=============
GatherSampleLight
=============
*/
#define NUMVERTEXNORMALS	162
float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "..\..\engine\anorms.h"
};

#define VectorMaximum(a) ( max( (a)[0], max( (a)[1], (a)[2] ) ) )

void GatherSampleLight (vec3_t pos, byte *pvs, vec3_t normal, vec3_t *sample, byte *styles)
{
	int				i;
	directlight_t	*l;
	vec3_t			add;
	vec3_t			delta;
	float			dot, dot2;
	float			dist;
	float			ratio;
	int				style_index;
	directlight_t	*sky_used = NULL;

	for (i = 1 ; i<numleafs ; i++)
	{
		if ( (l = directlights[i]) && (pvs[ (i-1)>>3] & (1<<((i-1)&7))) )
		{
			for (; l ; l=l->next)
			{
				// skylights work fundamentally differently than normal lights
				if (l->type == emit_skylight)
				{
					// only allow one of each sky type to hit any given point
					if (sky_used)
						continue;
					sky_used = l;

					// make sure the angle is okay
					dot = -DotProduct( normal, l->normal );
					if (dot <= ON_EPSILON/10)
						continue;

					// search back to see if we can hit a sky brush
					VectorScale( l->normal, -10000, delta );
					VectorAdd( pos, delta, delta );
					if (TestLine_r (0, pos, delta) != CONTENTS_SKY)
						continue;	// occluded
					
					VectorScale(l->intensity, dot, add);
				}
				else
				{
					VectorSubtract (l->origin, pos, delta);
					dist = VectorNormalize (delta);
					dot = DotProduct (delta, normal);
					if (dot <= ON_EPSILON/10)
						continue;	// behind sample surface

					if (dist < 1.0)
						dist = 1.0;

					switch (l->type)
					{
						case emit_point:
							ratio = dot / (dist * dist);
							VectorScale(l->intensity, ratio, add);
							break;

						case emit_surface:
							dot2 = -DotProduct (delta, l->normal);
							if (dot2 <= ON_EPSILON/10)
								continue; // behind light surface
							ratio = dot * dot2 / (dist * dist);
							VectorScale(l->intensity, ratio, add);
							break;

						case emit_spotlight:
							dot2 = -DotProduct (delta, l->normal);
							if (dot2 <= l->stopdot2)
								continue; // outside light cone
							ratio = dot * dot2 / (dist * dist);
							if (dot2 <= l->stopdot)
								ratio *= (dot2 - l->stopdot2) / (l->stopdot - l->stopdot2);
							VectorScale(l->intensity, ratio, add);
							break;
						default:
							Error ("Bad l->type");
					}
				}

				if( VectorMaximum( add ) > ( l->style ? coring : 0 ) )
				{
					if ( l->type != emit_skylight && TestLine_r (0, pos, l->origin) != CONTENTS_EMPTY )
						continue;	// occluded


					for( style_index = 0; style_index < MAXLIGHTMAPS; style_index++ )
						if ( styles[style_index] == l->style || styles[style_index] == 255 )
							break;

					if ( style_index == MAXLIGHTMAPS )
					{
						printf ("WARNING: Too many direct light styles on a face(%f,%f,%f)\n", 
							pos[0], pos[1], pos[2] );
						continue;
					}
					
					if ( styles[style_index] == 255 )
						styles[style_index] = l->style;

					VectorAdd( sample[style_index], add, sample[style_index] );
				}
			}
		}
	}
	if (sky_used && indirect_sun != 0.0)
	{
		vec3_t total;
		int j;
		vec3_t sky_intensity;

		VectorScale( sky_used->intensity, indirect_sun / (NUMVERTEXNORMALS * 2), sky_intensity );

		total[0] = total[1] = total[2] = 0.0;
		for (j = 0; j < NUMVERTEXNORMALS; j++)
		{
			// make sure the angle is okay
			dot = -DotProduct( normal, r_avertexnormals[j] );
			if (dot <= ON_EPSILON/10)
				continue;

			// search back to see if we can hit a sky brush
			VectorScale( r_avertexnormals[j], -10000, delta );
			VectorAdd( pos, delta, delta );
			if (TestLine_r (0, pos, delta) != CONTENTS_SKY)
				continue;	// occluded
			
			VectorScale(sky_intensity, dot, add);
			VectorAdd(total, add, total);
		}
		if( VectorMaximum( total ) > 0 )
		{
			for( style_index = 0; style_index < MAXLIGHTMAPS; style_index++ )
				if ( styles[style_index] == sky_used->style || styles[style_index] == 255 )
					break;

			if ( style_index == MAXLIGHTMAPS )
			{
				printf ("WARNING: Too many direct light styles on a face(%f,%f,%f)\n", 
					pos[0], pos[1], pos[2] );
				return;
			}
			
			if ( styles[style_index] == 255 )
				styles[style_index] = sky_used->style;

			VectorAdd( sample[style_index], total, sample[style_index] );
		}
	}
}

/*
=============
AddSampleToPatch

Take the sample's collected light and
add it back into the apropriate patch
for the radiosity pass.
=============
*/
void AddSampleToPatch (sample_t *s, int facenum)
{
	patch_t	*patch;
	vec3_t	mins, maxs;
	int		i;

	if (numbounce == 0)
		return;
	if( VectorAvg( s->light ) < 1)
		return;

	for (patch = face_patches[facenum] ; patch ; patch=patch->next)
	{
		// see if the point is in this patch (roughly)
		WindingBounds (patch->winding, mins, maxs);
		for (i=0 ; i<3 ; i++)
		{
			if (mins[i] > s->pos[i] + 16)
				goto nextpatch;
			if (maxs[i] < s->pos[i] - 16)
				goto nextpatch;
		}

		// add the sample to the patch
		patch->samples++;
		VectorAdd( patch->samplelight, s->light, patch->samplelight );
		//return;

nextpatch:;
	}

	// don't worry if some samples don't find a patch
}

void
GetPhongNormal( int facenum, vec3_t spot, vec3_t phongnormal )
{
	int	j;
	dface_t		*f = dfaces + facenum;
	dplane_t	*p = dplanes + f->planenum;
	vec3_t		facenormal;

	VectorCopy( p->normal, facenormal );
	if ( f->side )
		VectorSubtract( vec3_origin, facenormal, facenormal );
	VectorCopy( facenormal, phongnormal );

	if ( smoothing_threshold != 0 )
	{
		// Calculate modified point normal for surface
		// Use the edge normals iff they are defined.  Bend the surface towards the edge normal(s)
		// Crude first attempt: find nearest edge normal and do a simple interpolation with facenormal.
		// Second attempt: find edge points+center that bound the point and do a three-point triangulation(baricentric)
		// Better third attempt: generate the point normals for all vertices and do baricentric triangulation.

		for (j=0 ; j<f->numedges ; j++)
		{
			vec3_t	p1, p2, v1, v2, vspot;
			int e = dsurfedges[f->firstedge + j];
			int e1 = dsurfedges[f->firstedge + ((j-1)%f->numedges)];
			int e2 = dsurfedges[f->firstedge + ((j+1)%f->numedges)];
			vec3_t	n1, n2;
			edgeshare_t	*es = &edgeshare[abs(e)];
			edgeshare_t	*es1 = &edgeshare[abs(e1)];
			edgeshare_t	*es2 = &edgeshare[abs(e2)];
			dface_t	*f2;
			float		a, a1, a2, d1, d2, aa, bb, ab;

			if ( es->coplanar && es1->coplanar && es2->coplanar 
			|| VectorCompare(es->interface_normal, vec3_origin)
			&& VectorCompare(es1->interface_normal, vec3_origin)
			&& VectorCompare(es2->interface_normal, vec3_origin) )
				continue;

			if (e > 0)
			{
				f2 = es->faces[1];
				VectorCopy( dvertexes[dedges[e].v[0]].point, p1 );
				VectorCopy( dvertexes[dedges[e].v[1]].point, p2 );
			}
			else
			{
				f2 = es->faces[0];
				VectorCopy( dvertexes[dedges[-e].v[1]].point, p1 );
				VectorCopy( dvertexes[dedges[-e].v[0]].point, p2 );
			}


			// Build vectors from the middle of the face to the edge vertexes and the sample pos.
			VectorSubtract( p1, face_centroids[facenum], v1 );
			VectorSubtract( p2, face_centroids[facenum], v2 );
			VectorSubtract( spot, face_centroids[facenum], vspot );
			aa = DotProduct( v1, v1 );
			bb = DotProduct( v2, v2 );
			ab = DotProduct( v1, v2 );
			a1 = (bb * DotProduct( v1, vspot ) - ab * DotProduct( vspot, v2 )) / (aa * bb - ab * ab);
			a2 = (DotProduct( vspot, v2 ) - a1 * ab) / bb;

			// Test center to sample vector for inclusion between center to vertex vectors (Use dot product of vectors)
			if ( a1 >= 0.0 && a2 >= 0.0)
			{
				// calculate distance from edge to pos
				vec3_t	temp;
				VectorAdd( es->interface_normal, es1->interface_normal, n1 );

				if ( VectorCompare( n1, vec3_origin ) )
					VectorCopy( facenormal, n1 );
				VectorNormalize(n1);

				VectorAdd( es->interface_normal, es2->interface_normal, n2 );

				if ( VectorCompare( n2, vec3_origin ) )
					VectorCopy( facenormal, n2 );
				VectorNormalize(n2);

				// Interpolate between the center and edge normals based on sample position
				VectorScale( facenormal, 1.0 - a1 - a2, phongnormal );
				VectorScale( n1, a1, temp );
				VectorAdd( phongnormal, temp, phongnormal );
				VectorScale( n2, a2, temp );
				VectorAdd( phongnormal, temp, phongnormal );
				VectorNormalize( phongnormal );
				break;

			}
		}
	}
}


/*
=============
BuildFacelights
=============
*/
void BuildFacelights (int facenum)
{
	dface_t		*f;
	vec3_t		sampled[MAXLIGHTMAPS];
	lightinfo_t	l;
	int			i, j, k;
	sample_t	*s;
	float		*spot;
	patch_t		*patch;
	byte		pvs[(MAX_MAP_LEAFS+7)/8];
    int         thisoffset = -1, lastoffset = -1;
	int			lightmapwidth, lightmapheight, size;
	vec3_t centroid = { 0, 0, 0 };

	f = &dfaces[facenum];

//
// some surfaces don't need lightmaps
//
	f->lightofs = -1;
	for (j=0 ; j<MAXLIGHTMAPS ; j++)
		f->styles[j] = 255;

	if ( texinfo[f->texinfo].flags & TEX_SPECIAL)
		return;		// non-lit texture

	f->styles[0] = 0; // Everyone gets the style zero map.

	memset (&l, 0, sizeof(l));
	l.surfnum = facenum;
	l.face = f;

//
// rotate plane
//
	VectorCopy (dplanes[f->planenum].normal, l.facenormal);
	l.facedist = dplanes[f->planenum].dist;
	if (f->side)
	{
		VectorSubtract (vec3_origin, l.facenormal, l.facenormal);
		l.facedist = -l.facedist;
	}

	CalcFaceVectors (&l);
	CalcFaceExtents (&l);
	CalcPoints (&l);

	lightmapwidth = l.texsize[0]+1;
	lightmapheight = l.texsize[1]+1;

	size = lightmapwidth*lightmapheight;
	if (size > SINGLEMAP)
		Error ("Bad lightmap size");

	facelight[facenum].numsamples = l.numsurfpt;

	for (k=0 ; k<MAXLIGHTMAPS; k++)
		facelight[facenum].samples[k] = calloc(l.numsurfpt, sizeof(sample_t));

	spot = l.surfpt[0];
	for (i=0 ; i<l.numsurfpt ; i++, spot += 3)
	{
		vec3_t	pointnormal = {0,0,0};

		for (k=0 ; k<MAXLIGHTMAPS; k++)
			VectorCopy (spot, facelight[facenum].samples[k][i].pos);

	    // get the PVS for the pos to limit the number of checks
        if (!visdatasize)
        {       
            memset (pvs, 255, (numleafs+7)/8 );
            lastoffset = -1;
        }
        else 
        {
            dleaf_t *leaf = PointInLeaf( spot );
            thisoffset = leaf->visofs;
            if ( i == 0 || thisoffset != lastoffset )
            { 
                if (thisoffset == -1)
                        Error ("leaf->visofs == -1");

                DecompressVis (&dvisdata[leaf->visofs], pvs);
            }
            lastoffset = thisoffset;
        }

		for( j = 0; j < MAXLIGHTMAPS; j++)
			VectorFill( sampled[j], 0 );

		// If we are doing "extra" samples, oversample the direct light around the point.
		if ( extra )
		{
			int		weighting[3][3] = { { 5, 9, 5 }, { 9, 16, 9 }, { 5, 9, 5 } };
			vec3_t	pos;
			int		s, t, subsamples = 0;
			for ( t = -1; t <= 1; t ++ )
			{
				for ( s = -1; s <= 1; s++ )
				{
					int	subsample = i + t * lightmapwidth + s;
					int	sample_s = i % lightmapwidth;
					int	sample_t = i / lightmapwidth;
					if ( (0 <= s+sample_s) && (s+sample_s < lightmapwidth)
					  && (0 <= t+sample_t) && (t+sample_t < lightmapheight) )
					{
						vec3_t		subsampled[MAXLIGHTMAPS];
						for( j = 0; j < MAXLIGHTMAPS; j++)
							VectorFill( subsampled[j], 0 );
						// Calculate the point one third of the way toward the "subsample point"
						VectorCopy( l.surfpt[i], pos );
						VectorAdd( pos, l.surfpt[i], pos );
						VectorAdd( pos, l.surfpt[subsample], pos );
						VectorScale( pos, 1.0/3.0, pos );

						GetPhongNormal( facenum, pos, pointnormal );
						GatherSampleLight( pos, pvs, pointnormal, subsampled, f->styles );
						for( j = 0; j < MAXLIGHTMAPS && (f->styles[j] != 255); j++)
							{
							VectorScale( subsampled[j], weighting[s+1][t+1], subsampled[j] );
							VectorAdd( sampled[j], subsampled[j], sampled[j] );
							}
						subsamples += weighting[s+1][t+1];
					}
				}
			}
			for( j=0; j < MAXLIGHTMAPS && (f->styles[j] != 255); j++ )
				VectorScale( sampled[j], 1.0/subsamples, sampled[j] );
		}
		else
		{
			GetPhongNormal( facenum, spot, pointnormal );
			GatherSampleLight( spot, pvs, pointnormal, sampled, f->styles );
		}

		for( j=0; j < MAXLIGHTMAPS && (f->styles[j] != 255); j++ )
		{
			VectorCopy (sampled[j], facelight[facenum].samples[j][i].light );
			if ( f->styles[j] == 0 )
			{
				AddSampleToPatch ( &facelight[facenum].samples[j][i], facenum);
			}
		}
	}

	// average up the direct light on each patch for radiosity
	if (numbounce > 0)
	{
		for (patch = face_patches[facenum] ; patch ; patch=patch->next)
		{
			if (patch->samples)
			{ 
				vec3_t v;		// BUGBUG: Use a weighted average instead?
				VectorScale( patch->samplelight, (1.0f/patch->samples), v );
				VectorAdd( patch->totallight, v, patch->totallight );
				VectorAdd( patch->directlight, v, patch->directlight );
			}
		}
	}

	// add an ambient term if desired
	if (ambient[0] || ambient[1] || ambient[2])
	{
		for( j=0; j < MAXLIGHTMAPS && f->styles[j] != 255; j++ )
		{
			if ( f->styles[j] == 0 )
			{
				s = facelight[facenum].samples[j];
				for (i=0 ; i<l.numsurfpt ; i++, s++)
					VectorAdd(s->light, ambient, s->light);
				break;
			}
		}

	}

	// light from dlight_threshold and above is sent out, but the
	// texture itself should still be full bright

	// if( VectorAvg( face_patches[facenum]->baselight ) >= dlight_threshold)	// Now all lighted surfaces glow
	{
		for( j=0; j < MAXLIGHTMAPS && f->styles[j] != 255; j++ )
		{
			if ( f->styles[j] == 0 )
			{
				s = facelight[facenum].samples[j];
				for (i=0 ; i<l.numsurfpt ; i++, s++)
					VectorAdd( s->light, face_patches[facenum]->baselight, s->light ); 
				break;
			}
		}
	}
}

/*
=============
ProgressiveRefinement

Progressive mesh refinement of the patches
=============
*/

int
ProgressiveRefinement()
{
	return 0;
}

/*
=============
PrecompLightmapOffsets
=============
*/

void
PrecompLightmapOffsets()
{
int			facenum;
dface_t		*f;
patch_t		*patch;
facelight_t	*fl;
int			lightstyles;

lightdatasize = 0;

for( facenum = 0; facenum < numfaces; facenum++ )
	{
	f = &dfaces[facenum];
	fl = &facelight[facenum];

	if ( texinfo[f->texinfo].flags & TEX_SPECIAL)
		continue;		// non-lit texture

	for (lightstyles=0; lightstyles < MAXLIGHTMAPS; lightstyles++ )
		if ( f->styles[lightstyles] == 255 )
			break;

	if ( !lightstyles )
		continue;

	f->lightofs = lightdatasize;
	lightdatasize += fl->numsamples * 3 * lightstyles;
	}
}

/*
=============
FinalLightFace

Add the indirect lighting on top of the direct
lighting and save into final map format
=============
*/
void FinalLightFace (int facenum)
{
	dface_t	*f, *f2;
	int		i, j, k;
	vec3_t	lb, v;
	patch_t	*patch;
	triangulation_t	*trian;
	edgeshare_t	*es;
	int		edgenum;
	facelight_t	*fl;
	sample_t	*samp;
	triangle_t	*last_tri;
	float		minlight;
	int			lightstyles;

	f = &dfaces[facenum];
	fl = &facelight[facenum];

	if ( texinfo[f->texinfo].flags & TEX_SPECIAL)
		return;		// non-lit texture

	for (lightstyles=0; lightstyles < MAXLIGHTMAPS; lightstyles++ )
		if ( f->styles[lightstyles] == 255 )
			break;

	if ( !lightstyles )
		return;

	//
	// set up the triangulation
	//
	if (numbounce > 0)
	{
		trian = AllocTriangulation (&dplanes[f->planenum]);

		for (patch = face_patches[facenum] ; patch ; patch=patch->next)
			AddPatchToTriangulation (patch, trian);
		for (j=0 ; j<f->numedges ; j++)
		{
			edgenum = dsurfedges[f->firstedge + j];
			if (edgenum > 0)
			{
				es = &edgeshare[edgenum];
				f2 = es->faces[1];
			}
			else
			{
				es = &edgeshare[-edgenum];
				f2 = es->faces[0];
			}

			if (!es->coplanar && VectorCompare(vec3_origin, es->interface_normal) )
				continue;
			for (patch = face_patches[f2-dfaces] ; patch ; patch=patch->next)
				AddPatchToTriangulation (patch, trian);
		}

		TriangulatePoints (trian);
	}
	//
	// sample the triangulation
	//
	minlight = FloatForKey (face_entity[facenum], "_minlight") * 128;

	for (k=0 ; k < lightstyles; k++ )
	{
		last_tri = NULL;
		samp = fl->samples[k];
		for (j=0 ; j<fl->numsamples ; j++, samp++)
		{
			// Should be a VectorCopy, but we scale by 2 to compensate for an earlier lighting flaw
			// Specifically, the directlight contribution was included in the bounced light AND the directlight
			// Since many of the levels were built with this assumption, this "fudge factor" compensates for it.
			VectorScale( samp->light, 2.0, lb ); 

			if (numbounce > 0 && k == 0 )
			{
				SampleTriangulation (samp->pos, trian, &last_tri, v);
				VectorAdd( lb, v, lb );
			}

			VectorScale( lb, lightscale, lb );

			// clip from the bottom first
			for( i=0; i<3; i++ )
				if( lb[i] < minlight )
					lb[i] = minlight;

			// clip from the top
			if( lb[0]>maxlight || lb[1]>maxlight || lb[2]>maxlight )
			{
				// find max value and scale the whole color down;
				float max = lb[0] > lb[1] ? lb[0] : lb[1];
				max = max > lb[2] ? max : lb[2];

				for( i=0; i<3; i++ )
					lb[i] = ( lb[i] * maxlight ) / max;
			}

			// gamma adjust
			if (gamma != 1.0)
				for( i=0; i<3; i++ )
					lb[i] = (float)pow( lb[i] / 256.0f, gamma ) * 256.0f;

			dlightdata[f->lightofs + k*fl->numsamples*3 + j*3] = (unsigned char)lb[0];
			dlightdata[f->lightofs + k*fl->numsamples*3 + j*3 + 1] = (unsigned char)lb[1];
			dlightdata[f->lightofs + k*fl->numsamples*3 + j*3 + 2] = (unsigned char)lb[2];
		}
	}

	if (numbounce > 0)
		FreeTriangulation (trian);
}
