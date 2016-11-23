/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/


#include <windows.h>
#include <STDIO.H>


int
ReadBmpFile(
	char* szFile,
	unsigned char** ppbPalette,
	unsigned char** ppbBits,
	int *pwidth,
	int *pheight)
	{
	int rc = 0;
	FILE *pfile = NULL;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	RGBQUAD rgrgbPalette[256];
	ULONG cbPalBytes;
	ULONG cbBmpBits;
	BYTE* pbBmpBits;

	// Bogus parameter check
	if (!(ppbPalette != NULL && ppbBits != NULL))
		{ rc = -1000; goto GetOut; }

	// File exists?
	if ((pfile = fopen(szFile, "rb")) == NULL)
		{ rc = -1; goto GetOut; }
	
	// Read file header
	if (fread(&bmfh, sizeof bmfh, 1/*count*/, pfile) != 1)
		{ rc = -2; goto GetOut; }

	// Bogus file header check
	if (!(bmfh.bfReserved1 == 0 && bmfh.bfReserved2 == 0))
		{ rc = -2000; goto GetOut; }

	// Read info header
	if (fread(&bmih, sizeof bmih, 1/*count*/, pfile) != 1)
		{ rc = -3; goto GetOut; }

	// Bogus info header check
	if (!(bmih.biSize == sizeof bmih && bmih.biPlanes == 1))
		{ rc = -3000; goto GetOut; }

	// Bogus bit depth?  Only 8-bit supported.
	if (bmih.biBitCount != 8)
		{ rc = -4; goto GetOut; }
	
	// Bogus compression?  Only non-compressed supported.
	if (bmih.biCompression != BI_RGB)
		{ rc = -5; goto GetOut; }

	// Figure out how many entires are actually in the table
	if (bmih.biClrUsed == 0)
		{
		cbPalBytes = (1 << bmih.biBitCount) * sizeof( RGBQUAD );
		}
	else 
		{
		cbPalBytes = bmih.biClrUsed * sizeof( RGBQUAD );
		}

	// Read palette (256 entries)
	if (fread(rgrgbPalette, cbPalBytes, 1/*count*/, pfile) != 1)
		{ rc = -6; goto GetOut; }

	// Read bitmap bits (remainder of file)
	cbBmpBits = bmfh.bfSize - ftell(pfile);
	pbBmpBits = malloc(cbBmpBits);
	if (fread(pbBmpBits, cbBmpBits, 1/*count*/, pfile) != 1)
		{ rc = -7; goto GetOut; }

	// Set output parameters
	*ppbPalette = malloc(sizeof rgrgbPalette);
	memcpy(*ppbPalette, rgrgbPalette, cbPalBytes);
	*ppbBits = pbBmpBits;


    *pwidth = bmih.biWidth;
    *pheight = bmih.biHeight;

	printf("w %d h %d s %d\n",bmih.biWidth, bmih.biHeight, cbBmpBits );

GetOut:
	if (pfile) fclose(pfile);
	return rc;
	}

