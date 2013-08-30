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
#include "bsplib.h"

void main (int argc, char **argv)
{
	int			i;
	char		source[1024];
	int			size;
	FILE		*f;

	printf( "bspinfo.exe v2.1 (%s)\n", __DATE__ );
	printf ("---- bspinfo ----\n" );


	if (argc == 1)
		Error ("usage: bspinfo bspfile [bspfiles]");
		
	for (i=1 ; i<argc ; i++)
	{
		printf ("---------------------\n");
		strcpy (source, argv[i]);
		DefaultExtension (source, ".bsp");
		f = fopen (source, "rb");
		if (f)
		{
			size = filelength (f);
			fclose (f);
		}
		else
			size = 0;
		printf ("%s: %i\n", source, size);
		
		LoadBSPFile (source);		
		PrintBSPFileSizes ();
		printf ("---------------------\n");
	}
}
