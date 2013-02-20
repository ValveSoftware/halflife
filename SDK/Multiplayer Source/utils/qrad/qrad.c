/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

// qrad.c

#include "qrad.h"


/*

NOTES
-----

every surface must be divided into at least two patches each axis

*/

patch_t		*face_patches[MAX_MAP_FACES];
entity_t	*face_entity[MAX_MAP_FACES];
patch_t		patches[MAX_PATCHES];
unsigned	num_patches;
vec3_t		emitlight[MAX_PATCHES];
vec3_t		addlight[MAX_PATCHES];
vec3_t		face_offset[MAX_MAP_FACES];		// for rotating bmodels
dplane_t	backplanes[MAX_MAP_PLANES];

unsigned	numbounce = 1; // 3; /* Originally this was 8 */

float		maxchop = 64;
float		minchop = 64;
qboolean	dumppatches;

int TestLine (vec3_t start, vec3_t stop);

int			junk;

vec3_t		ambient = { 0, 0, 0 };
float		maxlight = 256; // 196  /* Originally this was 196 */

float		lightscale = 1.0;
float		dlight_threshold = 25.0;  // was DIRECT_LIGHT constant

char		source[MAX_PATH] = "";

char		global_lights[MAX_PATH] = "";
char		designer_lights[MAX_PATH] = "";
char		level_lights[MAX_PATH] = "";

char		transferfile[MAX_PATH] = "";
char		vismatfile[_MAX_PATH] = "";
char		incrementfile[_MAX_PATH] = "";
qboolean	incremental = 0;
float		gamma = 0.5;
float		indirect_sun = 1.0;
qboolean	extra = false;
float		smoothing_threshold = 0; // default: cos(45.0*(Q_PI/180)); 
// Cosine of smoothing angle(in radians)
float		coring = 1.0;	// Light threshold to force to blackness(minimizes lightmaps)
qboolean	texscale = true;

/*
===================================================================

MISC

===================================================================
*/


/*
=============
MakeBackplanes
=============
*/
void MakeBackplanes (void)
{
	int		i;

	for (i=0 ; i<numplanes ; i++)
	{
		backplanes[i].dist = -dplanes[i].dist;
		VectorSubtract (vec3_origin, dplanes[i].normal, backplanes[i].normal);
	}
}

int		leafparents[MAX_MAP_LEAFS];
int		nodeparents[MAX_MAP_NODES];

/*
=============
MakeParents
=============
*/
void MakeParents (int nodenum, int parent)
{
	int		i, j;
	dnode_t	*node;

	nodeparents[nodenum] = parent;
	node = dnodes+nodenum;

	for (i=0 ; i<2 ; i++)
	{
		j = node->children[i];
		if (j < 0)
			leafparents[-j - 1] = nodenum;
		else
			MakeParents (j, nodenum);
	}
}


/*
===================================================================

  TEXTURE LIGHT VALUES

===================================================================
*/

typedef struct
{
	char	name[256];
	vec3_t	value;
	char	*filename;
} texlight_t;

#define	MAX_TEXLIGHTS	128

texlight_t	texlights[MAX_TEXLIGHTS];
int			num_texlights;

/*
============
ReadLightFile
============
*/
void ReadLightFile (char *filename)
{
	FILE	*f;
	char	scan[128];
	short	argCnt;
	vec_t	intensity;
	int		i = 1.0, j, file_texlights = 0;

	f = fopen (filename, "r");
	if (!f)
		Error ("ERROR: Couldn't open texlight file %s", filename);
	else
		printf("[Reading texlights from '%s']\n", filename);

	while ( fgets(scan, sizeof(scan), f) )
	{
		char	szTexlight[256];
		vec_t	r, g, b, i = 1;
		if (num_texlights == MAX_TEXLIGHTS)
			Error ("MAX_TEXLIGHTS");

		argCnt = sscanf (scan, "%s %f %f %f %f",szTexlight, &r, &g, &b, &i );
		
		if( argCnt == 2 )
		{
			// With 1+1 args, the R,G,B values are all equal to the first value
			g = b = r;
		}
		else if ( argCnt == 5 )
		{
			// With 1+4 args, the R,G,B values are "scaled" by the fourth numeric value i;
			r *= i / 255.0;
			g *= i / 255.0;
			b *= i / 255.0;
		}
		else if( argCnt != 4 )
		{
			if (strlen( scan ) > 4)
				printf("ignoring bad texlight '%s' in %s", scan, filename );
			continue;
		}

		for( j=0; j<num_texlights; j++ )
		{
			if ( strcmp( texlights[j].name, szTexlight ) == 0 )
			{
				if ( strcmp(texlights[j].filename, filename ) == 0 )
				{
					printf( "ERROR\a: Duplication of '%s' in file '%s'!\n",
							texlights[j].name, texlights[j].filename );
				} 
				else if ( texlights[j].value[0] != r
				  || texlights[j].value[1] != g
				  || texlights[j].value[2] != b )
				{
					printf( "Warning: Overriding '%s' from '%s' with '%s'!\n", 
							texlights[j].name, texlights[j].filename, filename );
				}
				else
				{
					printf( "Warning: Redundant '%s' def in '%s' AND '%s'!\n", 
							texlights[j].name, texlights[j].filename, filename );
				}
				break;
			}
		}
		strcpy( texlights[j].name, szTexlight );
		texlights[j].value[0] = r;
		texlights[j].value[1] = g;
		texlights[j].value[2] = b;
		texlights[j].filename = filename;
		file_texlights++;

		num_texlights = max( num_texlights, j+1 );
	}		
	qprintf ("[%i texlights parsed from '%s']\n\n", file_texlights, filename);
}


/*
============
LightForTexture
============
*/
void LightForTexture( char *name, vec3_t result )
{
	int		i;

	result[ 0 ] = result[ 1 ] = result[ 2 ] = 0;

	for (i=0 ; i<num_texlights ; i++)
	{
		if (!Q_strcasecmp (name, texlights[i].name))
		{
			VectorCopy( texlights[i].value, result );
			return;
		}
	}
}

/*
=======================================================================

MAKE FACES

=======================================================================
*/

/*
=============
WindingFromFace
=============
*/
winding_t	*WindingFromFace (dface_t *f)
{
	int			i;
	int			se;
	dvertex_t	*dv;
	int			v;
	winding_t	*w;

	w = AllocWinding (f->numedges);
	w->numpoints = f->numedges;

	for (i=0 ; i<f->numedges ; i++)
	{
		se = dsurfedges[f->firstedge + i];
		if (se < 0)
			v = dedges[-se].v[1];
		else
			v = dedges[se].v[0];

		dv = &dvertexes[v];
		VectorCopy (dv->point, w->p[i]);
	}

	RemoveColinearPoints (w);

	return w;
}

/*
=============
BaseLightForFace
=============
*/
void BaseLightForFace( dface_t *f, vec3_t light, vec3_t reflectivity )
{
	texinfo_t	*tx;
	miptex_t	*mt;
	int			ofs;

	long		sum[3];
	long		samples = 0;
	int			x, y, i;

	//
	// check for light emited by texture
	//
	tx = &texinfo[f->texinfo];

	ofs = ((dmiptexlump_t *)dtexdata)->dataofs[tx->miptex];
	mt = (miptex_t *) ( (byte *)dtexdata + ofs);

	LightForTexture (mt->name, light);

#ifdef TEXTURE_REFLECTIVITY
	// Average up the texture pixels' color for an average reflectivity
	for ( x = 0; x < ; x++ )
		for ( y = 0; y < ; y++ )
			{
			samples++;
			for(i=0; i < 3; i++)
				sum[i] += mt[][x][y][i] // FIXME later
			}
	for(i=0; i < 3; i++)
		reflectivity[i] = samples ? (BYTE)(sum[i] / samples) : 0;
#endif
}

/*
=============
IsSky
=============
*/
qboolean IsSky (dface_t *f)
{
	texinfo_t	*tx;
	miptex_t	*mt;
	int			ofs;

	tx = &texinfo[f->texinfo];
	ofs = ((dmiptexlump_t *)dtexdata)->dataofs[tx->miptex];
	mt = (miptex_t *) ( (byte *)dtexdata + ofs);
	if (!strncmp (mt->name, "sky", 3) )
		return true;
	if (!strncmp (mt->name, "SKY", 3) )
		return true;
	return false;
}

/*
=============
MakePatchForFace
=============
*/
float	totalarea;
void MakePatchForFace (int fn, winding_t *w)
{
	dface_t *f = dfaces + fn;

	// No patches at all for the sky!
	if ( !IsSky(f) )
	{
		float	area;
		patch_t		*patch;
		vec3_t light;
		vec3_t		centroid = {0,0,0};
		int			i, j;
		texinfo_t	*tx = &texinfo[f->texinfo];

		area = WindingArea (w);
		totalarea += area;

		patch = &patches[num_patches];
		if (num_patches == MAX_PATCHES)
			Error ("num_patches == MAX_PATCHES");
		patch->next = face_patches[fn];
		face_patches[fn] = patch;

		if ( texscale )
			{
			// Compute the texture "scale" in s,t
			for( i=0; i<2; i++ )
				{
				patch->scale[i] = 0.0;
				for( j=0; j<3; j++ )
					patch->scale[i] += tx->vecs[i][j] * tx->vecs[i][j];
				patch->scale[i] = sqrt( patch->scale[i] );
				}
			}
		else
			patch->scale[0] = patch->scale[1] = 1.0;

		patch->area = area;
		patch->chop = maxchop / (int)((patch->scale[0]+patch->scale[1])/2);
		patch->sky = FALSE;

		patch->winding = w;

		if (f->side)
			patch->plane = &backplanes[f->planenum];
		else
			patch->plane = &dplanes[f->planenum];

		for (j=0 ; j<f->numedges ; j++)
		{
			int edge = dsurfedges[ f->firstedge + j ];
			int edge2 = dsurfedges[ j==f->numedges-1 ? f->firstedge : f->firstedge + j + 1 ];

			if (edge > 0)
			{
				VectorAdd( dvertexes[dedges[edge].v[0]].point, centroid, centroid );
				VectorAdd( dvertexes[dedges[edge].v[1]].point, centroid, centroid );
			}
			else
			{
				VectorAdd( dvertexes[dedges[-edge].v[1]].point, centroid, centroid );
				VectorAdd( dvertexes[dedges[-edge].v[0]].point, centroid, centroid );
			}
		}
		VectorScale( centroid, 1.0 / (f->numedges * 2), centroid );
		VectorCopy( centroid, face_centroids[fn] );  // Save them for generating the patch normals later.

		patch->faceNumber = fn;
		WindingCenter (w, patch->origin);

#ifdef PHONG_NORMAL_PATCHES
// This seems to be a bad idea for some reason.  Leave it turned off for now.
		VectorAdd (patch->origin, patch->plane->normal, patch->origin);
		GetPhongNormal( fn, patch->origin, patch->normal );
		VectorSubtract (patch->origin, patch->plane->normal, patch->origin);
		if ( !VectorCompare( patch->plane->normal, patch->normal ) )
			patch->chop = 16; // Chop it fine!
#else
		VectorCopy( patch->plane->normal, patch->normal );
#endif
		VectorAdd (patch->origin, patch->normal, patch->origin);

		WindingBounds (w, patch->face_mins, patch->face_maxs);
		VectorCopy( patch->face_mins, patch->mins );
		VectorCopy( patch->face_maxs, patch->maxs );

		BaseLightForFace( f, light, patch->reflectivity );
		VectorCopy( light, patch->totallight );
		VectorCopy( light, patch->baselight );

		// Chop all texlights very fine.
		if ( !VectorCompare( light, vec3_origin ) )
			patch->chop = extra ? minchop / 2 : minchop;

		num_patches++;
	}
}


entity_t *EntityForModel (int modnum)
{
	int		i;
	char	*s;
	char	name[16];

	sprintf (name, "*%i", modnum);
	// search the entities for one using modnum
	for (i=0 ; i<num_entities ; i++)
	{
		s = ValueForKey (&entities[i], "model");
		if (!strcmp (s, name))
			return &entities[i];
	}

	return &entities[0];
}

/*
=============
MakePatches
=============
*/
void MakePatches (void)
{
	int		i, j, k;
	dface_t	*f;
	int		fn;
	winding_t	*w;
	dmodel_t	*mod;
	vec3_t		origin;
	entity_t	*ent;
	char		*s;

	ParseEntities ();
	qprintf ("%i faces\n", numfaces);

	for (i=0 ; i<nummodels ; i++)
	{
		mod = dmodels+i;
		ent = EntityForModel (i);
		VectorCopy (vec3_origin, origin);

		// bmodels with origin brushes need to be offset into their
		// in-use position
		if ( *(s = ValueForKey(ent,"origin")) )
		{
			double	v1, v2, v3;
			if ( sscanf (s, "%lf %lf %lf", &v1, &v2, &v3) == 3 )
			{
				origin[0] = v1;
				origin[1] = v2;
				origin[2] = v3;
			}
		}

		for (j=0 ; j<mod->numfaces ; j++)
		{
			fn = mod->firstface + j;
			face_entity[fn] = ent;
			VectorCopy (origin, face_offset[fn]);
			f = dfaces+fn;
			w = WindingFromFace (f);
			for (k=0 ; k<w->numpoints ; k++)
			{
				VectorAdd (w->p[k], origin, w->p[k]);
			}
			MakePatchForFace (fn, w);
		}
	}

	qprintf ("%i square feet [%.2f square inches]\n", (int)(totalarea/144), totalarea );
}

/*
=======================================================================

SUBDIVIDE

=======================================================================
*/

/*
=============
SubdividePatch
=============
*/
void	SubdividePatch (patch_t *patch)
{
	winding_t *w, *o1, *o2;
	vec3_t	total;
	vec3_t	split;
	vec_t	dist;
	vec_t	widest = -1;
	int		i, j, widest_axis = -1;
	int		subdivide_it = 0;
	vec_t	v;
	patch_t	*newp;

	w = patch->winding;

	VectorSubtract (patch->maxs, patch->mins, total);
	for (i=0 ; i<3 ; i++)
	{
		if ( total[i] > widest )
			{
			widest_axis = i;
			widest = total[i];
			}
		if ( total[i] > patch->chop
		  || (patch->face_maxs[i] == patch->maxs[i] || patch->face_mins[i] == patch->mins[i] )
		  && total[i] > minchop )
		{
			subdivide_it = 1;
		}
	}

	if ( subdivide_it )
	{
		//
		// split the winding
		//
		VectorCopy (vec3_origin, split);
		split[widest_axis] = 1;
		dist = (patch->mins[widest_axis] + patch->maxs[widest_axis])*0.5f;
		ClipWinding (w, split, dist, &o1, &o2);

		//
		// create a new patch
		//
		if (num_patches == MAX_PATCHES)
			Error ("MAX_PATCHES");
		newp = &patches[num_patches];

		newp->next = patch->next;
		patch->next = newp;

		patch->winding = o1;
		newp->winding = o2;

		VectorCopy( patch->face_mins, newp->face_mins );
		VectorCopy( patch->face_maxs, newp->face_maxs );

		VectorCopy( patch->baselight, newp->baselight );
		VectorCopy( patch->directlight, newp->directlight );
		VectorCopy( patch->totallight, newp->totallight );
		VectorCopy( patch->reflectivity, newp->reflectivity );
		newp->plane = patch->plane;
		newp->sky = patch->sky;
		newp->chop = patch->chop;
		newp->faceNumber = patch->faceNumber;

		num_patches++;

		patch->area = WindingArea (patch->winding);
		newp->area = WindingArea (newp->winding);

		WindingCenter (patch->winding, patch->origin);
		WindingCenter (newp->winding, newp->origin);

#ifdef PHONG_NORMAL_PATCHES
// This seems to be a bad idea for some reason.  Leave it turned off for now.
		// Set (Copy or Calculate) the synthetic normal for these new patches
		VectorAdd (patch->origin, patch->plane->normal, patch->origin);
		VectorAdd (newp->origin, newp->plane->normal, newp->origin);
		GetPhongNormal( patch->faceNumber, patch->origin, patch->normal );
		GetPhongNormal( newp->faceNumber, newp->origin, newp->normal );
		VectorSubtract( patch->origin, patch->plane->normal, patch->origin);
		VectorSubtract( newp->origin, newp->plane->normal, newp->origin);
#else
		VectorCopy( patch->plane->normal, patch->normal );
		VectorCopy( newp->plane->normal, newp->normal );
#endif
		VectorAdd( patch->origin, patch->normal, patch->origin );
		VectorAdd( newp->origin, newp->normal, newp->origin );

		WindingBounds(patch->winding, patch->mins, patch->maxs);
		WindingBounds(newp->winding, newp->mins, newp->maxs);

		// Subdivide patch even more if on the edge of the face; this is a hack!
		VectorSubtract (patch->maxs, patch->mins, total);
		if ( total[0] < patch->chop && total[1] < patch->chop && total[2] < patch->chop )
			for ( i=0; i<3; i++ )
				if ( (patch->face_maxs[i] == patch->maxs[i] || patch->face_mins[i] == patch->mins[i] )
				  && total[i] > minchop )
				{
					patch->chop = max( minchop, patch->chop / 2 );
					break;
				}

		SubdividePatch (patch);

		// Subdivide patch even more if on the edge of the face; this is a hack!
		VectorSubtract (newp->maxs, newp->mins, total);
		if ( total[0] < newp->chop && total[1] < newp->chop && total[2] < newp->chop )
			for ( i=0; i<3; i++ )
				if ( (newp->face_maxs[i] == newp->maxs[i] || newp->face_mins[i] == newp->mins[i] )
				  && total[i] > minchop )
				{
					newp->chop = max( minchop, newp->chop / 2 );
					break;
				}

		SubdividePatch (newp);
	}
}


/*
=============
SubdividePatches
=============
*/
void SubdividePatches (void)
{
	int		i, num;

	num = num_patches;	// because the list will grow
	for (i=0 ; i<num ; i++)
		{
		patch_t *patch = patches + i;
		SubdividePatch( patch );
		}
	qprintf ("%i patches after subdivision\n", num_patches);
}

//=====================================================================

/*
=============
MakeScales

  This is the primary time sink.
  It can be run multi threaded.
=============
*/
int	total_transfer;

void MakeScales (int threadnum)
{
	int		i;
	unsigned j;
	vec3_t	delta;
	vec_t	dist, scale;
	int		count;
	float	trans;
	patch_t		*patch, *patch2;
	float		total, send;
	dplane_t	plane;
	vec3_t		origin;
	vec_t		area;
	transfer_t	transfers[MAX_PATCHES], *all_transfers;

	count = 0;

	while (1)
	{
		i = GetThreadWork ();
		if (i == -1)
			break;

		patch = patches + i;

		total = 0;
		patch->numtransfers = 0;

		VectorCopy (patch->origin, origin);
		plane = *patch->plane;
		plane.dist = PatchPlaneDist( patch );

		area = patch->area;

		// find out which patch2's will collect light
		// from patch

		all_transfers = transfers;
		for (j=0, patch2 = patches ; j<num_patches ; j++, patch2++)
		{
			if (!CheckVisBit (i, j))
				continue;

			// calculate transferemnce
			VectorSubtract (patch2->origin, origin, delta);
			dist = VectorNormalize (delta);
			
			// skys don't care about the interface angle, but everything
			// else does
			if (!patch->sky)
				scale = DotProduct (delta, patch->normal);
			else
				scale = 1;

			scale *= -DotProduct (delta, patch2->normal);

			trans = scale / (dist*dist);

			if (trans < -ON_EPSILON)
				Error ("transfer < 0");
			send = trans*patch2->area;
			if (send > 0.4f)
			{
				trans = 0.4f / patch2->area;
				send = 0.4f;
			}
			total += send;


			// scale to 16 bit
			trans = trans * area * INVERSE_TRANSFER_SCALE;
			if (trans >= 0x10000)
				trans = 0xffff;
			if (!trans)
				continue;
			all_transfers->transfer = (unsigned short)trans;
			all_transfers->patch = j;
			all_transfers++;
			patch->numtransfers++;
			count++;

		}

		// copy the transfers out
		if (patch->numtransfers)
		{
			transfer_t	*t, *t2;

			patch->transfers = calloc (patch->numtransfers, sizeof(transfer_t));

			if (!patch->transfers)
				Error ("Memory allocation failure");

			//
			// normalize all transfers so exactly 50% of the light
			// is transfered to the surroundings
			//
			total = 0.5f/total;
			t = patch->transfers;
			t2 = transfers;
			for (j=0 ; j<(unsigned)patch->numtransfers ; j++, t++, t2++)
			{
				t->transfer = (unsigned short)(t2->transfer*total);
				t->patch = t2->patch;
			}
		}
	}

	ThreadLock ();
	total_transfer += count;
	ThreadUnlock ();
}


/*
=============
WriteWorld
=============
*/
void WriteWorld (char *name)
{
	int			i;
	unsigned	j;
	FILE		*out;
	patch_t		*patch;
	winding_t	*w;

	out = fopen (name, "w");
	if (!out)
		Error ("Couldn't open %s", name);

	for (j=0, patch=patches ; j<num_patches ; j++, patch++)
	{
		w = patch->winding;
		fprintf (out, "%i\n", w->numpoints);
		for (i=0 ; i<w->numpoints ; i++)
		{
			fprintf (out, "%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n",
				w->p[i][0],
				w->p[i][1],
				w->p[i][2],
				patch->totallight[ 0 ] / 256,
				patch->totallight[ 1 ] / 256,
				patch->totallight[ 2 ] / 256 );
		}
		fprintf (out, "\n");
	}

	fclose (out);
}

/*
=============
SwapTransfersTask

Change transfers from light sent out to light collected in.
In an ideal world, they would be exactly symetrical, but
because the form factors are only aproximated, then normalized,
they will actually be rather different.
=============
*/
void SwapTransfersTask (int patchnum)
{
	int		j, k, l, m, n, h;
	patch_t	*patch, *patch2;
	transfer_t	*t, *t2;
	int		transfer;

	patch = patches + patchnum;

	t = patch->transfers;
	for (j=0 ; j<patch->numtransfers ; j++, t++)
	{
		k = t->patch;
		if (k > patchnum)
			break;		// done with this list
		patch2 = &patches[k];
		t2 = patch2->transfers;

		if (!patch2->numtransfers)
		{
			printf ("WARNING: SwapTransfers: unmatched\n");
			continue;
		}
		//
		// binary search for match
		//
		l = 0;
		h = patch2->numtransfers-1;
		while (1)
		{
			m = (l+h)>>1;
			n = t2[m].patch;
			if (n < patchnum)
			{
				l = m+1;
				continue;
			}
			if (n > patchnum)
			{
				h = m-1;
				continue;
			}

			t2 += m;
			transfer = t2->transfer;
			t2->transfer = t->transfer;
			t->transfer = transfer;
			break;
		}

#if 0
		for (l=0 ; l<patch2->numtransfers ; l++, t2++)
		{
			if (t2->patch == i)
			{
				transfer = t2->transfer;
				t2->transfer = t->transfer;
				t->transfer = transfer;
				break;
			}
		}
#endif
		if (l == patch2->numtransfers)
			Error ("Didn't match transfer");
	}
}

/*
=============
CollectLight
=============
*/
void CollectLight( vec3_t total )
{
	unsigned i;
	patch_t	*patch;

	VectorFill( total, 0 );

	for (i=0, patch=patches ; i<num_patches ; i++, patch++)
	{
		// sky's never collect light, it is just dropped
		if (patch->sky)
		{
			VectorFill( emitlight[ i ], 0 );
			VectorFill( addlight[ i ], 0 );
			continue;
		}

		VectorAdd( patch->totallight, addlight[i], patch->totallight );
		VectorScale( addlight[i], TRANSFER_SCALE, emitlight[i] );
		VectorAdd( total, emitlight[i], total );
		VectorFill( addlight[ i ], 0 );
	}

	VectorScale( total, INVERSE_TRANSFER_SCALE, total );
}

/*
=============
GatherLight

Get light from other patches
  Run multi-threaded
=============
*/
void GatherLight (int threadnum)
{
	int			j, k;
	transfer_t	*trans;
	int			num;
	patch_t		*patch;
	vec3_t		sum, v;

	while (1)
	{
		j = GetThreadWork ();
		if (j == -1)
			break;

		patch = &patches[j];

		trans = patch->transfers;
		num = patch->numtransfers;

		VectorFill( sum, 0 )

		for (k=0 ; k<num ; k++, trans++)
		{
			VectorScale( emitlight[trans->patch], trans->transfer, v );
			VectorAdd( sum, v, sum );
		}

		VectorCopy( sum, addlight[j] );
	}
}

/*
=============
BounceLight
=============
*/
void BounceLight (void)
{
	unsigned i;
	vec3_t	added;
	char	name[64];

	for (i=0 ; i<num_patches ; i++)
		VectorScale( patches[i].totallight, TRANSFER_SCALE, emitlight[i] );

	for (i=0 ; i<numbounce ; i++)
	{
		RunThreadsOn (num_patches, true, GatherLight);
		CollectLight( added );

		qprintf ("\tBounce #%i added RGB(%.0f, %.0f, %.0f)\n", i+1, added[0], added[1], added[2] );
		if ( dumppatches && (i==0 || i == (unsigned)numbounce-1) )
		{
			sprintf (name, "bounce%i.txt", i);
			WriteWorld (name);
		}
	}
}


/*
=============
writetransfers
=============
*/

long
writetransfers(char *transferfile, long total_patches)
{
	int		handle;
	long	writtenpatches = 0, writtentransfers = 0, totalbytes = 0;
	int		spacerequired = sizeof(long) + total_patches * sizeof(long) + total_transfer * sizeof(transfer_t);

	if ( spacerequired - getfilesize(transferfile) < getfreespace(transferfile) )
	{
		if ( (handle = _open( transferfile, _O_WRONLY | _O_BINARY | _O_CREAT | _O_TRUNC, _S_IREAD | _S_IWRITE )) != -1 )
		{
			unsigned			byteswritten;
			qprintf("Writing [%s] with new saved qrad data", transferfile );
			
			if ( (byteswritten = _write(handle, &total_patches, sizeof(total_patches))) == sizeof(total_patches) )
			{
				patch_t			*patch;

				totalbytes += byteswritten;

				for( patch = patches; total_patches-- > 0; patch++ )
				{
					if ( (byteswritten = _write(handle, &patch->numtransfers, sizeof(patch->numtransfers)))
					  == sizeof(patch->numtransfers) )
					{
						totalbytes += byteswritten;

						if ( patch->numtransfers && 
							 (byteswritten = _write(handle, patch->transfers, patch->numtransfers*sizeof(transfer_t)))
						      == patch->numtransfers*sizeof(transfer_t) )
						{
							totalbytes += byteswritten;
							writtentransfers += patch->numtransfers;
						}
						writtenpatches++;
					}
					else
					{
						break;
					}
				}
			}

			qprintf("(%d)\n", totalbytes );
			
			_close( handle );
		}
	}
	else
		printf("Insufficient disk space(%ld) for 'QRAD save file'[%s]!\n",
				spacerequired - getfilesize(transferfile), transferfile );


	return writtenpatches;
}

/*
=============
readtransfers
=============
*/

long
readtransfers(char *transferfile, long numpatches)
{
	int		handle;
	long	readpatches = 0, readtransfers = 0, totalbytes = 0;
	long	start, end;
	time(&start);
	if ( (handle = _open( transferfile, _O_RDONLY | _O_BINARY )) != -1 )
	{
		long			filepatches;
		unsigned long	bytesread;

		printf("%-20s Restoring [%-13s - ", "MakeAllScales:", transferfile );
		
		if ( (bytesread = _read(handle, &filepatches, sizeof(filepatches))) == sizeof(filepatches) )
		{
			if ( filepatches == numpatches )
			{

				patch_t			*patch;

				totalbytes += bytesread;

				for( patch = patches; readpatches < numpatches; patch++ )
				{
					if ( (bytesread = _read(handle, &patch->numtransfers, sizeof(patch->numtransfers)))
					  == sizeof(patch->numtransfers) )
					{
						if ( patch->transfers = calloc(patch->numtransfers, sizeof(patch->transfers[0])) )
						{
							totalbytes += bytesread;

							if ( patch->numtransfers )
							{
								if ( (bytesread = _read(handle, patch->transfers, patch->numtransfers*sizeof(transfer_t)))
								  == patch->numtransfers*sizeof(transfer_t) )
									{
										totalbytes += bytesread;
										readtransfers += patch->numtransfers;
									}
								else
								{
									printf("\nMissing transfer count!  Save file will now be rebuilt." );
									break;
								}
							}
							readpatches++;
						}
						else
						{
							printf("\nMemory allocation failure creating transfer lists(%d*%d)!\n",
								  patch->numtransfers, sizeof(transfer_t) );
							break;
						}
					}
					else
					{
						printf("\nMissing patch count!  Save file will now be rebuilt." );
						break;
					}
				}
			}
			else
				printf("\nIncorrect transfer patch count found!  Save file will now be rebuilt." );
		}
		_close( handle );
		time(&end);
		printf("%10.3fMB] (%d)\n",totalbytes/(1024.0*1024.0), end-start);
	}

	if (readpatches != numpatches )
		unlink(transferfile);
	else
		total_transfer = readtransfers;

	return readpatches;
}


//==============================================================

void MakeAllScales (void)
{
	strcpy(transferfile, source);
	StripExtension( transferfile );
	DefaultExtension( transferfile, ".r2" );

	if ( !incremental
	  || !IsIncremental(incrementfile)
	  || (unsigned)readtransfers(transferfile, num_patches) != num_patches )
	{
		// determine visibility between patches
		BuildVisMatrix ();

		RunThreadsOn (num_patches, true, MakeScales);
		if ( incremental )
			writetransfers(transferfile, num_patches);
		else
			unlink(transferfile);

		// release visibility matrix
		FreeVisMatrix ();
	}

	qprintf ("transfer lists: %5.1f megs\n"
		, (float)total_transfer * sizeof(transfer_t) / (1024*1024));
}

/*
=============
RadWorld
=============
*/
void RadWorld (void)
{
	int	i;

	MakeBackplanes ();
	MakeParents (0, -1);
	MakeTnodes (&dmodels[0]);

	// turn each face into a single patch
	MakePatches ();
	PairEdges ();

	// subdivide patches to a maximum dimension
	SubdividePatches ();

	do
	{
		// create directlights out of patches and lights
		CreateDirectLights ();

		// build initial facelights
		RunThreadsOnIndividual (numfaces, true, BuildFacelights);

		// free up the direct lights now that we have facelights
		DeleteDirectLights ();
	}
	while( numbounce != 0 && ProgressiveRefinement() );

	if (numbounce > 0)
	{
		// build transfer lists
		MakeAllScales ();

		// invert the transfers for gather vs scatter
		RunThreadsOnIndividual (num_patches, true, SwapTransfersTask);

		// spread light around
		BounceLight ();

		for( i=0; i < num_patches; i++ )
			if ( !VectorCompare( patches[i].directlight, vec3_origin ) )
				VectorSubtract( patches[i].totallight, patches[i].directlight, patches[i].totallight );
	}

	// blend bounced light into direct light and save
	PrecompLightmapOffsets();

	RunThreadsOnIndividual (numfaces, true, FinalLightFace);
}


/*
========
main

light modelfile
========
*/
extern char qproject[];
int main (int argc, char **argv)
{
	int		i;
	double		start, end;

	printf( "qrad.exe v 1.5 (%s)\n", __DATE__ );
	printf ("----- Radiosity ----\n");

	verbose = true;  // Originally FALSE
	smoothing_threshold = cos(45.0*(Q_PI/180)); // Originally zero.

	for (i=1 ; i<argc ; i++)
	{
		if (!strcmp(argv[i],"-dump"))
			dumppatches = true;
		else if (!strcmp(argv[i],"-bounce"))
		{
			if ( ++i < argc )
			{
				numbounce = atoi (argv[i]);
				if ( numbounce < 0 )
				{
					fprintf(stderr, "Error: expected non-negative value after '-bounce'\n" );
					return 1;
				}
			}
			else
			{
				fprintf( stderr, "Error: expected a value after '-bounce'\n" );
				return 1;
			}
		}
		else if (!strcmp(argv[i],"-verbose"))
		{
			verbose = true;
		}
		else if (!strcmp(argv[i],"-terse"))
		{
			verbose = false;
		}
		else if (!strcmp(argv[i],"-threads"))
		{
			if ( ++i < argc )
			{
				numthreads = atoi (argv[i]);
				if ( numthreads <= 0 )
				{
					fprintf(stderr, "Error: expected positive value after '-threads'\n" );
					return 1;
				}
			}
			else
			{
				fprintf( stderr, "Error: expected a value after '-threads'\n" );
				return 1;
			}
		}
		else if (!strcmp(argv[i],"-maxchop"))
		{
			if ( ++i < argc )
			{
				maxchop = (float)atof (argv[i]);
				if ( maxchop < 2 )
				{
					fprintf(stderr, "Error: expected positive value after '-maxchop'\n" );
					return 1;
				}
			}
			else
			{
				fprintf( stderr, "Error: expected a value after '-maxchop'\n" );
				return 1;
			}
		}
		else if (!strcmp(argv[i],"-chop"))
		{
			if ( ++i < argc )
			{
				minchop = (float)atof (argv[i]);
				if ( minchop < 1 )
				{
					fprintf(stderr, "Error: expected positive value after '-chop'\n" );
					return 1;
				}
				if ( minchop < 32 )
				{
					fprintf(stderr, "WARNING: Chop values below 32 are not recommended.  Use -extra instead.\n");
				}
			}
			else
			{
				fprintf( stderr, "Error: expected a value after '-chop'\n" );
				return 1;
			}
		}
		else if (!strcmp(argv[i],"-scale"))
		{
			if ( ++i < argc )
			{
				lightscale = (float)atof (argv[i]);
			}
			else
			{
				fprintf( stderr, "Error: expected a value after '-scale'\n" );
				return 1;
			}
		}
		else if (!strcmp(argv[i],"-ambient"))
		{
			if ( i+3 < argc )
			{
 				ambient[0] = (float)atof (argv[++i]) * 128;
 				ambient[1] = (float)atof (argv[++i]) * 128;
 				ambient[2] = (float)atof (argv[++i]) * 128;
			}
			else
			{
				fprintf( stderr, "Error: expected three color values after '-ambient'\n" );
				return 1;
			}
		}
		else if( !strcmp(argv[i], "-proj") )
		{
			if ( ++i < argc && *argv[i] )
				strcpy( qproject, argv[i] );
			else
			{
				fprintf(stderr, "Error: expected path name after '-proj'\n" );
				return 1;
			}
		}
		else if ( !strcmp(argv[i], "-maxlight") )
		{
			if ( ++i < argc && *argv[i] )
			{
				maxlight = (float)atof (argv[i]) * 128;
				if ( maxlight <= 0 )
				{
					fprintf(stderr, "Error: expected positive value after '-maxlight'\n" );
					return 1;
				}
			}
			else
			{
				fprintf( stderr, "Error: expected a value after '-maxlight'\n" );
				return 1;
			}
		}
		else if ( !strcmp(argv[i], "-lights" ) )
		{
			if ( ++i < argc && *argv[i] )
			{
				strcpy( designer_lights, argv[i] );
			}
			else
			{
				fprintf( stderr, "Error: expected a filepath after '-lights'\n" );
				return 1;
			}
		}
		else if ( !strcmp(argv[i], "-inc" ) )
		{
			incremental = true;
		} 
		else if (!strcmp(argv[i],"-gamma"))
		{
			if ( ++i < argc )
			{
				gamma = (float)atof (argv[i]);
			}
			else
			{
				fprintf( stderr, "Error: expected a value after '-gamma'\n" );
				return 1;
			}
		}
		else if (!strcmp(argv[i],"-dlight"))
		{
			if ( ++i < argc )
			{
				dlight_threshold = (float)atof (argv[i]);
			}
			else
			{
				fprintf( stderr, "Error: expected a value after '-dlight'\n" );
				return 1;
			}
		}
		else if (!strcmp(argv[i],"-extra"))
		{
			extra = true;
		}
		else if (!strcmp(argv[i],"-sky"))
		{
			if ( ++i < argc )
			{
				indirect_sun = (float)atof (argv[i]);
			}
			else
			{
				fprintf( stderr, "Error: expected a value after '-gamma'\n" );
				return 1;
			}
		}
		else if (!strcmp(argv[i],"-smooth"))
		{
			if ( ++i < argc )
			{
				smoothing_threshold = (float)cos(atof(argv[i])*(Q_PI/180.0));
			}
			else
			{
				fprintf( stderr, "Error: expected an angle after '-smooth'\n" );
				return 1;
			}
		}
		else if (!strcmp(argv[i],"-coring"))
		{
			if ( ++i < argc )
			{
				coring = (float)atof( argv[i] );
			}
			else
			{
				fprintf( stderr, "Error: expected a light threshold after '-coring'\n" );
				return 1;
			}
		}
		else if (!strcmp(argv[i],"-notexscale"))
		{
			texscale = false;
		}
		else
		{
			break;
		}
	}

	ThreadSetDefault ();

	if (maxlight > 255)
		maxlight = 255;

	if (i != argc - 1)
		Error ("usage: qrad [-dump] [-inc] [-bounce n] [-threads n] [-verbose] [-terse] [-chop n] [-maxchop n] [-scale n] [-ambient red green blue] [-proj file] [-maxlight n] [-threads n] [-lights file] [-gamma n] [-dlight n] [-extra] [-smooth n] [-coring n] [-notexscale] bspfile");

	start = I_FloatTime ();

	strcpy (source, argv[i]);
	StripExtension (source);
	SetQdirFromPath (source);

	// Set the required global lights filename
	strcat( strcpy( global_lights, gamedir ), "lights.rad" );
	if ( _access( global_lights, 0x04) == -1 ) 
	{
		// try looking in qproject
		strcat( strcpy( global_lights, qproject ), "lights.rad" );
		if ( _access( global_lights, 0x04) == -1 ) 
		{
			// try looking in the directory we were run from
			GetModuleFileName( NULL, global_lights, sizeof( global_lights ) );
			ExtractFilePath( global_lights, global_lights );
			strcat( global_lights, "lights.rad" );
		}
	}

	// Set the optional level specific lights filename
	DefaultExtension( strcpy( level_lights, source ), ".rad" );
	if ( _access( level_lights, 0x04) == -1 ) *level_lights = 0;	

	ReadLightFile(global_lights);							// Required
	if ( *designer_lights ) ReadLightFile(designer_lights);	// Command-line
	if ( *level_lights )	ReadLightFile(level_lights);	// Optional & implied

	strcpy(incrementfile, source);
	DefaultExtension(incrementfile, ".r0");
	DefaultExtension(source, ".bsp");

	LoadBSPFile (source);
	ParseEntities ();

	if (!visdatasize)
	{
		printf ("No vis information, direct lighting only.\n");
		numbounce = 0;
		ambient[0] = ambient[1] = ambient[2] = 0.1f;
	}

	RadWorld ();

	if (verbose)
		PrintBSPFileSizes ();

	WriteBSPFile (source);

	if ( incremental )
	{
		if ( !IsIncremental(incrementfile) )
		{
			SaveIncremental(incrementfile);
		}
	}
	else
	{
		unlink(incrementfile);
	}

	end = I_FloatTime ();
	printf ("%5.0f seconds elapsed\n", end-start);
	
	return 0;
}

