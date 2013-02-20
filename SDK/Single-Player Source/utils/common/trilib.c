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
// trilib.c: library for loading triangles from an Alias triangle file
//

#include <stdio.h>
#include "cmdlib.h"
#include "mathlib.h"
#include "trilib.h"

// on disk representation of a face


#define	FLOAT_START	99999.0
#define	FLOAT_END	-FLOAT_START
#define MAGIC       123322

//#define NOISY 1

typedef struct {
	float v[3];
} vector;

typedef struct
{
	vector n;    /* normal */
	vector p;    /* point */
	vector c;    /* color */
	float  u;    /* u */
	float  v;    /* v */
} aliaspoint_t;

typedef struct {
	aliaspoint_t	pt[3];
} tf_triangle;


void ByteSwapTri (tf_triangle *tri)
{
	int		i;
	
	for (i=0 ; i<sizeof(tf_triangle)/4 ; i++)
	{
		((int *)tri)[i] = BigLong (((int *)tri)[i]);
	}
}

void LoadTriangleList (char *filename, triangle_t **pptri, int *numtriangles)
{
	FILE        *input;
	float       start;
	char        name[256], tex[256];
	int         i, count, magic;
	tf_triangle	tri;
	triangle_t	*ptri;
	int			iLevel;
	int			exitpattern;
	float		t;


	t = -FLOAT_START;
	*((unsigned char *)&exitpattern + 0) = *((unsigned char *)&t + 3);
	*((unsigned char *)&exitpattern + 1) = *((unsigned char *)&t + 2);
	*((unsigned char *)&exitpattern + 2) = *((unsigned char *)&t + 1);
	*((unsigned char *)&exitpattern + 3) = *((unsigned char *)&t + 0);

	if ((input = fopen(filename, "rb")) == 0) {
		fprintf(stderr,"reader: could not open file '%s'\n", filename);
		exit(0);
	}

	iLevel = 0;

	fread(&magic, sizeof(int), 1, input);
	if (BigLong(magic) != MAGIC) {
		fprintf(stderr,"File is not a Alias object separated triangle file, magic number is wrong.\n");
		exit(0);
	}

	ptri = malloc (MAXTRIANGLES * sizeof(triangle_t));

	*pptri = ptri;

	while (feof(input) == 0) {
		fread(&start,  sizeof(float), 1, input);
		*(int *)&start = BigLong(*(int *)&start);
		if (*(int *)&start != exitpattern)
		{
			if (start == FLOAT_START) {
				/* Start of an object or group of objects. */
				i = -1;
				do {
					/* There are probably better ways to read a string from */
					/* a file, but this does allow you to do error checking */
					/* (which I'm not doing) on a per character basis.      */
					++i;
					fread( &(name[i]), sizeof( char ), 1, input);
				} while( name[i] != '\0' );
	
	//			indent();
	//			fprintf(stdout,"OBJECT START: %s\n",name);
				fread( &count, sizeof(int), 1, input);
				count = BigLong(count);
				++iLevel;
				if (count != 0) {
	//				indent();
	
	//				fprintf(stdout,"NUMBER OF TRIANGLES: %d\n",count);
	
					i = -1;
					do {
						++i;
						fread( &(tex[i]), sizeof( char ), 1, input);
					} while( tex[i] != '\0' );
	
	//				indent();
	//				fprintf(stdout,"  Object texture name: '%s'\n",tex);
				}
	
				/* Else (count == 0) this is the start of a group, and */
				/* no texture name is present. */
			}
			else if (start == FLOAT_END) {
				/* End of an object or group. Yes, the name should be */
				/* obvious from context, but it is in here just to be */
				/* safe and to provide a little extra information for */
				/* those who do not wish to write a recursive reader. */
				/* Mia culpa. */
				--iLevel;
				i = -1;
				do {
					++i;
					fread( &(name[i]), sizeof( char ), 1, input);
				} while( name[i] != '\0' );
	
	//			indent();
	//			fprintf(stdout,"OBJECT END: %s\n",name);
				continue;
			}
		}

//
// read the triangles
//		
		for (i = 0; i < count; ++i) {
			int		j;

			fread( &tri, sizeof(tf_triangle), 1, input );
			ByteSwapTri (&tri);
			for (j=0 ; j<3 ; j++)
			{
				int		k;

				for (k=0 ; k<3 ; k++)
				{
					ptri->verts[j][k] = tri.pt[j].p.v[k];
				}
			}

			ptri++;

			if ((ptri - *pptri) >= MAXTRIANGLES)
				Error ("Error: too many triangles; increase MAXTRIANGLES\n");
		}
	}

	*numtriangles = ptri - *pptri;

	fclose (input);
}

