/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

//
// trilib.h: header file for loading triangles from an Alias triangle file
//
#define MAXTRIANGLES	2048

typedef struct {
	vec3_t	verts[3];
} triangle_t;

void LoadTriangleList (char *filename, triangle_t **pptri, int *numtriangles);

