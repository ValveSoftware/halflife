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

int			nummapbrushes;
brush_t		mapbrushes[MAX_MAP_BRUSHES];

int			numbrushsides;
side_t		brushsides[MAX_MAP_SIDES];

int g_nMapFileVersion;


/*
==================
textureAxisFromPlane
==================
*/
vec3_t	baseaxis[18] =
{
	{0,0,1}, {1,0,0}, {0,-1,0},			// floor
	{0,0,-1}, {1,0,0}, {0,-1,0},		// ceiling
	{1,0,0}, {0,1,0}, {0,0,-1},			// west wall
	{-1,0,0}, {0,1,0}, {0,0,-1},		// east wall
	{0,1,0}, {1,0,0}, {0,0,-1},			// south wall
	{0,-1,0}, {1,0,0}, {0,0,-1}			// north wall
};

void TextureAxisFromPlane(plane_t *pln, vec3_t xv, vec3_t yv)
{
	int		bestaxis;
	vec_t	dot,best;
	int		i;
	
	best = 0;
	bestaxis = 0;
	
	for (i=0 ; i<6 ; i++)
	{
		dot = DotProduct (pln->normal, baseaxis[i*3]);
		if (dot > best)
		{
			best = dot;
			bestaxis = i;
		}
	}
	
	VectorCopy (baseaxis[bestaxis*3+1], xv);
	VectorCopy (baseaxis[bestaxis*3+2], yv);
}


/*
=================
ParseBrush
=================
*/
void ParseBrush (entity_t *mapent)
{
	brush_t		*b;
	bface_t		*f, *f2;
	int			planepts[3][3];
	vec3_t			t1, t2, t3;
	int			i,j;
	vec_t		d;
	int			planenum;
	side_t		*side;
	int			contents;

	if (nummapbrushes == MAX_MAP_BRUSHES)
		Error ("nummapbrushes == MAX_MAP_BRUSHES");

	b = &mapbrushes[nummapbrushes];
	nummapbrushes++;
	b->firstside = numbrushsides;
	b->entitynum = num_entities-1;
	b->brushnum = nummapbrushes - mapent->firstbrush - 1;

	mapent->numbrushes++;
		
	do
	{
		if (!GetToken (true))
			break;
		if (!strcmp (token, "}") )
			break;
		
		if (numbrushsides == MAX_MAP_SIDES)
			Error ("numbrushsides == MAX_MAP_SIDES");
		side = &brushsides[numbrushsides];
		numbrushsides++;

		b->numsides++;

		// read the three point plane definition
		for (i=0 ; i<3 ; i++)
		{
			if (i != 0)
				GetToken (true);
			if (strcmp (token, "(") )
				Error ("parsing brush");
			
			for (j=0 ; j<3 ; j++)
			{
				GetToken (false);
				side->planepts[i][j] = atoi(token);
			}
			
			GetToken (false);
			if (strcmp (token, ")") )
				Error ("parsing brush");
				
		}

		// read the texturedef
		GetToken (false);
		strcpy (side->td.name, token);

		if (g_nMapFileVersion < 220)
		{
			GetToken (false);
			side->td.shift[0] = atof(token);
			GetToken (false);
			side->td.shift[1] = atof(token);
			GetToken (false);
			side->td.rotate = atof(token);
		}
		else
		{
			// texture U axis
			GetToken (false);
			if (strcmp (token, "["))
			{
				Error("missing '[ in texturedef");
			}

			GetToken (false);
			side->td.UAxis[0] = atof(token);
			GetToken (false);
			side->td.UAxis[1] = atof(token);
			GetToken (false);
			side->td.UAxis[2] = atof(token);
			GetToken (false);
			side->td.shift[0] = atof(token);

			GetToken (false);
			if (strcmp (token, "]"))
			{
				Error("missing ']' in texturedef");
			}

			// texture V axis
			GetToken (false);
			if (strcmp (token, "["))
			{
				Error("missing '[ in texturedef");
			}

			GetToken (false);
			side->td.VAxis[0] = atof(token);
			GetToken (false);
			side->td.VAxis[1] = atof(token);
			GetToken (false);
			side->td.VAxis[2] = atof(token);
			GetToken (false);
			side->td.shift[1] = atof(token);

			GetToken (false);
			if (strcmp (token, "]"))
			{
				Error("missing ']' in texturedef");
			}

			// Texture rotation is implicit in U/V axes.
			GetToken(false);
			side->td.rotate = 0;
		}

		// texure scale
		GetToken (false);
		side->td.scale[0] = atof(token);
		GetToken (false);
		side->td.scale[1] = atof(token);

	} while (1);

	b->contents = contents = BrushContents (b);

	//
	// origin brushes are removed, but they set
	// the rotation origin for the rest of the brushes
	// in the entity
	//
	if (contents == CONTENTS_ORIGIN)
	{
		char	string[32];
		vec3_t	origin;

		b->contents = CONTENTS_SOLID;
		CreateBrush (mapent->firstbrush + b->brushnum);	// to get sizes
		b->contents = contents;

		for (i = 0; i < NUM_HULLS; i++) {
			b->hulls[i].faces = NULL;
		}

		if (b->entitynum == 0)
		{
			printf ("Entity %i, Brush %i: origin brushes not allowed in world"
				, b->entitynum, b->brushnum);
			return;
		}
		VectorAdd (b->hulls[0].mins, b->hulls[0].maxs, origin);
		VectorScale (origin, 0.5, origin);

		sprintf (string, "%i %i %i", (int)origin[0], (int)origin[1], (int)origin[2]);
		SetKeyValue (&entities[b->entitynum], "origin", string);
	}

}

/*
================
ParseMapEntity
================
*/
qboolean	ParseMapEntity (void)
{
	entity_t	*mapent;
	epair_t		*e;

	if (!GetToken (true))
		return false;

	if (strcmp (token, "{") )
		Error ("ParseEntity: { not found");
	
	if (num_entities == MAX_MAP_ENTITIES)
		Error ("num_entities == MAX_MAP_ENTITIES");

	mapent = &entities[num_entities];
	num_entities++;
	mapent->firstbrush = nummapbrushes;
	mapent->numbrushes = 0;

	do
	{
		if (!GetToken (true))
			Error ("ParseEntity: EOF without closing brace");
		if (!strcmp (token, "}") )
			break;
		if (!strcmp (token, "{") )
			ParseBrush (mapent);
		else
		{
			e = ParseEpair ();

			if (!strcmp(e->key, "mapversion"))
			{
				g_nMapFileVersion = atoi(e->value);
			}

			e->next = mapent->epairs;
			mapent->epairs = e;
		}
	} while (1);

	if ( mapent->numbrushes == 1 && mapbrushes[mapent->firstbrush].contents == CONTENTS_ORIGIN )
	{
		brushhull_t	*hull = mapbrushes[mapent->firstbrush].hulls;
		Error("Found an entity with ONLY an origin brush near(%.0f,%.0f,%.0f)!\n",
			 hull->mins[0], hull->mins[1], hull->mins[2] );
	}

	GetVectorForKey (mapent, "origin", mapent->origin);

	// group entities are just for editor convenience
	// toss all brushes into the world entity
	if ( !onlyents && !strcmp ("func_group", ValueForKey (mapent, "classname")))
	{
		// this is pretty gross, because the brushes are expected to be
		// in linear order for each entity
		brush_t		*temp;
		int			newbrushes;
		int			worldbrushes;
		int			i;

		newbrushes = mapent->numbrushes;
		worldbrushes = entities[0].numbrushes;

		temp = malloc(newbrushes*sizeof(brush_t));
		memcpy (temp, mapbrushes + mapent->firstbrush, newbrushes*sizeof(brush_t));

		for (i=0 ; i<newbrushes ; i++)
			temp[i].entitynum = 0;
		
		// make space to move the brushes (overlapped copy)
		memmove (mapbrushes + worldbrushes + newbrushes,
			mapbrushes + worldbrushes,
			sizeof(brush_t) * (nummapbrushes - worldbrushes - newbrushes) );

		// copy the new brushes down
		memcpy (mapbrushes + worldbrushes, temp, sizeof(brush_t) * newbrushes);

		// fix up indexes
		num_entities--;
		entities[0].numbrushes += newbrushes;
		for (i=1 ; i<num_entities ; i++)
			entities[i].firstbrush += newbrushes;
		memset (mapent,0, sizeof(*mapent));
		free (temp);
	}

	return true;
}

/*
================
LoadMapFile
================
*/
void LoadMapFile (char *filename)
{		
	LoadScriptFile (filename);
	
	num_entities = 0;
	
	while (ParseMapEntity ())
	{
	}
	
	qprintf ("Load map:%s\n", filename);
	qprintf ("%5i brushes\n", nummapbrushes);
	qprintf ("%5i entities\n", num_entities);
}

