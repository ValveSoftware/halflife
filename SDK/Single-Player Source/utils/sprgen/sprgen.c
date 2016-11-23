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
// spritegen.c: generates a .spr file from a series of .lbm frame files.
// Result is stored in /raid/quake/id1/sprites/<scriptname>.spr.
//
#pragma warning(disable : 4244)     // type conversion warning.
#define INCLUDELIBS


#ifdef NeXT
#include <libc.h>
#endif

#include "spritegn.h"

#define MAX_BUFFER_SIZE		0x100000
#define MAX_FRAMES			1000

dsprite_t		sprite;
byte			*byteimage, *lbmpalette;
int				byteimagewidth, byteimageheight;
byte			*lumpbuffer = NULL, *plump;
char			spritedir[1024];
char			spriteoutname[1024];
int				framesmaxs[2];
int				framecount;

qboolean do16bit = true;

typedef struct {
	spriteframetype_t	type;		// single frame or group of frames
	void				*pdata;		// either a dspriteframe_t or group info
	float				interval;	// only used for frames in groups
	int					numgroupframes;	// only used by group headers
} spritepackage_t;

spritepackage_t	frames[MAX_FRAMES];

void FinishSprite (void);
void Cmd_Spritename (void);


/*
============
WriteFrame
============
*/
void WriteFrame (FILE *spriteouthandle, int framenum)
{
	dspriteframe_t	*pframe;
	dspriteframe_t	frametemp;

	pframe = (dspriteframe_t *)frames[framenum].pdata;
	frametemp.origin[0] = LittleLong (pframe->origin[0]);
	frametemp.origin[1] = LittleLong (pframe->origin[1]);
	frametemp.width = LittleLong (pframe->width);
	frametemp.height = LittleLong (pframe->height);

	SafeWrite (spriteouthandle, &frametemp, sizeof (frametemp));
	SafeWrite (spriteouthandle,
			   (byte *)(pframe + 1),
			   pframe->height * pframe->width);
}


/*
============
WriteSprite
============
*/
void WriteSprite (FILE *spriteouthandle)
{
	int			i, groupframe, curframe;
	dsprite_t	spritetemp;

	sprite.boundingradius = sqrt (((framesmaxs[0] >> 1) *
								   (framesmaxs[0] >> 1)) +
								  ((framesmaxs[1] >> 1) *
								   (framesmaxs[1] >> 1)));

//
// write out the sprite header
//
	spritetemp.type = LittleLong (sprite.type);
	spritetemp.texFormat = LittleLong (sprite.texFormat);
	spritetemp.boundingradius = LittleFloat (sprite.boundingradius);
	spritetemp.width = LittleLong (framesmaxs[0]);
	spritetemp.height = LittleLong (framesmaxs[1]);
	spritetemp.numframes = LittleLong (sprite.numframes);
	spritetemp.beamlength = LittleFloat (sprite.beamlength);
	spritetemp.synctype = LittleFloat (sprite.synctype);
	spritetemp.version = LittleLong (SPRITE_VERSION);
	spritetemp.ident = LittleLong (IDSPRITEHEADER);

	SafeWrite (spriteouthandle, &spritetemp, sizeof(spritetemp));

	if( do16bit )
	{
		// Write out palette in 16bit mode
		short cnt = 256;
		SafeWrite( spriteouthandle, (void *) &cnt, sizeof(cnt) );
		SafeWrite( spriteouthandle, lbmpalette, cnt * 3 );
	}

//
// write out the frames
//
	curframe = 0;

	for (i=0 ; i<sprite.numframes ; i++)
	{
		SafeWrite (spriteouthandle, &frames[curframe].type,
				   sizeof(frames[curframe].type));

		if (frames[curframe].type == SPR_SINGLE)
		{
		//
		// single (non-grouped) frame
		//
			WriteFrame (spriteouthandle, curframe);
			curframe++;
		}
		else
		{
			int					j, numframes;
			dspritegroup_t		dsgroup;
			float				totinterval;

			groupframe = curframe;
			curframe++;
			numframes = frames[groupframe].numgroupframes;

		//
		// set and write the group header
		//
			dsgroup.numframes = LittleLong (numframes);

			SafeWrite (spriteouthandle, &dsgroup, sizeof(dsgroup));

		//
		// write the interval array
		//
			totinterval = 0.0;

			for (j=0 ; j<numframes ; j++)
			{
				dspriteinterval_t	temp;

				totinterval += frames[groupframe+1+j].interval;
				temp.interval = LittleFloat (totinterval);

				SafeWrite (spriteouthandle, &temp, sizeof(temp));
			}

			for (j=0 ; j<numframes ; j++)
			{
				WriteFrame (spriteouthandle, curframe);
				curframe++;
			}
		}
	}

}


/*
============
ExecCommand
============
*/
int	cmdsrun;

void ExecCommand (char *cmd, ...)
{
	int		ret;
	char	cmdline[1024];
	va_list	argptr;
	
	cmdsrun++;
	
	va_start (argptr, cmd);
	vsprintf (cmdline,cmd,argptr);
	va_end (argptr);
	
//	printf ("=============================================================\n");
//	printf ("spritegen: %s\n",cmdline);
	fflush (stdout);
	ret = system (cmdline);
//	printf ("=============================================================\n");
	
	if (ret)
		Error ("spritegen: exiting due to error");
}

/*
==============
LoadScreen
==============
*/
void LoadScreen (char *name)
{
	static byte origpalette[256*3];
	int iError;

	printf ("grabbing from %s...\n",name);
	iError = LoadBMP( name, &byteimage, &lbmpalette );
	if (iError)
		Error( "unable to load file \"%s\"\n", name );

	byteimagewidth = bmhd.w;
	byteimageheight = bmhd.h;

	if (sprite.numframes == 0)
		memcpy( origpalette, lbmpalette, sizeof( origpalette ));
	else if (memcmp( origpalette, lbmpalette, sizeof( origpalette )) != 0)
	{
		Error( "bitmap \"%s\" doesn't share a pallette with the previous bitmap\n", name );
	}
}


/*
===============
Cmd_Type
===============
*/
void Cmd_Type (void)
{
	GetToken (false);
	if (!strcmp (token, "vp_parallel_upright"))
		sprite.type = SPR_VP_PARALLEL_UPRIGHT;
	else if (!strcmp (token, "facing_upright"))
		sprite.type = SPR_FACING_UPRIGHT;
	else if (!strcmp (token, "vp_parallel"))
		sprite.type = SPR_VP_PARALLEL;
	else if (!strcmp (token, "oriented"))
		sprite.type = SPR_ORIENTED;
	else if (!strcmp (token, "vp_parallel_oriented"))
		sprite.type = SPR_VP_PARALLEL_ORIENTED;
	else
		Error ("Bad sprite type\n");
}


/*
===============
Cmd_Texture
===============
*/
void Cmd_Texture (void)
{
	GetToken (false);

	if (!strcmp (token, "additive"))
		sprite.texFormat = SPR_ADDITIVE;
	else if (!strcmp (token, "normal"))
		sprite.texFormat = SPR_NORMAL;
	else if (!strcmp (token, "indexalpha"))
		sprite.texFormat = SPR_INDEXALPHA;
	else if (!strcmp (token, "alphatest"))
		sprite.texFormat = SPR_ALPHTEST;
	else
		Error ("Bad sprite texture type\n");
}


/*
===============
Cmd_Beamlength
===============
*/
void Cmd_Beamlength ()
{
	GetToken (false);
	sprite.beamlength = atof (token);
}


/*
===============
Cmd_Load
===============
*/
void Cmd_Load (void)
{
	GetToken (false);
	LoadScreen (ExpandPathAndArchive(token));
}


/*
===============
Cmd_Frame
===============
*/
void Cmd_Frame ()
{
	int             x,y,xl,yl,xh,yh,w,h;
	byte            *screen_p, *source;
	int             linedelta;
	dspriteframe_t	*pframe;
	int				pix;
	
	GetToken (false);
	xl = atoi (token);
	GetToken (false);
	yl = atoi (token);
	GetToken (false);
	w = atoi (token);
	GetToken (false);
	h = atoi (token);

	if ((xl & 0x07) || (yl & 0x07) || (w & 0x07) || (h & 0x07))
		Error ("Sprite dimensions not multiples of 8\n");

	if ((w > 256) || (h > 256))
		Error ("Sprite has a dimension longer than 256");

	xh = xl+w;
	yh = yl+h;

	pframe = (dspriteframe_t *)plump;
	frames[framecount].pdata = pframe;
	frames[framecount].type = SPR_SINGLE;

	if (TokenAvailable ())
	{
		GetToken (false);
		frames[framecount].interval = atof (token);
		if (frames[framecount].interval <= 0.0)
			Error ("Non-positive interval");
	}
	else
	{
		frames[framecount].interval = (float)0.1;
	}
	
	if (TokenAvailable ())
	{
		GetToken (false);
		pframe->origin[0] = -atoi (token);
		GetToken (false);
		pframe->origin[1] = atoi (token);
	}
	else
	{
		pframe->origin[0] = -(w >> 1);
		pframe->origin[1] = h >> 1;
	}

	pframe->width = w;
	pframe->height = h;

	if (w > framesmaxs[0])
		framesmaxs[0] = w;
	
	if (h > framesmaxs[1])
		framesmaxs[1] = h;
	
	plump = (byte *)(pframe + 1);

	screen_p = byteimage + yl*byteimagewidth + xl;
	linedelta = byteimagewidth - w;

	source = plump;

	for (y=yl ; y<yh ; y++)
	{
		for (x=xl ; x<xh ; x++)
		{
			pix = *screen_p;
			*screen_p++ = 0;
//			if (pix == 255)
//				pix = 0;
			*plump++ = pix;
		}
		screen_p += linedelta;
	}

	framecount++;
	if (framecount >= MAX_FRAMES)
		Error ("Too many frames; increase MAX_FRAMES\n");
}


/*
===============
Cmd_GroupStart	
===============
*/
void Cmd_GroupStart (void)
{
	int			groupframe;

	groupframe = framecount++;

	frames[groupframe].type = SPR_GROUP;
	frames[groupframe].numgroupframes = 0;

	while (1)
	{
		GetToken (true);
		if (endofscript)
			Error ("End of file during group");

		if (!strcmp (token, "$frame"))
		{
			Cmd_Frame ();
			frames[groupframe].numgroupframes++;
		}
		else if (!strcmp (token, "$load"))
		{
			Cmd_Load ();
		}
		else if (!strcmp (token, "$groupend"))
		{
			break;
		}
		else
		{
			Error ("$frame, $load, or $groupend expected\n");
		}

	}

	if (frames[groupframe].numgroupframes == 0)
		Error ("Empty group\n");
}


/*
===============
ParseScript	
===============
*/
void ParseScript (void)
{
	while (1)
	{
		GetToken (true);
		if (endofscript)
			break;
	
		if (!strcmp (token, "$load"))
		{
			Cmd_Load ();
		}
		if (!strcmp (token, "$spritename"))
		{
			Cmd_Spritename ();
		}
		else if (!strcmp (token, "$type"))
		{
			Cmd_Type ();
		}
		else if (!strcmp (token, "$texture"))
		{
			Cmd_Texture ();
		}
		else if (!strcmp (token, "$beamlength"))
		{
			Cmd_Beamlength ();
		}
		else if (!strcmp (token, "$sync"))
		{
			sprite.synctype = ST_SYNC;
		}
		else if (!strcmp (token, "$frame"))
		{
			Cmd_Frame ();
			sprite.numframes++;
		}		
		else if (!strcmp (token, "$load"))
		{
			Cmd_Load ();
		}
		else if (!strcmp (token, "$groupstart"))
		{
			Cmd_GroupStart ();
			sprite.numframes++;
		}
	}
}

/*
==============
Cmd_Spritename
==============
*/
void Cmd_Spritename (void)
{
	if (sprite.numframes)
		FinishSprite ();

	GetToken (false);
	sprintf (spriteoutname, "%s%s.spr", spritedir, token);
	memset (&sprite, 0, sizeof(sprite));
	framecount = 0;

	framesmaxs[0] = -9999999;
	framesmaxs[1] = -9999999;

	if ( !lumpbuffer )
		lumpbuffer = malloc (MAX_BUFFER_SIZE * 2);	// *2 for padding

	if (!lumpbuffer)
		Error ("Couldn't get buffer memory");

	plump = lumpbuffer;
	sprite.synctype = ST_RAND;	// default
}

/*
==============
FinishSprite	
==============
*/
void FinishSprite (void)
{
	FILE	*spriteouthandle;

	if (sprite.numframes == 0)
		Error ("no frames\n");

	if (!strlen(spriteoutname))
		Error ("Didn't name sprite file");
		
	if ((plump - lumpbuffer) > MAX_BUFFER_SIZE)
		Error ("Sprite package too big; increase MAX_BUFFER_SIZE");

	spriteouthandle = SafeOpenWrite (spriteoutname);
	printf ("saving in %s\n", spriteoutname);
	WriteSprite (spriteouthandle);
	fclose (spriteouthandle);
	
	printf ("spritegen: successful\n");
	printf ("%d frame(s)\n", sprite.numframes);
	printf ("%d ungrouped frame(s), including group headers\n", framecount);
	
	spriteoutname[0] = 0;		// clear for a new sprite
}

/*
==============
main
	
==============
*/
extern char qproject[];

int main (int argc, char **argv)
{
	int		i;

	printf( "sprgen.exe v 1.1 (%s)\n", __DATE__ );
	printf ("----- Sprite Gen ----\n");

	if (argc < 2 || argc > 7 )
		Error ("usage: sprgen [-archive directory] [-no16bit] [-proj <project>] file.qc");
		
	for( i=1; i<argc; i++ )
	{
		if( *argv[ i ] == '-' )
		{
			if( !strcmp( argv[ i ], "-archive" ) )
			{
				archive = true;
				strcpy (archivedir, argv[i+1]);
				printf ("Archiving source to: %s\n", archivedir);
				i++;
			}
			else if( !strcmp( argv[ i ], "-proj" ) )
			{
				strcpy( qproject, argv[i+1] );
				i++;
			}
			else if( !strcmp( argv[ i ], "-16bit" ) )
			{
				do16bit = true;
			}
			else if( !strcmp( argv[ i ], "-no16bit" ) )
			{
				do16bit = false;
			} 
			else
			{
				printf( "Unsupported command line flag: '%s'", argv[i] );
			}

		}
		else
			break;
	}

	// SetQdirFromPath (argv[i]);
	ExtractFilePath (argv[i], spritedir);	// chop the filename

//
// load the script
//
	LoadScriptFile (argv[i]);
	
	ParseScript ();
	FinishSprite ();

	return 0;
}

