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

typedef struct
{
	char		identification[4];		// should be WAD2/WAD3
	int			numlumps;
	int			infotableofs;
} wadinfo_t;


typedef struct
{
	int			filepos;
	int			disksize;
	int			size;					// uncompressed
	char		type;
	char		compression;
	char		pad1, pad2;
	char		name[16];				// must be null terminated

	int			iTexFile;	// index of the wad this texture is located in

} lumpinfo_t;

int			nummiptex;
lumpinfo_t	miptex[MAX_MAP_TEXTURES];

int			nTexLumps = 0;
lumpinfo_t	*lumpinfo = NULL;

int			nTexFiles = 0;
FILE		*texfiles[128];

int			nWadInclude;
char		*pszWadInclude[128];
qboolean	wadInclude[128];	// include the textures from this WAD in the BSP

void CleanupName (char *in, char *out)
{
	int		i;
	
	for (i=0 ; i< 16 ; i++ )
	{
		if (!in[i])
			break;
			
		out[i] = toupper(in[i]);
	}
	
	for ( ; i< 16 ; i++ )
		out[i] = 0;
}

/*
=================
lump_sorters
=================
*/

int
lump_sorter_by_wad_and_name( const void *lump1, const void *lump2 )
{
	lumpinfo_t	*plump1 = (lumpinfo_t *)lump1;
	lumpinfo_t	*plump2 = (lumpinfo_t *)lump2;
	if ( plump1->iTexFile == plump2->iTexFile )
		return strcmp( plump1->name, plump2->name );
	else
		return plump1->iTexFile - plump2->iTexFile;
}

int
lump_sorter_by_name( const void *lump1, const void *lump2 )
{
	lumpinfo_t	*plump1 = (lumpinfo_t *)lump1;
	lumpinfo_t	*plump2 = (lumpinfo_t *)lump2;
	return strcmp( plump1->name, plump2->name );
}

/*
=================
TEX_InitFromWad
=================
*/
qboolean	TEX_InitFromWad (char *path)
{
	int			i;
	wadinfo_t	wadinfo;
	char		szTmpPath[512];
	char		*pszWadFile;

	strcpy(szTmpPath, path);

	// temporary kludge so we don't have to deal with no occurances of a semicolon
	//  in the path name ..
	if(strchr(szTmpPath, ';') == NULL)
		strcat(szTmpPath, ";");

	pszWadFile = strtok(szTmpPath, ";");

	while(pszWadFile)
	{
		FILE *texfile;	// temporary used in this loop

		texfiles[nTexFiles] = fopen(pszWadFile, "rb");
		if (!texfiles[nTexFiles])
		{
			// maybe this wad file has a hard code drive
			if (pszWadFile[1] == ':')
			{
				pszWadFile += 2; // skip past the file
				texfiles[nTexFiles] = fopen (pszWadFile, "rb");
			}
		}


		if (!texfiles[nTexFiles])
		{
			printf ("WARNING: couldn't open %s\n", pszWadFile);
			return false;
		}

		++nTexFiles;

		// look and see if we're supposed to include the textures from this WAD in the bsp.
		for (i = 0; i < nWadInclude; i++)
		{
			if (stricmp( pszWadInclude[i], pszWadFile ) == 0)
			{
				wadInclude[nTexFiles-1] = true;
			}
		}

		// temp assignment to make things cleaner:
		texfile = texfiles[nTexFiles-1];

		printf ("Using WAD File: %s\n", pszWadFile);

		SafeRead(texfile, &wadinfo, sizeof(wadinfo));
		if (strncmp (wadinfo.identification, "WAD2", 4) &&
			strncmp (wadinfo.identification, "WAD3", 4))
			Error ("TEX_InitFromWad: %s isn't a wadfile",pszWadFile);
		wadinfo.numlumps = LittleLong(wadinfo.numlumps);
		wadinfo.infotableofs = LittleLong(wadinfo.infotableofs);
		fseek (texfile, wadinfo.infotableofs, SEEK_SET);
		lumpinfo = realloc(lumpinfo, (nTexLumps + wadinfo.numlumps) 
			* sizeof(lumpinfo_t));

		for(i = 0; i < wadinfo.numlumps; i++)
		{
			SafeRead(texfile, &lumpinfo[nTexLumps], sizeof(lumpinfo_t) - 
				sizeof(int));	// iTexFile is NOT read from file
			CleanupName (lumpinfo[nTexLumps].name, lumpinfo[nTexLumps].name);
			lumpinfo[nTexLumps].filepos = LittleLong(lumpinfo[nTexLumps].filepos);
			lumpinfo[nTexLumps].disksize = LittleLong(lumpinfo[nTexLumps].disksize);
			lumpinfo[nTexLumps].iTexFile = nTexFiles-1;

			++nTexLumps;
		}

		// next wad file
		pszWadFile = strtok(NULL, ";");
	}

	qsort( (void *)lumpinfo, (size_t)nTexLumps, sizeof(lumpinfo[0]), lump_sorter_by_name );

	return true;
}

/*
==================
FindTexture
==================
*/

lumpinfo_t *
FindTexture (lumpinfo_t *source )
{
	lumpinfo_t *found = NULL;
	if ( !( found = bsearch( source, (void *)lumpinfo, (size_t)nTexLumps, sizeof(lumpinfo[0]), lump_sorter_by_name ) ) )
		printf ("WARNING: texture %s not found in BSP's wad file list!\n", source->name);

	return found;
}

/*
==================
LoadLump
==================
*/
int LoadLump (lumpinfo_t *source, byte *dest, int *texsize)
{
	*texsize = 0;
	if ( source->filepos )
	{
		fseek (texfiles[source->iTexFile], source->filepos, SEEK_SET);
		*texsize = source->disksize;
		
		// Should we just load the texture header w/o the palette & bitmap?
		if ( wadtextures && !wadInclude[source->iTexFile] )
		{
			// Just read the miptex header and zero out the data offsets.
			// We will load the entire texture from the WAD at engine runtime
			int			i;
			miptex_t	*miptex = (miptex_t *)dest;
			SafeRead (texfiles[source->iTexFile], dest, sizeof(miptex_t) );
			for( i=0; i<MIPLEVELS; i++ )
				miptex->offsets[i] = 0;
			return sizeof(miptex_t);
		}
		else
		{
			// Load the entire texture here so the BSP contains the texture
			SafeRead (texfiles[source->iTexFile], dest, source->disksize );
			return source->disksize;
		}
	}

	printf ("WARNING: texture %s not found in BSP's wad file list!\n", source->name);
	return 0;
}


/*
==================
AddAnimatingTextures
==================
*/
void AddAnimatingTextures (void)
{
	int		base;
	int		i, j, k;
	char	name[32];

	base = nummiptex;
	
	for (i=0 ; i<base ; i++)
	{
		if (miptex[i].name[0] != '+' && miptex[i].name[0] != '-')
			continue;
		strcpy (name, miptex[i].name);

		for (j=0 ; j<20 ; j++)
		{
			if (j < 10)
				name[1] = '0'+j;
			else
				name[1] = 'A'+j-10;		// alternate animation
			

		// see if this name exists in the wadfile
			for (k=0 ; k < nTexLumps; k++)
				if (!strcmp(name, lumpinfo[k].name))
				{
					FindMiptex (name);	// add to the miptex list
					break;
				}
		}
	}
	
	if ( nummiptex - base )
		printf ("added %i additional animating textures.\n", nummiptex - base);
}

/*
==================
WriteMiptex
==================
*/
void WriteMiptex(void)
{
	int		i, len, texsize, totaltexsize = 0;
	byte	*data;
	dmiptexlump_t	*l;
	char	*path;
	char	fullpath[1024];
	int		start;

	texdatasize = 0;

	path = ValueForKey (&entities[0], "_wad");
	if (!path || !path[0])
	{
		path = ValueForKey (&entities[0], "wad");
		if (!path || !path[0])
		{
			printf ("WARNING: no wadfile specified\n");
			return;
		}
	}
	
	strcpy(fullpath, path);

	start = GetTickCount();
	{
		if (!TEX_InitFromWad (fullpath))
			return;
		AddAnimatingTextures ();
	}
	qprintf( "TEX_InitFromWad & AddAnimatingTextures elapsed time = %ldms\n", GetTickCount()-start );

	start = GetTickCount();
	{
		for (i=0; i<nummiptex; i++ )
		{
			lumpinfo_t	*found;
			if ( found = FindTexture( miptex + i ) )
				miptex[i] = *found;
			else
			{
				miptex[i].iTexFile = miptex[i].filepos = miptex[i].disksize = 0;
			}
		}
	}
	qprintf( "FindTextures elapsed time = %ldms\n", GetTickCount()-start );

	start = GetTickCount();
	{
		int			final_miptex, i;
		texinfo_t	*tx = texinfo;

		// Sort them FIRST by wadfile and THEN by name for most efficient loading in the engine.
		qsort( (void *)miptex, (size_t)nummiptex, sizeof(miptex[0]), lump_sorter_by_wad_and_name );

		// Sleazy Hack 104 Pt 2 - After sorting the miptex array, reset the texinfos to point to the right miptexs
		for(i=0; i<numtexinfo; i++, tx++)
		{
			char *miptex_name = (char *)tx->miptex;
			tx->miptex = FindMiptex( miptex_name );

			// Free up the originally strdup()'ed miptex_name
			free( miptex_name );
		}
	}
	qprintf( "qsort(miptex) elapsed time = %ldms\n", GetTickCount()-start );

	start = GetTickCount();
	{
		// Now setup to get the miptex data (or just the headers if using -wadtextures) from the wadfile
		l = (dmiptexlump_t *)dtexdata;
		data = (byte *)&l->dataofs[nummiptex];
		l->nummiptex = nummiptex;
		for (i=0 ; i<nummiptex ; i++)
		{
			l->dataofs[i] = data - (byte *)l;
			len = LoadLump (miptex+i, data, &texsize);
			if (data + len - dtexdata >= MAX_MAP_MIPTEX)
				Error ("Textures exceeded MAX_MAP_MIPTEX");
			if (!len)
				l->dataofs[i] = -1;	// didn't find the texture
			else
			{
				totaltexsize += texsize;
				if ( totaltexsize > MAX_MAP_MIPTEX )
					Error("Textures exceeded MAX_MAP_MIPTEX" );
			}
			data += len;
		}
		texdatasize = data - dtexdata;
	}
	qprintf( "LoadLump() elapsed time = %ldms\n", GetTickCount()-start );
}

//==========================================================================


int	FindMiptex (char *name)
{
	int		i;

	ThreadLock ();
	for (i=0 ; i<nummiptex ; i++)
		if (!Q_strcasecmp (name, miptex[i].name))
		{
			ThreadUnlock ();
			return i;
		}
	if (nummiptex == MAX_MAP_TEXTURES)
		Error ("Exceeded MAX_MAP_TEXTURES");
	strcpy (miptex[i].name, name);
	nummiptex++;
	ThreadUnlock ();
	return i;
}

int TexinfoForBrushTexture (plane_t *plane, brush_texture_t *bt, vec3_t origin)
{
	vec3_t	vecs[2];
	int		sv, tv;
	vec_t	ang, sinv, cosv;
	vec_t	ns, nt;
	texinfo_t	tx, *tc;
	int		i, j, k;

	memset (&tx, 0, sizeof(tx));
	tx.miptex = FindMiptex (bt->name);
	// Note: FindMiptex() still needs to be called here to add it to the global miptex array

	// Very Sleazy Hack 104 - since the tx.miptex index will be bogus after we sort the miptex array later
	// Put the string name of the miptex in this "index" until after we are done sorting it in WriteMiptex().
	tx.miptex = (int)strdup(bt->name);

	// set the special flag
	if (bt->name[0] == '*' 
	|| !Q_strncasecmp (bt->name, "sky",3)
	|| !Q_strncasecmp (bt->name, "clip",4)
	|| !Q_strncasecmp (bt->name, "origin",6)
	|| !Q_strncasecmp (bt->name, "aaatrigger",10))
		tx.flags |= TEX_SPECIAL;

	if (g_nMapFileVersion < 220)
	{
		TextureAxisFromPlane(plane, vecs[0], vecs[1]);
	}

	if (!bt->scale[0])
		bt->scale[0] = 1;
	if (!bt->scale[1])
		bt->scale[1] = 1;

	if (g_nMapFileVersion < 220)
	{
		// rotate axis
		if (bt->rotate == 0)
			{ sinv = 0 ; cosv = 1; }
		else if (bt->rotate == 90)
			{ sinv = 1 ; cosv = 0; }
		else if (bt->rotate == 180)
			{ sinv = 0 ; cosv = -1; }
		else if (bt->rotate == 270)
			{ sinv = -1 ; cosv = 0; }
		else
		{	
			ang = bt->rotate / 180 * Q_PI;
			sinv = sin(ang);
			cosv = cos(ang);
		}

		if (vecs[0][0])
			sv = 0;
		else if (vecs[0][1])
			sv = 1;
		else
			sv = 2;
					
		if (vecs[1][0])
			tv = 0;
		else if (vecs[1][1])
			tv = 1;
		else
			tv = 2;
						
		for (i=0 ; i<2 ; i++)
		{
			ns = cosv * vecs[i][sv] - sinv * vecs[i][tv];
			nt = sinv * vecs[i][sv] +  cosv * vecs[i][tv];
			vecs[i][sv] = ns;
			vecs[i][tv] = nt;
		}

		for (i=0 ; i<2 ; i++)
			for (j=0 ; j<3 ; j++)
				tx.vecs[i][j] = vecs[i][j] / bt->scale[i];
	}
	else
	{
		tx.vecs[0][0] = bt->UAxis[0] / bt->scale[0];
		tx.vecs[0][1] = bt->UAxis[1] / bt->scale[0];
		tx.vecs[0][2] = bt->UAxis[2] / bt->scale[0];

		tx.vecs[1][0] = bt->VAxis[0] / bt->scale[1];
		tx.vecs[1][1] = bt->VAxis[1] / bt->scale[1];
		tx.vecs[1][2] = bt->VAxis[2] / bt->scale[1];
	}

	tx.vecs[0][3] = bt->shift[0] + DotProduct( origin, tx.vecs[0] );
	tx.vecs[1][3] = bt->shift[1] + DotProduct( origin, tx.vecs[1] );

	//
	// find the texinfo
	//
	ThreadLock ();
	tc = texinfo;
	for (i=0 ; i<numtexinfo ; i++, tc++)
	{
		// Sleazy hack 104, Pt 3 - Use strcmp on names to avoid dups
		if ( strcmp( (char *)(tc->miptex), (char *)(tx.miptex)) != 0 )
			continue;
		if (tc->flags != tx.flags)
			continue;
		for (j=0 ; j<2 ; j++)
		{
			for (k=0 ; k<4 ; k++)
			{
				if (tc->vecs[j][k] != tx.vecs[j][k])
					goto skip;
			}
		}
		ThreadUnlock ();
		return i;
skip:;
	}
	if (numtexinfo == MAX_MAP_TEXINFO)
		Error ("Exceeded MAX_MAP_TEXINFO");
	*tc = tx;
	numtexinfo++;
	ThreadUnlock ();
	return i;
}

