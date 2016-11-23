/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

/*
	mkmovie.c

	converts a movie file from the game engine to a collection of bitmaps
*/

#include "windows.h"
#include "cmdlib.h"
#include "movie.h"

//------------------------------------------------------------ Globals

#define MAJOR_VERSION	1
#define MINOR_VERSION	0
#define MAX_FILE		256


char *basename;
int framecnt;

const char *const formatStr = "Format: mkmovie [-basename <name>] <filename>";

//------------------------------------------------------------ Types

typedef struct 
{
	byte b;
	byte g;
	byte r;
} winColor24;

//------------------------------------------------------------ Functions

void PrintError( const char *pStr )
{
	puts( pStr );
	exit( 5 );
}



void ProcessMFRMBlock( HANDLE hf, movieblockheader_t *pHeader )
{
	HANDLE hout;
	BITMAPFILEHEADER hdr;
	BITMAPINFOHEADER bi;
	DWORD imagesize, bytesread, datasize;
	long rowbytes, pelsize;
	short row;
	byte *hp, *data, *movie;
	movieframe_t frame;
	winColor24 *pRGB;
	char outfilename[ MAX_FILE ];

	// Read in header
	if( !ReadFile( hf, (void *) &frame, sizeof(movieframe_t), &bytesread, NULL ) )
		PrintError( "Error reading MFRM header." );
	if( !bytesread )
		PrintError( "Unexpected EOF found." );

	// Read in data
	datasize = pHeader->size - sizeof(movieframe_t);
	data = (byte *) LocalAlloc( LPTR, datasize );

	if( !ReadFile( hf, (void *) data, datasize, &bytesread, NULL ) )
		PrintError( "Error reading MFRM data." );
	if( !bytesread )
		PrintError( "Unexpected EOF found." );

	// NOTE:  Writing 24bpp images
	imagesize = frame.width * frame.height * 3;

	// make the new data
	movie = (byte *) LocalAlloc( LPTR, imagesize );
	pRGB = (winColor24 *) movie;

	
	if (frame.depth == 24)
		pelsize = 3;
	else
		pelsize = 2;

	
	// the 24 bit images from gl are already reversed

	if (frame.depth == 24)
	{
		rowbytes = frame.width * pelsize;
		hp = (byte *) data;

	} else {

		rowbytes = frame.width * pelsize;
		hp = (byte *) data + datasize;
	}

	// Flip the bitmap vertically while we write it.
	for( row=0; row<frame.height; row++ )
	{
		int i;

		// back the pointer up a row;
		if ( frame.depth != 24 ) 
		{
			hp -= rowbytes;
			for( i=0; i<frame.width; i++, hp += pelsize, pRGB++ )
			{
				// Convert to 24bpp, swap r and b
				if( frame.depth == 15 )
				{
					unsigned short nc = *(short *) hp;
					pRGB->r = ( nc & 0x7c00 ) >> 7;
					pRGB->g = ( nc & 0x03e0 ) >> 2;
					pRGB->b = ( nc & 0x001f ) << 3;
				}
				else if ( frame.depth == 16 )
				{
					unsigned short nc = *(short *) hp;
					pRGB->r = ( nc & 0xf800 ) >> 8;
					pRGB->g = ( nc & 0x07e0 ) >> 3;
					pRGB->b = ( nc & 0x001f ) << 3;
				}
			}
			hp -= rowbytes;
		}
		else	// 24-bit frames, just swap & copy
		{
			for( i=0; i<frame.width; i++, hp += pelsize, pRGB++ )
			{
				pRGB->r = hp[0];		// winColor24 is declared in reverse, so swapping is automatic
				pRGB->g = hp[1];
				pRGB->b = hp[2];
			}
		}

	}

	// Create the output file
	sprintf( outfilename, "%s%04d.bmp", basename, framecnt++ );
	printf( "Creating bitmap %s.\n", outfilename );
	hout = CreateFile( outfilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hout == INVALID_HANDLE_VALUE )
		PrintError( "Couldn't create bitmap file for frame." );

	// create file header
	hdr.bfType = 0x4d42;	// 'BM'
	hdr.bfSize = (DWORD) ( sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imagesize );
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;
	hdr.bfOffBits = (DWORD) ( sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) );

	if( !WriteFile( hout, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), (LPDWORD) &bytesread, NULL ) )
		PrintError( "Couldn't write file header to framebitmap." );

	// bitmap header
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = frame.width;
	bi.biHeight = frame.height;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	if( !WriteFile( hout, (LPVOID) &bi, sizeof(BITMAPINFOHEADER), (LPDWORD) &bytesread, NULL ) )
		PrintError( "Couldn't write bitmap header to frame bitmap." );

	// bitmap data
	if( !WriteFile( hout, (LPVOID) movie, imagesize, (LPDWORD) &bytesread, NULL ) )
		PrintError( "Couldn't write bitmap data to frame bitmap." );


	// clean up
	if( !CloseHandle( hout ) )
		PrintError( "Couldn't close bitmap file for frame." );

	LocalFree( (HLOCAL) data );
	LocalFree( (HLOCAL) movie );
}


void ProcessMovieFile( const char *pFilename )
{
	HANDLE hf;
	DWORD bytesread;
	movieblockheader_t header;
	qboolean eof = false;

	printf( "Processing movie %s:\n", pFilename );

	hf = CreateFile( pFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hf == INVALID_HANDLE_VALUE )
		PrintError( "Couldn't open movie file.\n" );

	while( !eof )
	{
		if( !ReadFile( hf, (void *) &header, sizeof(movieblockheader_t), &bytesread, NULL ) )
			PrintError( "Error reading from movie file.\n" );

		// Check for end of file
		if( !bytesread )
			eof = true;
		else
		{
			switch( header.tag )
			{
				case 'MFRM':
					ProcessMFRMBlock( hf, &header );
					break;

				default:
					PrintError( "Unknown block tag.\n" );
			}
		}
	}
	
	if( !CloseHandle( hf ) )
		PrintError( "Error closing movie file.\n" );

	printf( "Done processing movie.\n" );
}



void main( int argc, char *argv[] )
{
	int i;

	printf( "mkmovie v%d.%d (%s) Copyright 1997, valve software L.L.C\n", MAJOR_VERSION, MINOR_VERSION, __DATE__ );
	if( argc < 2 )
		PrintError( formatStr );

	basename = NULL;

	for( i=0; i<argc; i++ )
	{
		if( *argv[i] == '-' )
		{
			if( !strcmp( argv[i], "-basename" ) )
			{
				if( i >= argc - 1 )
					PrintError( formatStr );

				basename = argv[ ++i ];
			}
		}
	}

	if( *argv[ argc - 1 ] == '-' )
		PrintError( formatStr );

	if( !basename )
		basename = argv[ argc - 1 ];
	framecnt = 0;

	ProcessMovieFile( argv[ argc - 1 ] );
}
