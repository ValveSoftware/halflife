/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

// vis.c

#include "vis.h"
#include "threads.h"

#define	MAX_THREADS		4

int			numportals;
int			portalleafs;

portal_t	*portals;
leaf_t		*leafs;

int			c_portaltest, c_portalpass, c_portalcheck;


qboolean		showgetleaf = true;

int		leafon;			// the next leaf to be given to a thread to process

byte	*vismap, *vismap_p, *vismap_end;	// past visfile
int		originalvismapsize;

byte	*uncompressed;			// [bitbytes*portalleafs]

int		bitbytes;				// (portalleafs+63)>>3
int		bitlongs;

qboolean		fastvis;
qboolean		verbose;

//=============================================================================

void PlaneFromWinding (winding_t *w, plane_t *plane)
{
	vec3_t		v1, v2;

// calc plane
	VectorSubtract (w->points[2], w->points[1], v1);
	VectorSubtract (w->points[0], w->points[1], v2);
	CrossProduct (v2, v1, plane->normal);
	VectorNormalize (plane->normal);
	plane->dist = DotProduct (w->points[0], plane->normal);
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
	
	size = (int)((winding_t *)0)->points[points];
	w = malloc (size);
	memset (w, 0, size);
	
	return w;
}



void pw(winding_t *w)
{
	int		i;
	for (i=0 ; i<w->numpoints ; i++)
		printf ("(%5.1f, %5.1f, %5.1f)\n",w->points[i][0], w->points[i][1],w->points[i][2]);
}

void prl(leaf_t *l)
{
	int			i;
	portal_t	*p;
	plane_t		pl;
	
	for (i=0 ; i<l->numportals ; i++)
	{
		p = l->portals[i];
		pl = p->plane;
		printf ("portal %4i to leaf %4i : %7.1f : (%4.1f, %4.1f, %4.1f)\n",(int)(p-portals),p->leaf,pl.dist, pl.normal[0], pl.normal[1], pl.normal[2]);
	}
}


//=============================================================================

/*
=============
GetNextPortal

Returns the next portal for a thread to work on
Returns the portals from the least complex, so the later ones can reuse
the earlier information.
=============
*/
portal_t *GetNextPortal (void)
{
	int		j;
	portal_t	*p, *tp;
	int		min;
	int		i;

	i = GetThreadWork ();	// bump the pacifier
	if (i == -1)
		return NULL;

	ThreadLock();

	min = 99999;
	p = NULL;
	
	for (j=0, tp = portals ; j<numportals*2 ; j++, tp++)
	{
		if (tp->nummightsee < min && tp->status == stat_none)
		{
			min = tp->nummightsee;
			p = tp;
		}
	}

	if (p)
		p->status = stat_working;

	ThreadUnlock();

	return p;
}

/*
==============
LeafThread
==============
*/
void LeafThread (int thread)
{
	portal_t	*p;
		
	do
	{
		p = GetNextPortal ();
		if (!p)
			break;
			
		PortalFlow (p);
		
		qprintf ("portal:%4i  mightsee:%4i  cansee:%4i\n", (int)(p - portals), p->nummightsee, p->numcansee);
	} while (1);
}

/*
===============
CompressRow

===============
*/
int CompressRow (byte *vis, byte *dest)
{
	int		j;
	int		rep;
	int		visrow;
	byte	*dest_p;
	
	dest_p = dest;
	visrow = (portalleafs + 7)>>3;
	
	for (j=0 ; j<visrow ; j++)
	{
		*dest_p++ = vis[j];
		if (vis[j])
			continue;

		rep = 1;
		for ( j++; j<visrow ; j++)
			if (vis[j] || rep == 255)
				break;
			else
				rep++;
		*dest_p++ = rep;
		j--;
	}
	
	return dest_p - dest;
}


/*
===============
LeafFlow

Builds the entire visibility list for a leaf
===============
*/
int		totalvis;

void LeafFlow (int leafnum)
{
	leaf_t		*leaf;
	byte		*outbuffer;
	byte		compressed[MAX_MAP_LEAFS/8];
	int			i, j;
	int			numvis;
	byte		*dest;
	portal_t	*p;
	
//
// flow through all portals, collecting visible bits
//
	outbuffer = uncompressed + leafnum*bitbytes;
	leaf = &leafs[leafnum];
	for (i=0 ; i<leaf->numportals ; i++)
	{
		p = leaf->portals[i];
		if (p->status != stat_done)
			Error ("portal not done");
		for (j=0 ; j<bitbytes ; j++)
			outbuffer[j] |= p->visbits[j];
	}

	if (outbuffer[leafnum>>3] & (1<<(leafnum&7)))
		printf ("WARNING: Leaf portals saw into leaf");
		
	outbuffer[leafnum>>3] |= (1<<(leafnum&7));

	numvis = 0;
	for (i=0 ; i<portalleafs ; i++)
		if (outbuffer[i>>3] & (1<<(i&3)))
			numvis++;
			
//
// compress the bit string
//
	qprintf ("leaf %4i : %4i visible\n", leafnum, numvis);
	totalvis += numvis;

#if 0	
	i = (portalleafs+7)>>3;
	memcpy (compressed, outbuffer, i);
#else
	i = CompressRow (outbuffer, compressed);
#endif

	dest = vismap_p;
	vismap_p += i;
	
	if (vismap_p > vismap_end)
		Error ("Vismap expansion overflow");

	dleafs[leafnum+1].visofs = dest-vismap;	// leaf 0 is a common solid

	memcpy (dest, compressed, i);	
}


/*
==================
CalcPortalVis
==================
*/
void CalcPortalVis (void)
{
	int		i;

// fastvis just uses mightsee for a very loose bound
	if (fastvis)
	{
		for (i=0 ; i<numportals*2 ; i++)
		{
			portals[i].visbits = portals[i].mightsee;
			portals[i].status = stat_done;
		}
		return;
	}
	
	leafon = 0;
	
	RunThreadsOn (numportals*2, true, LeafThread);

	qprintf ("portalcheck: %i  portaltest: %i  portalpass: %i\n",c_portalcheck, c_portaltest, c_portalpass);
	qprintf ("c_vistest: %i  c_mighttest: %i\n",c_vistest, c_mighttest);
}


/*
==================
CalcVis
==================
*/
void CalcVis (void)
{
	int		i;
	
	RunThreadsOn (numportals*2, true, BasePortalVis);
	
	CalcPortalVis ();

//
// assemble the leaf vis lists by oring and compressing the portal lists
//
	for (i=0 ; i<portalleafs ; i++)
		LeafFlow (i);
		
	printf ("average leafs visible: %i\n", totalvis / portalleafs);
}


/*
============
LoadPortals
============
*/
void LoadPortals (char *name)
{
	int			i, j;
	portal_t	*p;
	leaf_t		*l;
	char		magic[80];
	FILE		*f;
	int			numpoints;
	winding_t	*w;
	int			leafnums[2];
	plane_t		plane;
	
	if (!strcmp(name,"-"))
		f = stdin;
	else
	{
		f = fopen(name, "r");
		if (!f)
		{
			printf ("LoadPortals: couldn't read %s\n",name);
			printf ("No vising performed.\n");
			exit (1);
		}
	}

	if (fscanf (f,"%79s\n%i\n%i\n",magic, &portalleafs, &numportals) != 3)
		Error ("LoadPortals: failed to read header");
	if (strcmp(magic,PORTALFILE))
		Error ("LoadPortals: not a portal file");

	printf ("%4i portalleafs\n", portalleafs);
	printf ("%4i numportals\n", numportals);

	bitbytes = ((portalleafs+63)&~63)>>3;
	bitlongs = bitbytes/sizeof(long);
	
// each file portal is split into two memory portals
	portals = malloc(2*numportals*sizeof(portal_t));
	memset (portals, 0, 2*numportals*sizeof(portal_t));
	
	leafs = malloc(portalleafs*sizeof(leaf_t));
	memset (leafs, 0, portalleafs*sizeof(leaf_t));

	originalvismapsize = portalleafs*((portalleafs+7)/8);

	vismap = vismap_p = dvisdata;
	vismap_end = vismap + MAX_MAP_VISIBILITY;
		
	for (i=0, p=portals ; i<numportals ; i++)
	{
		if (fscanf (f, "%i %i %i ", &numpoints, &leafnums[0], &leafnums[1])
			!= 3)
			Error ("LoadPortals: reading portal %i", i);
		if (numpoints > MAX_POINTS_ON_WINDING)
			Error ("LoadPortals: portal %i has too many points", i);
		if ( (unsigned)leafnums[0] > portalleafs
		|| (unsigned)leafnums[1] > portalleafs)
			Error ("LoadPortals: reading portal %i", i);
		
		w = p->winding = NewWinding (numpoints);
		w->original = true;
		w->numpoints = numpoints;
		
		for (j=0 ; j<numpoints ; j++)
		{
			double	v[3];
			int		k;

			// scanf into double, then assign to vec_t
			if (fscanf (f, "(%lf %lf %lf ) "
			, &v[0], &v[1], &v[2]) != 3)
				Error ("LoadPortals: reading portal %i", i);
			for (k=0 ; k<3 ; k++)
				w->points[j][k] = v[k];
		}
		fscanf (f, "\n");
		
	// calc plane
		PlaneFromWinding (w, &plane);

	// create forward portal
		l = &leafs[leafnums[0]];
		if (l->numportals == MAX_PORTALS_ON_LEAF)
			Error ("Leaf with too many portals");
		l->portals[l->numportals] = p;
		l->numportals++;
		
		p->winding = w;
		VectorSubtract (vec3_origin, plane.normal, p->plane.normal);
		p->plane.dist = -plane.dist;
		p->leaf = leafnums[1];
		p++;
		
	// create backwards portal
		l = &leafs[leafnums[1]];
		if (l->numportals == MAX_PORTALS_ON_LEAF)
			Error ("Leaf with too many portals");
		l->portals[l->numportals] = p;
		l->numportals++;
		
		p->winding = NewWinding(w->numpoints);
		p->winding->numpoints = w->numpoints;
		for (j=0 ; j<w->numpoints ; j++)
		{
			VectorCopy (w->points[w->numpoints-1-j], p->winding->points[j]);
		}

		p->plane = plane;
		p->leaf = leafnums[0];
		p++;

	}
	
	fclose (f);
}


/*
===========
main
===========
*/
int main (int argc, char **argv)
{
	char	portalfile[1024];
	char		source[1024];
	int		i;
	double		start, end;
		
	printf ("vis.exe v1.3 (%s)\n", __DATE__);
	printf ("---- vis ----\n");

	verbose = false;
	for (i=1 ; i<argc ; i++)
	{
		if (!strcmp(argv[i],"-threads"))
		{
			numthreads = atoi (argv[i+1]);
			i++;
		}
		else if (!strcmp(argv[i], "-fast"))
		{
			printf ("fastvis = true\n");
			fastvis = true;
		}
		else if (!strcmp(argv[i], "-v"))
		{
			printf ("verbose = true\n");
			verbose = true;
		}
		else if (argv[i][0] == '-')
			Error ("Unknown option \"%s\"", argv[i]);
		else
			break;
	}

	if (i != argc - 1)
		Error ("usage: vis [-threads #] [-level 0-4] [-fast] [-v] bspfile");

	start = I_FloatTime ();
	
	ThreadSetDefault ();

	printf ("%i thread(s)\n", numthreads);

	strcpy (source, argv[i]);
	StripExtension (source);
	DefaultExtension (source, ".bsp");

	LoadBSPFile (source);
	
	strcpy (portalfile, argv[i]);
	StripExtension (portalfile);
	strcat (portalfile, ".prt");
	
	LoadPortals (portalfile);
	
	uncompressed = malloc(bitbytes*portalleafs);
	memset (uncompressed, 0, bitbytes*portalleafs);

	CalcVis ();

	qprintf ("c_chains: %i\n",c_chains);
	
	visdatasize = vismap_p - dvisdata;	
	printf ("visdatasize:%i  compressed from %i\n", visdatasize, originalvismapsize);
	
	CalcAmbientSounds ();

	WriteBSPFile (source);	
	
//	unlink (portalfile);

	end = I_FloatTime ();
	printf ("%5.1f seconds elapsed\n", end-start);
	
	free(uncompressed);

	return 0;
}

