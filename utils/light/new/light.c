// lighting.c

#include "light.h"

/*

NOTES
-----

*/

float		scaledist = 1.0;
float		scalecos = 0.5;
float		rangescale = 0.5;

byte		*filebase, *file_p, *file_end;

dmodel_t	*bspmodel;

vec3_t	bsp_origin;

qboolean	extrasamples;

float		minlights[MAX_MAP_FACES];




lightentity_t	lightentities[MAX_MAP_ENTITIES];
int		numlightentities;


/*
==================
LoadEntities
==================
*/
void LoadEntities (void)
{
	char 		*s, *s2;
	entity_t	*e;
	lightentity_t	*le;
	int			i, j;

	ParseEntities ();
	
// go through all the entities
	for (i=1 ; i<num_entities ; i++)
	{
		e = &entities[i];

		s = ValueForKey (e, "classname");
		if (strncmp (s, "light", 5))
			continue;

		le = &lightentities[numlightentities];
		numlightentities++;

		strcpy (le->classname, s);
		le->light = FloatForKey (e, "light");
		if (!le->light)
			le->light = DEFAULTLIGHTLEVEL;
		le->style = FloatForKey (e, "style");
		le->angle = FloatForKey (e, "angle");
		GetVectorForKey (e, "origin", le->origin);

		s = ValueForKey (e, "target");
		if (!s[0])
			continue;

		// find matching targetname
		for (j=1 ; j<num_entities ; j++)
		{
			s2 = ValueForKey (&entities[j], "targetname");
			if (!strcmp (s, s2))
			{
				le->targetent = true;
				GetVectorForKey (&entities[j], "origin", le->targetorigin);
				break;
			}
		}
		if (j == num_entities)
			printf ("WARNING: entity %i has unmatched target %s\n", i, s);
	}

	qprintf ("%d lightentities\n", numlightentities);

}


byte *GetFileSpace (int size)
{
	byte	*buf;
	
	ThreadLock();
	file_p = (byte *)(((long)file_p + 3)&~3);
	buf = file_p;
	file_p += size;
	ThreadUnlock();
	if (file_p > file_end)
		Error ("GetFileSpace: overrun");
	return buf;
}



/*
=============
LightWorld
=============
*/
void LightWorld (void)
{
	filebase = file_p = dlightdata;
	file_end = filebase + MAX_MAP_LIGHTING;

	RunThreadsOnIndividual (numfaces, true, LightFace);

	lightdatasize = file_p - filebase;
	
	printf ("lightdatasize: %i\n", lightdatasize);
}


/*
========
main

light modelfile
========
*/
int main (int argc, char **argv)
{
	int		i;
	double		start, end;
	char		source[1024];

	printf ("----- LightFaces ----\n");

	for (i=1 ; i<argc ; i++)
	{
		if (!strcmp(argv[i],"-threads"))
		{
			numthreads = atoi (argv[i+1]);
			i++;
		}
		else if (!strcmp(argv[i],"-extra"))
		{
			extrasamples = true;
			printf ("extra sampling enabled\n");
		}
		else if (!strcmp(argv[i],"-dist"))
		{
			scaledist = atof (argv[i+1]);
			i++;
		}
		else if (!strcmp(argv[i],"-range"))
		{
			rangescale = atof (argv[i+1]);
			i++;
		}
		else if (argv[i][0] == '-')
			Error ("Unknown option \"%s\"", argv[i]);
		else
			break;
	}

	if (i != argc - 1)
		Error ("usage: light [-threads num] [-extra] bspfile");

	ThreadSetDefault ();

	start = I_FloatTime ();

	strcpy (source, argv[i]);
	StripExtension (source);
	DefaultExtension (source, ".bsp");
	
	LoadBSPFile (source);
	LoadEntities ();
		
	MakeTnodes (&dmodels[0]);

	LightWorld ();

	WriteBSPFile (source);

	end = I_FloatTime ();
	printf ("%5.1f seconds elapsed\n", end-start);
	
	return 0;
}

