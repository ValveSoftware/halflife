//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxBmp.cpp
// implementation: all
// last modified:  Apr 15 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

// lbmlib.c

#include <mx/mxBmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



mxImage *
mxBmpRead (const char *filename)
{
	int i;
	FILE *pfile = 0;
	mxBitmapFileHeader bmfh;
	mxBitmapInfoHeader bmih;
	mxBitmapRGBQuad rgrgbPalette[256];
	int cbBmpBits;
	byte *pbBmpBits;
	byte *pb, *pbHold;
	int cbPalBytes;
	int biTrueWidth;
	mxImage *image = 0;

	// File exists?
	if ((pfile = fopen (filename, "rb")) == 0)
		return 0;
	
	// Read file header
	if (fread (&bmfh, sizeof bmfh, 1/*count*/, pfile) != 1)
		goto GetOut;

	// Bogus file header check
	if (!(bmfh.bfReserved1 == 0 && bmfh.bfReserved2 == 0))
		goto GetOut;

	// Read info header
	if (fread (&bmih, sizeof bmih, 1/*count*/, pfile) != 1)
		goto GetOut;

	// Bogus info header check
	if (!(bmih.biSize == sizeof bmih && bmih.biPlanes == 1))
		goto GetOut;

	// Bogus bit depth?  Only 8-bit supported.
	if (bmih.biBitCount != 8)
		goto GetOut;

	// Bogus compression?  Only non-compressed supported.
	if (bmih.biCompression != 0) //BI_RGB)
		goto GetOut;

	// Figure out how many entires are actually in the table
	if (bmih.biClrUsed == 0)
	{
		bmih.biClrUsed = 256;
		cbPalBytes = (1 << bmih.biBitCount) * sizeof (mxBitmapRGBQuad);
	}
	else 
	{
		cbPalBytes = bmih.biClrUsed * sizeof (mxBitmapRGBQuad);
	}

	// Read palette (bmih.biClrUsed entries)
	if (fread (rgrgbPalette, cbPalBytes, 1/*count*/, pfile) != 1)
		goto GetOut;

	image = new mxImage ();
	if (!image)
		goto GetOut;

	if (!image->create (bmih.biWidth, bmih.biHeight, 8))
	{
		delete image;
		goto GetOut;
	}

	pb = (byte *) image->palette;

	// Copy over used entries
	for (i = 0; i < (int) bmih.biClrUsed; i++)
	{
		*pb++ = rgrgbPalette[i].rgbRed;
		*pb++ = rgrgbPalette[i].rgbGreen;
		*pb++ = rgrgbPalette[i].rgbBlue;
	}

	// Fill in unused entires will 0,0,0
	for (i = bmih.biClrUsed; i < 256; i++) 
	{
		*pb++ = 0;
		*pb++ = 0;
		*pb++ = 0;
	}

	// Read bitmap bits (remainder of file)
	cbBmpBits = bmfh.bfSize - ftell (pfile);

	pbHold = pb = (byte *) malloc (cbBmpBits * sizeof (byte));
	if (pb == 0)
	{
		delete image;
		goto GetOut;
	}

	if (fread (pb, cbBmpBits, 1/*count*/, pfile) != 1)
	{
		free (pbHold);
		delete image;
		goto GetOut;
	}
/*
	pbBmpBits = malloc(cbBmpBits);
	if (pbBmpBits == 0)
	{
		free (pb);
		free (pbPal);
		goto GetOut;
	}
*/
	pbBmpBits = (byte *) image->data;

	// data is actually stored with the width being rounded up to a multiple of 4
	biTrueWidth = (bmih.biWidth + 3) & ~3;

	// reverse the order of the data.
	pb += (bmih.biHeight - 1) * biTrueWidth;
	for(i = 0; i < bmih.biHeight; i++)
	{
		memmove (&pbBmpBits[bmih.biWidth * i], pb, bmih.biWidth);
		pb -= biTrueWidth;
	}

	//pb += biTrueWidth;
	free (pbHold);

GetOut:

	if (pfile) 
		fclose (pfile);

	return image;
}



bool
mxBmpWrite (const char *filename, mxImage *image)
{
	int i;
	FILE *pfile = 0;
	mxBitmapFileHeader bmfh;
	mxBitmapInfoHeader bmih;
	mxBitmapRGBQuad rgrgbPalette[256];
	int cbBmpBits;
	byte *pbBmpBits;
	byte *pb;
	int cbPalBytes;
	int biTrueWidth;

	if (!image || !image->data || !image->palette)
		return false;

	// File exists?
	if ((pfile = fopen(filename, "wb")) == 0)
		return false;

	biTrueWidth = ((image->width + 3) & ~3);
	cbBmpBits = biTrueWidth * image->height;
	cbPalBytes = 256 * sizeof (mxBitmapRGBQuad);

	// Bogus file header check
	//bmfh.bfType = MAKEWORD( 'B', 'M' );
	bmfh.bfType = (word) (('M' << 8) | 'B');
	bmfh.bfSize = sizeof bmfh + sizeof bmih + cbBmpBits + cbPalBytes;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof bmfh + sizeof bmih + cbPalBytes;

	// Write file header
	if (fwrite (&bmfh, sizeof bmfh, 1/*count*/, pfile) != 1)
	{
		fclose (pfile);
		return false;
	}

	// Size of structure
	bmih.biSize = sizeof bmih;
	// Width
	bmih.biWidth = biTrueWidth;
	// Height
	bmih.biHeight = image->height;
	// Only 1 plane 
	bmih.biPlanes = 1;
	// Only 8-bit supported.
	bmih.biBitCount = 8;
	// Only non-compressed supported.
	bmih.biCompression = 0; //BI_RGB;
	bmih.biSizeImage = 0;

	// huh?
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;

	// Always full palette
	bmih.biClrUsed = 256;
	bmih.biClrImportant = 0;
	
	// Write info header
	if (fwrite (&bmih, sizeof bmih, 1/*count*/, pfile) != 1)
	{
		fclose (pfile);
		return false;
	}
	

	// convert to expanded palette
	pb = (byte *) image->palette;

	// Copy over used entries
	for (i = 0; i < (int) bmih.biClrUsed; i++)
	{
		rgrgbPalette[i].rgbRed   = *pb++;
		rgrgbPalette[i].rgbGreen = *pb++;
		rgrgbPalette[i].rgbBlue  = *pb++;
        rgrgbPalette[i].rgbReserved = 0;
	}

	// Write palette (bmih.biClrUsed entries)
	cbPalBytes = bmih.biClrUsed * sizeof (mxBitmapRGBQuad);
	if (fwrite (rgrgbPalette, cbPalBytes, 1/*count*/, pfile) != 1)
	{
		fclose (pfile);
		return false;
	}

	pbBmpBits = (byte *) malloc (cbBmpBits * sizeof (byte));
	if (!pbBmpBits)
	{
		fclose (pfile);
		return false;
	}

	pb = (byte *) image->data;
	// reverse the order of the data.
	pb += (image->height - 1) * image->width;
	for(i = 0; i < bmih.biHeight; i++)
	{
		memmove (&pbBmpBits[biTrueWidth * i], pb, image->width);
		pb -= image->width;
	}

	// Write bitmap bits (remainder of file)
	if (fwrite (pbBmpBits, cbBmpBits, 1/*count*/, pfile) != 1)
	{
		free (pbBmpBits);
		fclose (pfile);
		return false;
	}

	free (pbBmpBits);
	fclose (pfile);

	return true;
}