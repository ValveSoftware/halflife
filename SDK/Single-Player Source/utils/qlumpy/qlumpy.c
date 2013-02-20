/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#define VERSION "2.2"
#include "qlumpy.h"


#define MAXLUMP		0x50000         // biggest possible lump

extern char qproject[];

int				grabbed;

byte            *byteimage, *lbmpalette;
int              byteimagewidth, byteimageheight;

char            basepath[1024];
char            lumpname[16];

char			destfile[1024];

byte            *lumpbuffer, *lump_p;

qboolean		savesingle;
qboolean		outputcreated;

qboolean		do16bit;
qboolean		fTransparent255;

/*
=============================================================================

							MAIN

=============================================================================
*/

void GrabRaw (void);
void GrabPalette (void);
void GrabPic (void);
void GrabMip (void);
void GrabColormap (void);
void GrabColormap2 (void);
void GrabFont( void );

typedef struct
{
	char    *name;
	void    (*function) (void);
} command_t;

command_t       commands[] =
{
	{ "palette", GrabPalette },
	{ "colormap", GrabColormap },
	{ "qpic", GrabPic },
	{ "miptex", GrabMip },
	{ "raw", GrabRaw },

	{ "colormap2", GrabColormap2 },
	{ "font", GrabFont },

	{ NULL, NULL }                     // list terminator
};


#define TRANSPARENT_R		0x0
#define TRANSPARENT_G		0x0
#define TRANSPARENT_B		0xFF
#define IS_TRANSPARENT(p)	(p[0]==TRANSPARENT_R && p[1]==TRANSPARENT_G && p[2]==TRANSPARENT_B)
/*
==============
TransparentByteImage
==============
*/
void TransparentByteImage( void )
{
	// Remap all pixels of color 0,0,255 to index 255 and remap index 255 to something else
	byte	transtable[256], *image;
	int		i, j, firsttrans;

	firsttrans = -1;
	for ( i = 0; i < 256; i++ ) {
		if ( IS_TRANSPARENT( (lbmpalette+(i*3)) ) ) {
			transtable[i] = 255;
			if ( firsttrans < 0 )
				firsttrans = i;
		}
		else
			transtable[i] = i;
	}

	// If there is some transparency, translate it
	if ( firsttrans >= 0 ) {
		if ( !IS_TRANSPARENT( (lbmpalette+(255*3)) ) )
			transtable[255] = firsttrans;
		image = byteimage;
		for ( j = 0; j < byteimageheight; j++ ) {
			for ( i = 0; i < byteimagewidth; i++ ) {
				*image = transtable[*image];
				image++;
			}
		}
		// Move palette entry for pixels previously mapped to entry 255
		lbmpalette[ firsttrans*3 + 0 ] = lbmpalette[ 255*3 + 0 ];
		lbmpalette[ firsttrans*3 + 1 ] = lbmpalette[ 255*3 + 1 ];
		lbmpalette[ firsttrans*3 + 2 ] = lbmpalette[ 255*3 + 2 ];
		lbmpalette[ 255*3 + 0 ] = TRANSPARENT_R;
		lbmpalette[ 255*3 + 1 ] = TRANSPARENT_G;
		lbmpalette[ 255*3 + 2 ] = TRANSPARENT_B;
	}
}



/*
==============
LoadScreen
==============
*/
void LoadScreen (char *name)
{
	char	*expanded;

	expanded = ExpandPathAndArchive (name);

	printf ("grabbing from %s...\n",expanded);
	LoadLBM (expanded, &byteimage, &lbmpalette);

	byteimagewidth = bmhd.w;
	byteimageheight = bmhd.h;
}


/*
==============
LoadScreenBMP
==============
*/
void LoadScreenBMP(char *pszName)
{
	char	*pszExpanded;
	char	basename[64];
	
	pszExpanded = ExpandPathAndArchive(pszName);

	printf("grabbing from %s...\n", pszExpanded);
	if (LoadBMP(pszExpanded, &byteimage, &lbmpalette))
		Error ("Failed to load!", pszExpanded);

	if ( byteimage == NULL || lbmpalette == NULL )
		Error("FAIL!",pszExpanded);
	byteimagewidth = bmhd.w;
	byteimageheight = bmhd.h;

	ExtractFileBase (token, basename);		// Files that start with '$' have color (0,0,255) transparent,
	if ( basename[0] == '{' ) {				// move to last palette entry.
		fTransparent255 = true;
		TransparentByteImage();
	}
}


/*
================
CreateOutput
================
*/
void CreateOutput (void)
{
	outputcreated = true;
//
// create the output wadfile file
//
	NewWad (destfile, false);	// create a new wadfile
}

/*
===============
WriteLump
===============
*/
void WriteLump (int type, int compression)
{
	int		size;
	
	if (!outputcreated)
		CreateOutput ();

//
// dword align the size
//
	while ((int)lump_p&3)
		*lump_p++ = 0;

	size = lump_p - lumpbuffer;
	if (size > MAXLUMP)
		Error ("Lump size exceeded %d, memory corrupted!",MAXLUMP);

//
// write the grabbed lump to the wadfile
//
	AddLump (lumpname,lumpbuffer,size,type, compression);
}

/*
===========
WriteFile

Save as a seperate file instead of as a wadfile lump
===========
*/
void WriteFile (void)
{
	char	filename[1024];
	char	*exp;

	sprintf (filename,"%s/%s.lmp", destfile, lumpname);
	exp = ExpandPath(filename);
	printf ("saved %s\n", exp);
	SaveFile (exp, lumpbuffer, lump_p-lumpbuffer);		
}

/*
================
ParseScript
================
*/
void ParseScript (void)
{
	int			cmd;
	int			size;

	fTransparent255 = false;
	do
	{
		//
		// get a command / lump name
		//
		GetToken (true);
		if (endofscript)
			break;
		if (!Q_strcasecmp (token,"$LOAD"))
		{
			GetToken (false);
			LoadScreen (token);
			continue;
		}

		if (!Q_strcasecmp (token,"$DEST"))
		{
			GetToken (false);
			strcpy (destfile, token);
			continue;
		}

		if (!Q_strcasecmp (token,"$SINGLEDEST"))
		{
			GetToken (false);
			strcpy (destfile, token);
			savesingle = true;
			continue;
		}


		if (!Q_strcasecmp (token,"$LOADBMP"))
		{
			GetToken (false);
			fTransparent255 = false;
			LoadScreenBMP (token);
			continue;
		}

		//
		// new lump
		//
		if (strlen(token) >= sizeof(lumpname) )
			Error ("\"%s\" is too long to be a lump name",token);
		memset (lumpname,0,sizeof(lumpname));			
		strcpy (lumpname, token);
		for (size=0 ; size<sizeof(lumpname) ; size++)
			lumpname[size] = tolower(lumpname[size]);

		//
		// get the grab command
		//
		lump_p = lumpbuffer;

		GetToken (false);

		//
		// call a routine to grab some data and put it in lumpbuffer
		// with lump_p pointing after the last byte to be saved
		//
		for (cmd=0 ; commands[cmd].name ; cmd++)
			if ( !Q_strcasecmp(token,commands[cmd].name) )
			{
				commands[cmd].function ();
				break;
			}

		if ( !commands[cmd].name )
			Error ("Unrecognized token '%s' at line %i",token,scriptline);
	
		grabbed++;
		
		if (savesingle)
			WriteFile ();
		else	
			WriteLump (TYP_LUMPY+cmd, 0);
		
	} while (!endofscript);
}

/*
=================
ProcessLumpyScript

Loads a script file, then grabs everything from it
=================
*/
void ProcessLumpyScript (char *basename)
{
	char            script[256];

	printf ("qlumpy script: %s\n",basename);
	
//
// create default destination directory
//
	strcpy (destfile, ExpandPath(basename));
	StripExtension (destfile);
	strcat (destfile,".wad");		// unless the script overrides, save in cwd

//
// save in a wadfile by default
//
	savesingle = false;
	grabbed = 0;
	outputcreated = false;
	
	
//
// read in the script file
//
	strcpy (script, basename);
	DefaultExtension (script, ".ls");
	LoadScriptFile (script);
	
	strcpy (basepath, basename);
	
	ParseScript ();				// execute load / grab commands
	
	if (!savesingle)
	{
		WriteWad (do16bit);				// write out the wad directory
		printf ("%i lumps grabbed in a wad file\n",grabbed);
	}
	else
		printf ("%i lumps written seperately\n",grabbed);
}


/*
==============================
main
==============================
*/
int main (int argc, char **argv)
{
	int		i;
	
	printf ("\nqlumpy "VERSION" by John Carmack, copyright (c) 1994 Id Software.\n");
	printf ("Portions copyright (c) 1998 Valve LLC (%s)\n", __DATE__ );

	if (argc == 1)
		Error ("qlumpy [-archive directory] [-8bit] [-proj <project>] scriptfile [scriptfile ...]");

	lumpbuffer = malloc (MAXLUMP);
	do16bit = true;

	for( i=1; i<argc; i++ )
	{
		if( *argv[ i ] == '-' )
		{
			if( !strcmp( argv[ i ], "-archive" ) )
			{
				archive = true;
				strcpy (archivedir, argv[2]);
				printf ("Archiving source to: %s\n", archivedir);
			}
			else if( !strcmp( argv[ i ], "-proj" ) )
			{
				strcpy( qproject, argv[ i + 1 ] );
				i++;
			}
			else if( !strcmp( argv[ i ], "-8bit" ) )
				do16bit = false;
		}
		else
			break;
	}

	// rest of arguments are script files
	for ( ; i<argc ; i++)
	{
		char szTemp[1024];
		char *pszPath = argv[i];

		// Fully qualify the path names before using them

		if (!(pszPath[0] == '/' || pszPath[0] == '\\' || pszPath[1] == ':'))
		{	// path is partial
			Q_getwd (szTemp);
			strcat (szTemp, pszPath);
			pszPath = szTemp;
		}
		SetQdirFromPath(pszPath);
		ProcessLumpyScript(pszPath);
	}
		
	return 0;
}