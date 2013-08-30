/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

// lbmlib.h

typedef unsigned char	UBYTE;

#ifndef _WINDOWS_
typedef short			WORD;
#endif

typedef unsigned short	UWORD;
typedef long			LONG;

typedef enum
{
	ms_none,
	ms_mask,
	ms_transcolor,
	ms_lasso
} mask_t;

typedef enum
{
	cm_none,
	cm_rle1
} compress_t;

typedef struct
{
	UWORD		w,h;
	WORD		x,y;
	UBYTE		nPlanes;
	UBYTE		masking;
	UBYTE		compression;
	UBYTE		pad1;
	UWORD		transparentColor;
	UBYTE		xAspect,yAspect;
	WORD		pageWidth,pageHeight;
} bmhd_t;

extern	bmhd_t	bmhd;						// will be in native byte order


void LoadLBM (char *filename, byte **picture, byte **palette);
int	LoadBMP (const char* szFile, byte** ppbBits, byte** ppbPalette);
void WriteLBMfile (char *filename, byte *data, int width, int height
	, byte *palette);
int WriteBMPfile (char *szFile, byte *pbBits, int width, int height, byte *pbPalette);

