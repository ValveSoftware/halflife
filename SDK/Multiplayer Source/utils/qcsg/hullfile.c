/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#include "csg.h"

extern vec3_t	hull_size[NUM_HULLS][2];
/*
============
CheckHullFile
============
*/
void CheckHullFile( qboolean hullfile, char *filename )
{
	FILE *f;
	char scan[ 128 ];
	vec3_t	new_hulls[NUM_HULLS][2];
	qboolean read_error = false;
	int i;

	if ( !hullfile )
		return;

	// Open up hull file
	f = fopen (filename, "r");
	if ( !f )
	{
		printf ("WARNING: Couldn't open hullfile %s, using default hulls", filename );
		return;
	}
	else
	{
		printf("[Reading hulls from '%s']\n", filename);
	}

	for ( i = 0 ; i < NUM_HULLS; i++ )
	{
		float x1, y1, z1, x2, y2, z2;

		vec3_t mins, maxs;
		int	argCnt;

		if ( !fgets(scan, sizeof(scan), f ) )
		{
			printf ("WARNING: Error parsing %s, couln't read hull line %i, using default hulls", filename, i );
			read_error = true;
			break;
		}

		argCnt = sscanf (scan, "( %f %f %f ) ( %f %f %f ) ", &x1, &y1, &z1, &x2, &y2, &z2 );
		if ( argCnt != 6 )
		{
			printf ("WARNING: Error parsing %s, expeciting '( x y z ) ( x y z )' using default hulls", filename );
			read_error = true;
			break;
		}
		else
		{
			mins[0] = x1;
			mins[1] = y1;
			mins[2] = z1;
			maxs[0] = x2;
			maxs[1] = y2;
			maxs[2] = z2;
		}

		VectorCopy( mins, new_hulls[ i ][ 0 ] );
		VectorCopy( maxs, new_hulls[ i ][ 1 ] );
	}

	if ( read_error )
	{
		printf ("WARNING: Error parsing %s, using default hulls", filename );
	}
	else
	{
		memcpy( hull_size, new_hulls, 2 * NUM_HULLS * sizeof( vec3_t ) );
	}

	fclose( f );
}