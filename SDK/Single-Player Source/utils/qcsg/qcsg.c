/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

// csg4.c

#include "csg.h"

/*


NOTES
-----

  check map size for +/- 4k limit at load time

*/

int		brushfaces;
int		c_csgfaces;
FILE	*out[NUM_HULLS];

int		c_tiny, c_tiny_clip;
int		c_outfaces;

qboolean	hullfile = false;
static char qhullfile[ 256 ];

qboolean	glview;
qboolean	noclip;
qboolean	onlyents;
qboolean	wadtextures = true;

vec3_t		world_mins, world_maxs;

/*
==================
NewFaceFromFace

Duplicates the non point information of a face, used by SplitFace
==================
*/
bface_t *NewFaceFromFace (bface_t *in)
{
	bface_t	*newf;
	
	newf = malloc (sizeof(bface_t));
	memset (newf, 0, sizeof(newf));
	newf->contents = in->contents;
	newf->texinfo = in->texinfo;
	newf->planenum = in->planenum;
	newf->plane = in->plane;
	
	return newf;
}

void FreeFace (bface_t *f)
{
	free (f->w);
	free (f);
}



/*
=================
ClipFace

Clips a faces by a plane, returning the fragment on the backside
and adding any fragment to the outside.

Faces exactly on the plane will stay inside unless overdrawn by later brush

frontside is the side of the plane that holds the outside list

Precedence is necesary to handle overlapping coplanar faces.
=================
*/
#define	SPLIT_EPSILON	0.3
bface_t *ClipFace (brush_t *b, bface_t *f, bface_t **outside,
				 int splitplane, qboolean precedence)
{
	bface_t		*front;
	winding_t	*fw, *bw;
	vec_t		d;
	plane_t		*split;
	int			i, count[3];

	// handle exact plane matches special

	if (f->planenum == (splitplane^1) )
	{	// opposite side, so put on inside list
		return f;
	}

	if ( f->planenum == splitplane )
	{
		// coplanar
		if (precedence)
		{	// this fragment will go to the inside, because
			// the earlier one was clipped to the outside
			return f;
		}
		f->next = *outside;
		*outside = f;
		return NULL;
	}

	split = &mapplanes[splitplane];
#if 0
	count[0] = count[1] = count[2] = 0;
	for (i=0 ; i<f->w->numpoints ; i++)
	{
		d = DotProduct (f->w->p[i], split->normal) - split->dist;
		if (d < -SPLIT_EPSILON)
			count[1]++;
		else if (d > SPLIT_EPSILON)
			count[0]++;
		else
			count[2]++;
	}

	if (!count[0])
	{
		fw = NULL;
		bw = f->w;
	}
	else if (!count[1])
	{
		fw = f->w;
		bw = NULL;
	}
	else
#endif
		ClipWindingNoCopy (f->w, split->normal, split->dist, &fw, &bw);

	if (!fw)
		return f;

	if (!bw)
	{
		f->next = *outside;
		*outside = f;
		return NULL;
	}

	FreeWinding (f->w);

	front = NewFaceFromFace (f);
	front->w = fw;
	WindingBounds (fw, front->mins, front->maxs);
	front->next = *outside;
	*outside = front;

	f->w = bw;
	WindingBounds (bw, f->mins, f->maxs);

	return f;
}

/*
===========
WriteFace
===========
*/
void WriteFace (int hull, bface_t *f)
{
	int		i, j;
	winding_t	*w;
	static	int	level = 128;
	vec_t		light;

	ThreadLock ();
	if (!hull)
		c_csgfaces++;

	if (glview)
	{
		// .gl format
		w = f->w;
		fprintf (out[hull], "%i\n", w->numpoints);
		level+=28;
		light = (level&255)/255.0;
		for (i=0 ; i<w->numpoints ; i++)
		{
			fprintf (out[hull], "%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n",
				w->p[i][0],
				w->p[i][1],
				w->p[i][2],
				light,
				light,
				light);
		}
		fprintf (out[hull], "\n");
	}
	else
	{
		// .p0 format
		w = f->w;
		fprintf (out[hull], "%i %i %i %i\n", f->planenum, f->texinfo, f->contents, w->numpoints);
		for (i=0 ; i<w->numpoints ; i++)
		{
			fprintf (out[hull], "%5.2f %5.2f %5.2f\n",
				w->p[i][0],
				w->p[i][1],
				w->p[i][2]);
		}
		fprintf (out[hull], "\n");
	}

	ThreadUnlock ();
}

/*
==================
SaveOutside

The faces remaining on the outside list are final
polygons.  Write them to the output file.

Passable contents (water, lava, etc) will generate
a mirrored copy of the face to be seen from the inside.
==================
*/
void SaveOutside (brush_t *b, int hull, bface_t *outside, int mirrorcontents)
{
	bface_t	*f , *next, *f2;
	int		i;
	int		planenum;
	vec3_t	temp;

	for (f=outside ; f ; f=next)
	{
		next = f->next;

		if (WindingArea (f->w) < 1.0)
		{
			c_tiny++;
			qprintf ("Entity %i, Brush %i: tiny fragment\n"
				, b->entitynum, b->brushnum);
			continue;
		}

		// count unique faces
		if (!hull)
		{
			for (f2=b->hulls[hull].faces ; f2 ; f2=f2->next)
			{
				if (f2->planenum == f->planenum)
				{
					if (!f2->used)
					{
						f2->used = true;
						c_outfaces++;
					}
					break;
				}
			}
		}

		WriteFace (hull, f);

//		if (mirrorcontents != CONTENTS_SOLID)
		{
			f->planenum ^= 1;
			f->plane = &mapplanes[f->planenum];
			f->contents = mirrorcontents;

			// swap point orders
			for (i=0 ; i<f->w->numpoints/2 ; i++)	// add points backwards
			{
				VectorCopy (f->w->p[i], temp);
				VectorCopy (f->w->p[f->w->numpoints-1-i]
					, f->w->p[i]);
				VectorCopy (temp, f->w->p[f->w->numpoints-1-i]);
			}
			WriteFace (hull, f);
		}

		FreeFace (f);
	}
}


/*
==================
EncloseInside

Changes the contents on all faces that got clipped out
and moves them back to the outside list
==================
*/
void EncloseInside (bface_t *inside, bface_t **outside
					, int contents)
{
	bface_t	*f, *next;
	
	for (f=inside ; f ; f=next)
	{
		next = f->next;
		
		f->contents = contents;
		f->next = *outside;
		*outside = f;
	}
}


//==========================================================================

bface_t	*CopyFace (bface_t *f)
{
	bface_t	*n;

	n = NewFaceFromFace (f);
	n->w = CopyWinding (f->w);
	VectorCopy (f->mins, n->mins);
	VectorCopy (f->maxs, n->maxs);
	return n;
}


/*
==================
CopyFacesToOutside

Make a copy of all the faces of the brush, so they
can be chewed up by other brushes.

All of the faces start on the outside list.
As other brushes take bites out of the faces, the
fragments are moved to the inside list, so they
can be freed when they are determined to be
completely enclosed in solid.
==================
*/
bface_t *CopyFacesToOutside (brushhull_t *bh)
{
	bface_t		*f, *newf;
	bface_t		*outside;

	outside = NULL;
	
	for (f=bh->faces ; f ; f=f->next)
	{
		brushfaces++;

		newf = CopyFace (f);
		WindingBounds (newf->w, newf->mins, newf->maxs);
		newf->next = outside;
		outside = newf;
	}

	return outside;
}

//============================================================


/*
===========
CSGBrush
===========
*/
void CSGBrush (int brushnum)
{
	int			hull;
	brush_t		*b1, *b2;
	brushhull_t	*bh1, *bh2;
	int			bn;
	qboolean	overwrite;
	int			i;
	bface_t		*f, *f2, *next, *fcopy;
	bface_t		*outside, *oldoutside;
	entity_t	*e;
	vec_t		area;

	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);

	b1 = &mapbrushes[brushnum];

	e = &entities[b1->entitynum];

	for (hull = 0 ; hull<NUM_HULLS ; hull++)
	{
		bh1 = &b1->hulls[hull];

		// set outside to a copy of the brush's faces
		outside = CopyFacesToOutside (bh1);
		overwrite = false;

		for (bn=0 ; bn<e->numbrushes ; bn++)
		{
			// see if b2 needs to clip a chunk out of b1

			if (bn==brushnum)
			{
				overwrite = true;	// later brushes now overwrite
				continue;
			}

			b2 = &mapbrushes[e->firstbrush + bn];
			bh2 = &b2->hulls[hull];

			if (!bh2->faces)
				continue;		// brush isn't in this hull

			// check brush bounding box first
			for (i=0 ; i<3 ; i++)
				if (bh1->mins[i] > bh2->maxs[i] 
				|| bh1->maxs[i] < bh2->mins[i])
					break;
			if (i<3)
				continue;

			// divide faces by the planes of the b2 to find which
			// fragments are inside
		
			f = outside;
			outside = NULL;
			for ( ; f ; f=next)
			{
				next = f->next;

				// check face bounding box first
				for (i=0 ; i<3 ; i++)
					if (bh2->mins[i] > f->maxs[i] 
					|| bh2->maxs[i] < f->mins[i])
						break;
				if (i<3)
				{	// this face doesn't intersect brush2's bbox
					f->next = outside;
					outside = f;
					continue;
				}

				oldoutside = outside;
				fcopy = CopyFace (f);	// save to avoid fake splits

				// throw pieces on the front sides of the planes
				// into the outside list, return the remains on the inside
				for (f2=bh2->faces ; f2 && f ; f2=f2->next)
					f = ClipFace (b1, f, &outside, f2->planenum, overwrite);

				area = f ? WindingArea (f->w) : 0;
				if (f && area < 1.0)
				{
					qprintf ("Entity %i, Brush %i: tiny penetration\n"
						, b1->entitynum, b1->brushnum);
					c_tiny_clip++;
					FreeFace (f);
					f = NULL;
				}
				if (f)
				{
					// there is one convex fragment of the original
					// face left inside brush2
					FreeFace (fcopy);

					if (b1->contents > b2->contents)
					{	// inside a water brush
						f->contents = b2->contents;
						f->next = outside;
						outside = f;
					}
					else	// inside a solid brush
						FreeFace (f);	// throw it away
				}
				else
				{	// the entire thing was on the outside, even
					// though the bounding boxes intersected,
					// which will never happen with axial planes

					// free the fragments chopped to the outside
					while (outside != oldoutside)
					{
						f2 = outside->next;
						FreeFace (outside);
						outside = f2;
					}

					// revert to the original face to avoid
					// unneeded false cuts
					fcopy->next = outside;
					outside = fcopy;
				}
			}

		}

		// all of the faces left in outside are real surface faces
		SaveOutside (b1, hull, outside, b1->contents);
	}
}

//======================================================================

/*
============
EmitPlanes
============
*/
void EmitPlanes (void)
{
	int			i;
	dplane_t	*dp;
	plane_t		*mp;

	numplanes = nummapplanes;
	mp = mapplanes;
	dp = dplanes;
	for (i=0 ; i<nummapplanes ; i++, mp++, dp++)
	{
		VectorCopy ( mp->normal, dp->normal);
		dp->dist = mp->dist;
		dp->type = mp->type;
	}
}

/*
============
SetModelNumbers
============
*/
void SetModelNumbers (void)
{
	int		i;
	int		models;
	char	value[10];

	models = 1;
	for (i=1 ; i<num_entities ; i++)
	{
		if (entities[i].numbrushes)
		{
			sprintf (value, "*%i", models);
			models++;
			SetKeyValue (&entities[i], "model", value);
		}
	}
}

/*
============
SetLightStyles
============
*/
#define	MAX_SWITCHED_LIGHTS	32
void SetLightStyles (void)
{
	int		stylenum;
	char	*t;
	entity_t	*e;
	int		i, j;
	char	value[10];
	char	lighttargets[MAX_SWITCHED_LIGHTS][64];


	// any light that is controlled (has a targetname)
	// must have a unique style number generated for it

	stylenum = 0;
	for (i=1 ; i<num_entities ; i++)
	{
		e = &entities[i];

		t = ValueForKey (e, "classname");
		if (Q_strncasecmp (t, "light", 5))
			continue;
		t = ValueForKey (e, "targetname");
		if (!t[0])
			continue;
		
		// find this targetname
		for (j=0 ; j<stylenum ; j++)
			if (!strcmp (lighttargets[j], t))
				break;
		if (j == stylenum)
		{
			if (stylenum == MAX_SWITCHED_LIGHTS)
				Error ("stylenum == MAX_SWITCHED_LIGHTS");
			strcpy (lighttargets[j], t);
			stylenum++;
		}
		sprintf (value, "%i", 32 + j);
		SetKeyValue (e, "style", value);
	}

}


/*
============
WriteBSP
============
*/
void WriteBSP (char *name)
{
	char	path[1024];

	strcpy (path, name);
	DefaultExtension (path, ".bsp");

	SetModelNumbers ();
	SetLightStyles ();
	UnparseEntities ();

	if ( !onlyents )
		WriteMiptex ();

	WriteBSPFile (path);
}

//======================================================================

/*
============
ProcessModels
============
*/
int typecontents[4] = {CONTENTS_WATER, CONTENTS_SLIME, CONTENTS_LAVA
, CONTENTS_SKY};

void ProcessModels (void)
{
	int		i, j, type;
	int		placed;
	int		first, contents;
	brush_t	temp;
	vec3_t	origin;

	for (i=0 ; i<num_entities ; i++)
	{
		if (!entities[i].numbrushes)
			continue;

		//
		// sort the contents down so stone bites water, etc
		//
		first = entities[i].firstbrush;
		placed = 0;
		for (type=0 ; type<4 ; type++)
		{
			contents = typecontents[type];
			for (j=placed+1 ; j< entities[i].numbrushes ; j++)
			{
				if (mapbrushes[first+j].contents == contents)
				{
					temp = mapbrushes[first+placed];
					mapbrushes[first+placed] = mapbrushes[j];
					mapbrushes[j] = temp;
					placed++;
				}
			}
		}

		//
		// csg them in order
		//
		if (i == 0)
		{
			RunThreadsOnIndividual (entities[i].numbrushes, 1 , CSGBrush);
		}
		else
		{
			for (j=0 ; j<entities[i].numbrushes ; j++)
				CSGBrush (first + j);
		}

		// write end of model marker
		if (!glview)
		{
			for (j=0 ; j<NUM_HULLS ; j++)
				fprintf (out[j], "-1 -1 -1 -1\n");
		}
	}
}

//=========================================

/*
============
BoundWorld
============
*/
void BoundWorld (void)
{
	int		i;
	brushhull_t	*h;

	ClearBounds (world_mins, world_maxs);

	for (i=0 ; i<nummapbrushes ; i++)
	{
		h = &mapbrushes[i].hulls[0];
		if (!h->faces)
			continue;
		AddPointToBounds (h->mins, world_mins, world_maxs);
		AddPointToBounds (h->maxs, world_mins, world_maxs);
	}

	qprintf ("World bounds: (%i %i %i) to (%i %i %i)\n",
		(int)world_mins[0],(int)world_mins[1],(int)world_mins[2],
		(int)world_maxs[0],(int)world_maxs[1],(int)world_maxs[2]);

	VectorCopy (world_mins, draw_mins);
	VectorCopy (world_maxs, draw_maxs);

	Draw_ClearWindow ();
}

/*
============
main
============
*/
extern char qproject[];

int main (int argc, char **argv)
{
	int		i, j;
	int		hull;
	entity_t	*ent;
	char	source[1024];
	char	name[1024];
	double		start, end;

	printf( "qcsg.exe v2.8 (%s)\n", __DATE__ );
	printf ("---- qcsg ----\n" );

	for (i=1 ; i<argc ; i++)
	{
		if (!strcmp(argv[i],"-threads"))
		{
			numthreads = atoi (argv[i+1]);
			i++;
		}
		else if (!strcmp(argv[i],"-glview"))
		{
			glview = true;
		}
		else if (!strcmp(argv[i], "-v"))
		{
			printf ("verbose = true\n");
			verbose = true;
		}
		else if (!strcmp(argv[i], "-draw"))
		{
			printf ("drawflag = true\n");
			drawflag = true;
		}
		else if (!strcmp(argv[i], "-noclip"))
		{
			printf ("noclip = true\n");
			noclip = true;
		}
		else if (!strcmp(argv[i], "-onlyents"))
		{
			printf ("onlyents = true\n");
			onlyents = true;
		}
		else if (!strcmp(argv[i], "-nowadtextures"))
		{
			printf ("wadtextures = false\n");
			wadtextures = false;
		}
		else if (!strcmp(argv[i], "-wadinclude"))
		{
			pszWadInclude[nWadInclude++] = strdup( argv[i + 1] );
			i++;
		}
		else if( !strcmp( argv[ i ], "-proj" ) )
		{
			strcpy( qproject, argv[ i + 1 ] );
			i++;
		}
		else if (!strcmp(argv[i], "-hullfile"))
		{
			hullfile = true;
			strcpy( qhullfile, argv[i + 1] );
			i++;
		}
		else if (argv[i][0] == '-')
			Error ("Unknown option \"%s\"", argv[i]);
		else
			break;
	}

	if (i != argc - 1)
		Error ("usage: qcsg [-nowadtextures] [-wadinclude <name>] [-draw] [-glview] [-noclip] [-onlyents] [-proj <name>] [-threads #] [-v] [-hullfile <name>] mapfile");

	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);
	start = I_FloatTime ();

	CheckHullFile( hullfile, qhullfile );

	ThreadSetDefault ();
	SetQdirFromPath (argv[i]);

	strcpy (source, ExpandArg (argv[i]));
	COM_FixSlashes(source);
	StripExtension (source);

	strcpy (name, ExpandArg (argv[i]));	
	DefaultExtension (name, ".map");	// might be .reg

	//
	// if onlyents, just grab the entites and resave
	//
	if (onlyents  && !glview)
	{
		char out[1024];
		int	old_entities;
		sprintf (out, "%s.bsp", source);
		LoadBSPFile (out);

		// Get the new entity data from the map file
		LoadMapFile (name);

		// Write it all back out again.
		WriteBSP (source);

		end = I_FloatTime ();
		printf ("%5.0f seconds elapsed\n", end-start);
		return 0;
	}

	//
	// start from scratch
	//
	LoadMapFile (name);

	RunThreadsOnIndividual (nummapbrushes, true, CreateBrush);

	BoundWorld ();

	qprintf ("%5i map planes\n", nummapplanes);

	for (i=0 ; i<NUM_HULLS ; i++)
	{
		char	name[1024];

		if (glview)
			sprintf (name, "%s.gl%i",source, i);
		else
			sprintf (name, "%s.p%i",source, i);
		out[i] = fopen (name, "w");
		if (!out[i])
			Error ("Couldn't open %s",name);
	}

	ProcessModels ();

	qprintf ("%5i csg faces\n", c_csgfaces);
	qprintf ("%5i used faces\n", c_outfaces);
	qprintf ("%5i tiny faces\n", c_tiny);
	qprintf ("%5i tiny clips\n", c_tiny_clip);

	for (i=0 ; i<NUM_HULLS ; i++)
		fclose (out[i]);

	if (!glview)
	{
		EmitPlanes ();
		WriteBSP (source);
	}

	end = I_FloatTime ();
	printf ("%5.0f seconds elapsed\n", end-start);

	return 0;
}

