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
#include "scriplib.h"
#include "lbmlib.h"
#include "wadlib.h"


extern  byte    *byteimage, *lbmpalette;
extern  int     byteimagewidth, byteimageheight;

#define SCRN(x,y)       (*(byteimage+(y)*byteimagewidth+x))

extern  byte    *lump_p;
extern  byte	*lumpbuffer;

extern	char	lumpname[];

