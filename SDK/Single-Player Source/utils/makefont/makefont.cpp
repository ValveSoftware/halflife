/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#define _NOENUMQBOOL

#include <windows.h>

extern "C" 
{
	#include "cmdlib.h"
	#include "wadlib.h"
}
#include "qfont.h"

#define DEFAULT_FONT "Arial"

#define FONT_TAG	6  // Font's are the 6th tag after the TYP_LUMPY base ( 64 )...i.e., type == 70

BOOL		bItalic = FALSE;
BOOL		bBold   = FALSE;
BOOL		bUnderline = FALSE;

char		fontname[ 256 ];
int			pointsize[3] = { 9, 11, 15 };

/*
=================
zeromalloc

Allocates and zeroes memory
=================
*/
void *zeromalloc( size_t size )
{
	unsigned char *pbuffer;
	pbuffer = ( unsigned char * )malloc( size );
	if ( !pbuffer )
	{
		printf( "Failed on allocation of %i bytes", size );
		exit( -1 );
	}

	memset( pbuffer, 0, size );
	return ( void * )pbuffer;
}

/*
=================
Draw_SetupConsolePalette

Set's the palette to full brightness ( 192 ) and 
set's up palette entry 0 -- black
=================
*/
void Draw_SetupConsolePalette( unsigned char *pal )
{
	unsigned char *pPalette;
	int i;

	pPalette = pal;

	*(short *)pPalette = 3 * 256;
	pPalette += sizeof( short );

	for ( i = 0; i < 256; i++ )
	{
		pPalette[3 * i + 0 ] = i;
		pPalette[3 * i + 1 ] = i;
		pPalette[3 * i + 2 ] = i;
	}

	// Set palette zero correctly
	pPalette[ 0 ] = 0;
	pPalette[ 1 ] = 0;
	pPalette[ 2 ] = 0;
}

/*
=================
CreateConsoleFont

Renders TT font into memory dc and creates appropriate qfont_t structure
=================
*/

// YWB:  Sigh, VC 6.0's global optimizer causes weird stack fixups in release builds.  Disable the globabl optimizer for this function.
#pragma optimize( "g", off )
qfont_t *CreateConsoleFont( char *pszFont, int nPointSize, BOOL bItalic, BOOL bUnderline, BOOL bBold, int *outsize )
{
	HDC hdc;
	HDC hmemDC;
	HBITMAP hbm, oldbm;
	RECT rc;	
	HFONT fnt, oldfnt;
	int startchar = 32;
	int c;
	int i, j;
	int x, y;
	int nScans;
	unsigned char *bits;
	BITMAPINFO tempbmi;
	BITMAPINFO *pbmi;
	BITMAPINFOHEADER *pbmheader;
	unsigned char *pqdata;
	unsigned char *pCur;
	int x1, y1;
	unsigned char *pPalette;
	qfont_t *pqf = NULL;
	int fullsize;
	int w = 16;
	int h = (128-32)/16;
	int charheight = nPointSize + 5;
	int charwidth = 16;
	RECT rcChar;

	// Create the font
	fnt = CreateFont( -nPointSize, 0, 0, 0, bBold ? FW_HEAVY : FW_MEDIUM, bItalic, bUnderline, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH | FF_DONTCARE, pszFont );

	bits = NULL;

	fullsize = sizeof( qfont_t ) - 4 + ( 128 * w * charwidth ) + sizeof(short) + 768 + 64;

	// Store off final size
	*outsize = fullsize;

	pqf = ( qfont_t * )zeromalloc( fullsize );
	pqdata = (unsigned char *)pqf + sizeof( qfont_t ) - 4;

	pPalette = pqdata + ( 128 * w * charwidth );

	// Configure palette
	Draw_SetupConsolePalette( pPalette );

	hdc		= GetDC( NULL );
	hmemDC	= CreateCompatibleDC( hdc );

	rc.top		= 0;
	rc.left		= 0;
	rc.right	= charwidth  * w;
	rc.bottom	= charheight * h;

	hbm		= CreateBitmap( charwidth * w, charheight * h, 1, 1, NULL ); 
	oldbm	= (HBITMAP)SelectObject( hmemDC, hbm );
	oldfnt	= (HFONT)SelectObject( hmemDC, fnt );

	SetTextColor( hmemDC, 0x00ffffff );
	SetBkMode( hmemDC, TRANSPARENT );

	// Paint black background
	FillRect( hmemDC, &rc, (HBRUSH)GetStockObject( BLACK_BRUSH ) );
   
	// Draw character set into memory DC
	for ( j = 0; j < h; j++ )
	{
		for ( i = 0; i < w; i++ )
		{
			x = i * charwidth;
			y = j * charheight;

			c = (char)( startchar + j * w + i );

			// Only draw printable characters, of course
			if ( isprint( c ) && c <= 127 )
			{
				// Draw it.
				rcChar.left		= x + 1;
				rcChar.top		= y + 1;
				rcChar.right	= x + charwidth - 1;
				rcChar.bottom	= y + charheight - 1;
				
				DrawText( hmemDC, (char *)&c, 1, &rcChar, DT_NOPREFIX | DT_LEFT );
			}
		}
	}

	// Now turn the qfont into raw format
	memset( &tempbmi, 0, sizeof( BITMAPINFO ) );
	pbmheader = ( BITMAPINFOHEADER * )&tempbmi;

	pbmheader->biSize	= sizeof( BITMAPINFOHEADER );
	pbmheader->biWidth	= w * charwidth; 
	pbmheader->biHeight	= -h * charheight; 
	pbmheader->biPlanes	= 1;
	pbmheader->biBitCount = 1;
	pbmheader->biCompression = BI_RGB;

	// Find out how big the bitmap is
	nScans = GetDIBits( hmemDC, hbm, 0, h * charheight, NULL, &tempbmi, DIB_RGB_COLORS );

	// Allocate space for all bits
	pbmi = ( BITMAPINFO * )zeromalloc( sizeof ( BITMAPINFOHEADER ) + 2 * sizeof( RGBQUAD ) + pbmheader->biSizeImage );

	memcpy( pbmi, &tempbmi, sizeof( BITMAPINFO ) );

	bits = ( unsigned char * )pbmi + sizeof( BITMAPINFOHEADER ) + 2 * sizeof( RGBQUAD ); 

	// Now read in bits
	nScans = GetDIBits( hmemDC, hbm, 0, h * charheight, bits, pbmi, DIB_RGB_COLORS );

	if ( nScans > 0 )
	{
		// Now convert to proper raw format
		//
		// Now get results from dib
		pqf->height = 128; // Always set to 128
		pqf->width = charwidth;
		pqf->rowheight = charheight;
		pqf->rowcount = h;
		pCur = pqdata;
		
		// Set everything to index 255 ( 0xff ) == transparent
		memset( pCur, 0xFF, w * charwidth * pqf->height );

		for ( j = 0; j < h; j++ )
		{
			for ( i = 0; i < w; i++ )
			{
				int edge = 1;
				int bestwidth;
				x = i * charwidth;
				y = j * charheight;


				c = (char)( startchar + j * w + i );

				pqf->fontinfo[ c ].charwidth = charwidth;
				pqf->fontinfo[ c ].startoffset = y * w * charwidth + x;

				bestwidth = 0;

				// In this first pass, place the black drop shadow so characters draw ok in the engine against
				//  most backgrounds.
				// YWB:  FIXME, apply a box filter and enable blending?

				// Make it all transparent for starters
				for ( y1 = 0; y1 < charheight; y1++ )
				{
					for ( x1 = 0; x1 < charwidth; x1++ )
					{
						int offset;
						offset = ( y + y1 ) * w * charwidth + x + x1 ;
						// Dest
						pCur = pqdata + offset;
						// Assume transparent
						pCur[0] = 255;
					}
				}

				// Put black pixels below and to the right of each pixel
				for ( y1 = edge; y1 < charheight - edge; y1++ )
				{
					for ( x1 = 0; x1 < charwidth; x1++ )
					{
						int offset;
						int srcoffset;

						int xx0, yy0;

						offset = ( y + y1 ) * w * charwidth + x + x1 ;

						// Dest
						pCur = pqdata + offset;

						for ( xx0 = -edge; xx0 <= edge; xx0++ )
						{
							for ( yy0 = -edge; yy0 <= edge; yy0++ )
							{
								srcoffset = ( y + y1 + yy0 ) * w * charwidth + x + x1 + xx0;

								if ( bits[ srcoffset >> 3 ] & ( 1 << ( 7 - srcoffset & 7 ) ) )
								{
									// Near Black
									pCur[0] = 32;
								}
							}
						}
					}
				}

				// Now copy in the actual font pixels
				for ( y1 = 0; y1 < charheight; y1++ )
				{
					for ( x1 = 0; x1 < charwidth; x1++ )
					{
						int offset;

						offset = ( y + y1 ) * w * charwidth + x + x1;

						// Dest
						pCur = pqdata + offset;

						if ( bits[ offset >> 3 ] & ( 1 << ( 7 - offset & 7 ) ) )
						{
							if ( x1 > bestwidth )
							{
								bestwidth = x1;
							}

							// Full color
							// FIXME:  Enable true palette support in engine?
							pCur[0] = 192;
						}
					}
				}

				// bestwidth += 1;
				/*
				// Now blend it
				for ( y1 = 0; y1 < charheight; y1++ )
				{
					for ( x1 = 0; x1 < charwidth; x1++ )
					{
						int offset;

						offset = ( y + y1 ) * w * charwidth + x + x1;

						// Dest
						pCur = pqdata + offset;

						if ( bits[ offset >> 3 ] & ( 1 << ( 7 - offset & 7 ) ) )
						{
							if ( x1 > bestwidth )
							{
								bestwidth = x1;
							}

							// Full color
							// FIXME:  Enable true palette support in engine?
							pCur[0] = 192;
						}
					}
				}
				*/

				// Space character width
				if ( c == 32 )
				{
					bestwidth = 8;
				}
				else
				{
					// Small characters needs can be padded a bit so they don't run into each other
					if ( bestwidth <= 14 )
					{
						bestwidth += 2;
					}
				}
				
				// Store off width
				pqf->fontinfo[ c ].charwidth = bestwidth;
			}
		}
	}

	// Free memory bits
	free ( pbmi );

	SelectObject( hmemDC, oldfnt );
	DeleteObject( fnt );

	SelectObject( hmemDC, oldbm );

	DeleteObject( hbm );

	DeleteDC( hmemDC );
	ReleaseDC( NULL, hdc );

	return pqf;
}

#pragma optimize( "g", on )

/*
=================
main

=================
*/
int main(int argc, char* argv[])
{
	int		i;
	DWORD	start, end;
	char	destfile[1024];
	char	sz[ 32 ];
	int		outsize[ 3 ];

	qfont_t *fonts[ 3 ];

	strcpy( fontname, DEFAULT_FONT );

	printf("makefont.exe Version 1.0 by valve (%s)\n", __DATE__ );
	
	printf ("----- Creating Console Font ----\n");

	for (i=1 ; i<argc ; i++)
	{
		if (!strcmp(argv[i],"-font"))
		{
			strcpy( fontname, argv[i+1] );
			i++;
		}
		else if (!strcmp(argv[i],"-pointsizes"))
		{
			if ( i + 3 >= argc )
			{
				Error( "Makefont:  Insufficient point sizes specified\n" );
			}
			pointsize[0] = atoi( argv[i+1] );
			pointsize[1] = atoi( argv[i+2] );
			pointsize[2] = atoi( argv[i+3] );
			i += 3;
		}
		else if (!strcmp(argv[i],"-italic"))
		{
			bItalic = TRUE;
			printf ( "italic set\n");
		}
		else if (!strcmp(argv[i],"-bold"))
		{
			bBold = TRUE;
			printf ( "bold set\n");
		}
		else if (!strcmp(argv[i],"-underline"))
		{
			bUnderline = TRUE;
			printf ( "underline set\n");
		}
		else if ( argv[i][0] == '-' )
		{
			Error ("Unknown option \"%s\"", argv[i]);
		}
		else
			break;
	}

	if ( i != argc - 1 )
	{
		Error ("usage: makefont [-font \"fontname\"] [-italic] [-underline] [-bold] [-pointsizes sm med lg] outfile");
	}

	printf( "Creating %i, %i, and %i point %s fonts\n", pointsize[0], pointsize[1], pointsize[2], fontname );

	start = timeGetTime();

	// Create the fonts
	for ( i = 0 ; i < 3; i++ )
	{
		fonts[ i ] = CreateConsoleFont( fontname, pointsize[i], bItalic, bUnderline, bBold, &outsize[ i ] );
	}

	// Create wad file
	strcpy (destfile, argv[argc - 1]);
	StripExtension (destfile);
	DefaultExtension (destfile, ".wad");

	NewWad( destfile, false );

	// Add fonts as lumps
	for ( i = 0; i < 3; i++ )
	{
		sprintf( sz, "font%i", i );
		AddLump( sz, fonts[ i ], outsize[ i ], TYP_LUMPY + FONT_TAG, false );
	}

	// Store results as a WAD3
	// NOTE:  ( should be named fonts.wad in the valve\ subdirectory )
	WriteWad( 3 );

	// Clean up memory
	for ( i = 0 ; i < 3; i++ )
	{
		free( fonts[ i ] );
	}

	end = timeGetTime ();

	printf ( "%5.5f seconds elapsed\n", (float)( end - start )/1000.0 );
	
	// Display for a second since it might not be running from command prompt
	Sleep( 1000 );
	return 0;
}
