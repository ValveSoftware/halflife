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
#include "scriplib.h"
#include "polylib.h"
#include "threads.h"
#include "bspfile.h"

#include <windows.h>

#ifndef DOUBLEVEC_T
#error you must add -dDOUBLEVEC_T to the project!
#endif

#define	BOGUS_RANGE	8192

typedef struct
{
	vec3_t	normal;
	vec_t	dist;
	int		type;
	int		iorigin[3];
	int		inormal[3];
} plane_t;


extern	plane_t	mapplanes[MAX_MAP_PLANES];
extern	int		nummapplanes;

extern int g_nMapFileVersion;	// map file version * 100 (ie 201), zero for pre-Worldcraft 2.0.1 maps

typedef struct
{
	vec3_t	UAxis;
	vec3_t	VAxis;
	vec_t	shift[2];
	vec_t	rotate;
	vec_t	scale[2];
	char	name[32];
} brush_texture_t;

typedef struct side_s
{
	brush_texture_t	td;
	int		planepts[3][3];
} side_t;

typedef struct bface_s
{
	struct		bface_s	*next;
	int			planenum;
	plane_t		*plane;
	winding_t	*w;
	int			texinfo;
	qboolean	used;		// just for face counting
	int			contents, backcontents;
	vec3_t		mins, maxs;
} bface_t;

#define	NUM_HULLS	4	// no larger than MAX_MAP_HULLS
typedef struct
{
	vec3_t	mins, maxs;
	bface_t	*faces;
} brushhull_t;

typedef struct brush_s
{
	int		entitynum;
	int		brushnum;

	int		firstside;
	int		numsides;

	int		contents;
	brushhull_t	hulls[NUM_HULLS];
} brush_t;


extern	int			nummapbrushes;
extern	brush_t		mapbrushes[MAX_MAP_BRUSHES];

#define		MAX_MAP_SIDES	(MAX_MAP_BRUSHES*6)

extern	int			numbrushplanes;
extern	plane_t		planes[MAX_MAP_PLANES];

extern	int			numbrushsides;
extern	side_t	brushsides[MAX_MAP_SIDES];

extern	qboolean	noclip;
extern	qboolean	wadtextures;

int			nWadInclude;
char		*pszWadInclude[];

void 	LoadMapFile (char *filename);

//=============================================================================

// textures.c

extern int nummiptex;
void WriteMiptex (void);
int TexinfoForBrushTexture (plane_t *plane, brush_texture_t *bt, vec3_t origin);

//=============================================================================

// brush.c

void FindGCD (int *v);

brush_t *Brush_LoadEntity (entity_t *ent, int hullnum);
int	PlaneTypeForNormal (vec3_t normal);

void CreateBrush (int brushnum);

//=============================================================================

// csg.c

bface_t *NewFaceFromFace (bface_t *in);
extern qboolean	onlyents;

//=============================================================================

// draw.c

extern vec3_t	draw_mins, draw_maxs;
extern	qboolean	drawflag;

void Draw_ClearWindow (void);
void DrawWinding (winding_t *w);

