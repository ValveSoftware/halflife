/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

// qbsp.c

#include "bsp5.h"

//
// command line flags
//
qboolean	drawflag;
qboolean	nofill;
qboolean	notjunc;
qboolean	allverbose;
qboolean	nogfx;
qboolean	noclip;
qboolean	leakonly;
qboolean	watervis;

int		subdivide_size = 240;

char	bspfilename[1024];
char	pointfilename[1024];
char	portfilename[1024];

FILE	*polyfiles[NUM_HULLS];

int		hullnum;

//===========================================================================


/*
=================
BaseWindingForPlane
=================
*/
winding_t *BaseWindingForPlane (dplane_t *p)
{
	int		i, x;
	vec_t	max, v;
	vec3_t	org, vright, vup;
	winding_t	*w;
	vec3_t	temp;

#if 0
// find the major axis

	max = -BOGUS_RANGE;
	x = -1;
	for (i=0 ; i<3; i++)
	{
		v = fabs(p->normal[i]);
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

	v = DotProduct (vup, p->normal);
	VectorMA (vup, -v, p->normal, vup);
	VectorNormalize (vup);
		
	VectorScale (p->normal, p->dist, org);
	
	CrossProduct (vup, p->normal, vright);
	
	VectorScale (vup, 8192, vup);
	VectorScale (vright, 8192, vright);
#else
	VectorScale (p->normal, p->dist, org);

	VectorCopy (vec3_origin, vup);
	VectorCopy (vec3_origin, vright);
	if (!p->normal[1] && !p->normal[2])
	{
		vup[2] = 8192;
		vright[1] = 8192*p->normal[0];
	}
	else if (!p->normal[0] && !p->normal[2])
	{
		vup[2] = 8192;
		vright[0] = -8192*p->normal[1];
	}
	else if (!p->normal[0] && !p->normal[1])
	{
		vup[1] = 8192;
		vright[0] = 8192*p->normal[2];
	}
	else
	{
		vup[0] = -2*p->normal[1]*p->normal[2];
		vup[1] = p->normal[0]*p->normal[2];
		vup[2] = p->normal[0]*p->normal[1];

		VectorNormalize (vup);

		VectorCopy (p->normal, temp);	// to vec3_t
		CrossProduct (vup, temp, vright);

		VectorScale (vup, 8192, vup);
		VectorScale (vright, 8192, vright);
	}
#endif

// project a really big	axis aligned box onto the plane
	w = NewWinding (4);
	
	VectorSubtract (org, vright, w->points[0]);
	VectorAdd (w->points[0], vup, w->points[0]);
	
	VectorAdd (org, vright, w->points[1]);
	VectorAdd (w->points[1], vup, w->points[1]);
	
	VectorAdd (org, vright, w->points[2]);
	VectorSubtract (w->points[2], vup, w->points[2]);
	
	VectorSubtract (org, vright, w->points[3]);
	VectorSubtract (w->points[3], vup, w->points[3]);
	
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
	
	size = (int)((winding_t *)0)->points[w->numpoints];
	c = malloc (size);
	memcpy (c, w, size);
	return c;
}



/*
==================
ClipWinding

Clips the winding to the plane, returning the new winding on the positive side
Frees the input winding.
If keepon is true, an exactly on-plane winding will be saved, otherwise
it will be clipped away.
==================
*/
winding_t *ClipWinding (winding_t *in, dplane_t *split, qboolean keepon)
{
	vec_t	dists[MAX_POINTS_ON_WINDING];
	int		sides[MAX_POINTS_ON_WINDING];
	int		counts[3];
	vec_t	dot;
	int		i, j;
	vec_t	*p1, *p2;
	vec3_t	mid;
	winding_t	*neww;
	int		maxpts;
	
	counts[0] = counts[1] = counts[2] = 0;

	// determine sides for each point
	// do this exactly, with no epsilon so tiny portals still work
	for (i=0 ; i<in->numpoints ; i++)
	{
		dot = DotProduct (in->points[i], split->normal);
		dot -= split->dist;
		dists[i] = dot;
		if (dot > 0)
			sides[i] = SIDE_FRONT;
		else if (dot < 0)
			sides[i] = SIDE_BACK;
		else
			sides[i] = SIDE_ON;
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];
	
	if (keepon && !counts[0] && !counts[1])
		return in;
		
	if (!counts[0])
	{
		FreeWinding (in);
		return NULL;
	}
	if (!counts[1])
		return in;
	
	maxpts = in->numpoints+4;	// can't use counts[0]+2 because
								// of fp grouping errors
	neww = NewWinding (maxpts);
		
	for (i=0 ; i<in->numpoints ; i++)
	{
		p1 = in->points[i];
		
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
	
	if (neww->numpoints > maxpts)
		Error ("ClipWinding: points exceeded estimate");
		
// free the original winding
	FreeWinding (in);
	
	return neww;
}


/*
==================
DivideWinding

Divides a winding by a plane, producing one or two windings.  The
original winding is not damaged or freed.  If only on one side, the
returned winding will be the input winding.  If on both sides, two
new windings will be created.
==================
*/

//#define	DIVIDE_EPSILON	0.5
#define DIVIDE_EPSILON	ON_EPSILON


void	DivideWinding (winding_t *in, dplane_t *split, winding_t **front, winding_t **back)
{
	vec_t	dists[MAX_POINTS_ON_WINDING];
	int		sides[MAX_POINTS_ON_WINDING];
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
		dot = DotProduct (in->points[i], split->normal);
		dot -= split->dist;
		dists[i] = dot;
		if (dot > DIVIDE_EPSILON)
			sides[i] = SIDE_FRONT;
		else if (dot < -DIVIDE_EPSILON)
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

	*front = f = NewWinding (maxpts);
	*back = b = NewWinding (maxpts);
		
	for (i=0 ; i<in->numpoints ; i++)
	{
		p1 = in->points[i];
		
		if (sides[i] == SIDE_ON)
		{
			VectorCopy (p1, f->points[f->numpoints]);
			f->numpoints++;
			VectorCopy (p1, b->points[b->numpoints]);
			b->numpoints++;
			continue;
		}
	
		if (sides[i] == SIDE_FRONT)
		{
			VectorCopy (p1, f->points[f->numpoints]);
			f->numpoints++;
		}
		if (sides[i] == SIDE_BACK)
		{
			VectorCopy (p1, b->points[b->numpoints]);
			b->numpoints++;
		}

		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;
			
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
			
		VectorCopy (mid, f->points[f->numpoints]);
		f->numpoints++;
		VectorCopy (mid, b->points[b->numpoints]);
		b->numpoints++;
	}
	
	if (f->numpoints > maxpts || b->numpoints > maxpts)
		Error ("ClipWinding: points exceeded estimate");
}


//===========================================================================

/*
==================
NewFaceFromFace

Duplicates the non point information of a face, used by SplitFace and
MergeFace.
==================
*/
face_t *NewFaceFromFace (face_t *in)
{
	face_t	*newf;
	
	newf = AllocFace ();

	newf->planenum = in->planenum;
	newf->texturenum = in->texturenum;	
	newf->original = in->original;
	newf->contents = in->contents;
	
	return newf;
}



void SplitFaceTmp( face_t *in, dplane_t *split, face_t **front, face_t **back )
{
	vec_t	dists[MAXEDGES+1];
	int		sides[MAXEDGES+1];
	int		counts[3];
	vec_t	dot;
	int		i, j;
	face_t	*newf, *new2;
	vec_t	*p1, *p2;
	vec3_t	mid;
	
	if (in->numpoints < 0)
		Error ("SplitFace: freed face");
	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	for (i=0 ; i<in->numpoints ; i++)
	{
		dot = DotProduct (in->pts[i], split->normal);
		dot -= split->dist;
		dists[i] = dot;
		if (dot > ON_EPSILON)
			sides[i] = SIDE_FRONT;
		else if (dot < -ON_EPSILON)
			sides[i] = SIDE_BACK;
		else
			sides[i] = SIDE_ON;
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];
	
	if (!counts[0])
	{
		*front = NULL;
		*back = in;
		return;
	}
	if (!counts[1])
	{
		*front = in;
		*back = NULL;
		return;
	}
	
	*back = newf = NewFaceFromFace (in);
	*front = new2 = NewFaceFromFace (in);
	
// distribute the points and generate splits

	for (i=0 ; i<in->numpoints ; i++)
	{
		if (newf->numpoints > MAXEDGES || new2->numpoints > MAXEDGES)
			Error ("SplitFace: numpoints > MAXEDGES");

		p1 = in->pts[i];
		
		if (sides[i] == SIDE_ON)
		{
			VectorCopy (p1, newf->pts[newf->numpoints]);
			newf->numpoints++;
			VectorCopy (p1, new2->pts[new2->numpoints]);
			new2->numpoints++;
			continue;
		}
	
		if (sides[i] == SIDE_FRONT)
		{
			VectorCopy (p1, new2->pts[new2->numpoints]);
			new2->numpoints++;
		}
		else
		{
			VectorCopy (p1, newf->pts[newf->numpoints]);
			newf->numpoints++;
		}
		
		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;
			
	// generate a split point
		p2 = in->pts[(i+1)%in->numpoints];
		
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
	
		VectorCopy (mid, newf->pts[newf->numpoints]);
		newf->numpoints++;
		VectorCopy (mid, new2->pts[new2->numpoints]);
		new2->numpoints++;
	}

	if (newf->numpoints > MAXEDGES || new2->numpoints > MAXEDGES)
		Error ("SplitFace: numpoints > MAXEDGES");

#if 0	
CheckFace (newf);
CheckFace (new2);
#endif

}

/*
==================
SplitFace

==================
*/
void SplitFace (face_t *in, dplane_t *split, face_t **front, face_t **back)
{
	SplitFaceTmp( in, split, front, back );

	// free the original face now that is is represented by the fragments
	if ( *front && *back )
		FreeFace (in);
}

//===========================================================================

int			c_activefaces, c_peakfaces;
int			c_activesurfaces, c_peaksurfaces;
int			c_activewindings, c_peakwindings;
int			c_activeportals, c_peakportals;

void PrintMemory (void)
{
	printf ("faces   : %6i (%6i)\n", c_activefaces, c_peakfaces);
	printf ("surfaces: %6i (%6i)\n", c_activesurfaces, c_peaksurfaces);
	printf ("windings: %6i (%6i)\n", c_activewindings, c_peakwindings);
	printf ("portals : %6i (%6i)\n", c_activeportals, c_peakportals);
}

/*
==================
NewWinding
==================
*/
winding_t *NewWinding (int points)
{
	winding_t	*w;
	int			size;
	
	if (points > MAX_POINTS_ON_WINDING)
		Error ("NewWinding: %i points", points);
	
	c_activewindings++;
	if (c_activewindings > c_peakwindings)
		c_peakwindings = c_activewindings;

	size = (int)((winding_t *)0)->points[points];
	w = malloc (size);
	memset (w, 0, size);
	
	return w;
}


void FreeWinding (winding_t *w)
{
	c_activewindings--;
	free (w);
}



/*
===========
AllocFace
===========
*/
face_t *AllocFace (void)
{
	face_t	*f;
	
	c_activefaces++;
	if (c_activefaces > c_peakfaces)
		c_peakfaces = c_activefaces;
		
	f = malloc (sizeof(face_t));
	memset (f, 0, sizeof(face_t));
	f->planenum = -1;

	return f;
}


void FreeFace (face_t *f)
{
	c_activefaces--;
	free (f);
}


/*
===========
AllocSurface
===========
*/
surface_t *AllocSurface (void)
{
	surface_t	*s;
	
	s = malloc (sizeof(surface_t));
	memset (s, 0, sizeof(surface_t));
	
	c_activesurfaces++;
	if (c_activesurfaces > c_peaksurfaces)
		c_peaksurfaces = c_activesurfaces;
		
	return s;
}

void FreeSurface (surface_t *s)
{
	c_activesurfaces--;
	free (s);
}

/*
===========
AllocPortal
===========
*/
portal_t *AllocPortal (void)
{
	portal_t	*p;
	
	c_activeportals++;
	if (c_activeportals > c_peakportals)
		c_peakportals = c_activeportals;
	
	p = malloc (sizeof(portal_t));
	memset (p, 0, sizeof(portal_t));
	
	return p;
}

void FreePortal (portal_t *p)
{
	c_activeportals--;
	free (p);
}


/*
===========
AllocNode
===========
*/
node_t *AllocNode (void)
{
	node_t	*n;
	
	n = malloc (sizeof(node_t));
	memset (n, 0, sizeof(node_t));
	
	return n;
}


//===========================================================================

face_t	*validfaces[MAX_MAP_PLANES];



void AddFaceToBounds (face_t *f, vec3_t mins, vec3_t maxs)
{
	int		i;

	for (i=0 ; i<f->numpoints ; i++)
		AddPointToBounds (f->pts[i], mins, maxs);
}


/*
===============
SurflistFromValidFaces
===============
*/
surfchain_t *SurflistFromValidFaces (void)
{
	surface_t	*n;
	int			i;
	face_t		*f, *next;
	surfchain_t	*sc;

	sc = malloc(sizeof(*sc));
	ClearBounds (sc->mins, sc->maxs);
	sc->surfaces = NULL;	

	// grab planes from both sides
	for (i=0 ; i<numplanes ; i+=2)
	{
		if (!validfaces[i] && !validfaces[i+1])
			continue;
		n = AllocSurface ();
		n->next = sc->surfaces;
		sc->surfaces = n;
		ClearBounds (n->mins, n->maxs);
		n->planenum = i;

		n->faces = NULL;
		for (f = validfaces[i] ; f ; f=next)
		{
			next = f->next;
			f->next = n->faces;
			n->faces = f;
			AddFaceToBounds (f, n->mins, n->maxs);
		}
		for (f = validfaces[i+1] ; f ; f=next)
		{
			next = f->next;
			f->next = n->faces;
			n->faces = f;
			AddFaceToBounds (f, n->mins, n->maxs);
		}


		AddPointToBounds (n->mins, sc->mins, sc->maxs);
		AddPointToBounds (n->maxs, sc->mins, sc->maxs);

		validfaces[i] = NULL;
		validfaces[i+1] = NULL;
	}

	// merge all possible polygons

	MergeAll (sc->surfaces);

	return sc;
}

/*
===============
ReadSurfs
===============
*/
surfchain_t	*ReadSurfs (FILE *file)
{
	int		nump;
	int		r;
	int		planenum, texinfo, contents, numpoints;
	face_t	*f;
	int		i;
	double	v[3];

	// read in the polygons
	while (1)
	{
		r = fscanf (file, "%i %i %i %i\n", &planenum, &texinfo, &contents, &numpoints);
		if (r == 0 || r == -1)
			return NULL;
		if (planenum == -1)	// end of model
			break;
		if (r != 4)
			Error ("ReadSurfs: scanf failure");
		if (numpoints > MAXPOINTS)
			Error ("ReadSurfs: %i > MAXPOINTS", numpoints);
		if (planenum > numplanes)
			Error ("ReadSurfs: %i > numplanes", planenum);
		if (texinfo > numtexinfo)
			Error ("ReadSurfs: %i > numtexinfo", texinfo);


		f = AllocFace ();
		f->planenum = planenum;
		f->texturenum = texinfo;
		f->contents = contents;
		f->numpoints = numpoints;
		f->next = validfaces[planenum];
		validfaces[planenum] = f;

		for (i=0 ; i<f->numpoints ; i++)
		{
			r = fscanf (file, "%lf %lf %lf\n", &v[0], &v[1], &v[2]);
			VectorCopy (v, f->pts[i]);
		}
		fscanf (file, "\n");
	}

	return SurflistFromValidFaces ();
}


/*
===============
ProcessModel
===============
*/
qboolean ProcessModel (void)
{
	char	mod[80];
	surfchain_t	*surfs;
	node_t		*nodes;
	dmodel_t	*model;
	int			i;
	int			startleafs;

	surfs = ReadSurfs (polyfiles[0]);

	if (!surfs)
		return false;		// all models are done

	VectorCopy (surfs->mins, draw_mins);
	VectorCopy (surfs->maxs, draw_maxs);

	if (nummodels >= MAX_MAP_MODELS)
		Error ("nummodels == MAX_MAP_MODELS");

	startleafs = numleafs;
	model = &dmodels[nummodels];
	nummodels++;

	VectorCopy (surfs->mins, model->mins);
	VectorCopy (surfs->maxs, model->maxs);

//
// SolidBSP generates a node tree
//
	nodes = SolidBSP (surfs);
	
//
// build all the portals in the bsp tree
// some portals are solid polygons, and some are paths to other leafs
//
	if (nummodels == 1 && !nofill)	// assume non-world bmodels are simple
		nodes = FillOutside (nodes, true);	// make a leakfile if bad

	FreePortals (nodes);

	// fix tjunctions
	tjunc (nodes);

	MakeFaceEdges (nodes);

	// emit the faces for the bsp file
	model->headnode[0] = numnodes;
	model->firstface = numfaces;
	WriteDrawNodes (nodes);
	model->numfaces = numfaces - model->firstface;;
	model->visleafs = numleafs - startleafs;

	if (noclip)
		return true;

	//
	// the clipping hulls are simpler
	//
	for (hullnum = 1 ; hullnum < NUM_HULLS ; hullnum++)
	{
		surfs = ReadSurfs (polyfiles[hullnum]);
		nodes = SolidBSP (surfs);
		if (nummodels == 1 && !nofill)	// assume non-world bmodels are simple
			nodes = FillOutside (nodes, false);
		FreePortals (nodes);
		model->headnode[hullnum] = numclipnodes;
		WriteClipNodes (nodes);
	}

	return true;
}

/*
=================
ProcessFile

=================
*/
void ProcessFile (char *bspfilename)
{
	int		i;
	char	name[1024];

// create filenames
	sprintf (portfilename, "%s.prt", bspfilename);
	remove (portfilename);

	sprintf (pointfilename, "%s.pts", bspfilename);
	remove (pointfilename);

	// open the polygon files
	for (i=0 ; i<NUM_HULLS ; i++)
	{
		sprintf (name, "%s.p%i", bspfilename, i);
		polyfiles[i] = fopen (name, "r");
		if (!polyfiles[i])
			Error ("Can't open %s", name);
	}

	// load the output of qcsg
	strcat (bspfilename, ".bsp");
	LoadBSPFile (bspfilename);
	ParseEntities ();

	// init the tables to be shared by all models
	BeginBSPFile ();

	// process each model individually
	while (ProcessModel ())
		;

	// write the updated bsp file out
	FinishBSPFile ();
}


/*
==================
main

==================
*/
extern char qproject[];

int main (int argc, char **argv)
{
	int			i;
	double		start, end;

	printf( "qbsp2.exe v2.2 (%s)\n", __DATE__ );
	printf ("---- qbsp2 ----\n" );

//
// check command line flags
//
	for (i=1 ; i<argc ; i++)
	{
		if (argv[i][0] != '-')
			break;
		if (!strcmp(argv[i],"-threads"))
		{
			numthreads = atoi (argv[i+1]);
			i++;
		}
		else if (!strcmp(argv[i], "-v"))
		{
			printf ("verbose = true\n");
			verbose = true;
		}
		else if (!strcmp (argv[i],"-notjunc"))
			notjunc = true;
		else if (!strcmp (argv[i],"-draw"))
			drawflag = true;
		else if (!strcmp (argv[i],"-watervis"))
			watervis = true;
		else if (!strcmp (argv[i],"-noclip"))
			noclip = true;
		else if (!strcmp (argv[i],"-nofill"))
			nofill = true;
		else if (!strcmp (argv[i],"-verbose"))
			allverbose = true;
		else if (!strcmp (argv[i],"-leakonly"))
			leakonly = true;
		else if (!strcmp (argv[i],"-nogfx"))
			nogfx = true;
		else if( !strcmp( argv[ i ], "-proj" ) )
		{
			strcpy( qproject, argv[ i + 1 ] );
			i++;
		}
		else if (!strcmp (argv[i],"-subdivide"))
		{
			subdivide_size = atoi(argv[i+1]);
			i++;
		}
		else
			Error ("qbsp: Unknown option '%s'", argv[i]);
	}
	
	if (i != argc - 2 && i != argc - 1)
		Error ("usage: qbsp [-draw] [-leakonly] [-noclip] [-nofill] [-nogfx] [-notjunc] [-proj name] [-subdivide size] [-threads n] [-v] [-watervis] sourcefile");

	ThreadSetDefault ();

	SetQdirFromPath (argv[i]);	
	strcpy (bspfilename, ExpandArg(argv[i]));
	StripExtension (bspfilename);

//
// do it!
//
	start = I_FloatTime ();

	ProcessFile (bspfilename);

	end = I_FloatTime ();
	printf ("%5.0f seconds elapsed\n", end-start);

	return 0;
}
