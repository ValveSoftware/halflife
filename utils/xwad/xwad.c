#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/file.h>
#include <stdarg.h>

#include "cmdlib.h"

#include "wadlib.h"
// #include "csg.h"
#include "lbmlib.h"
#include "mathlib.h"
#include "bsplib.h"

#define MAXLUMP (640*480*85/64)

byte inbuffer[MAXLUMP];
byte outbuffer[(640+320)*480];

extern FILE *wadhandle;


int main (int argc, char **argv)
{
	int i, j;
	miptex_t *qtex;
	int t;
	int width, height;
	int width2;
	byte *psrc, *pdest;
	char filename[1024];

	if (argc < 2) {
		printf("%s <wadfile> [image names]\n", argv[0] );
		exit(1);
	}

	W_OpenWad( argv[1] );

	for (i = 0; i < numlumps; i++) {
		for (j = 2; j < argc; j++) {
			if (stricmp( lumpinfo[i].name, argv[j] ) == 0)
				break;
		}
		if (argc == 2 || j < argc) {
			printf("extracting %s @ %d  size %d\n", lumpinfo[i].name, lumpinfo[i].filepos, lumpinfo[i].size );
			fseek( wadhandle, lumpinfo[i].filepos, SEEK_SET );
			SafeRead ( wadhandle, inbuffer, lumpinfo[i].size );

			qtex = (miptex_t *)inbuffer;
			width = LittleLong(qtex->width);
			height = LittleLong(qtex->height);

			// make a composite image
			width2 = width + width / 2;
			memset( outbuffer, 0, height * width2 );
			
			// copy in 0 image
			psrc = inbuffer + LittleLong( qtex->offsets[0] );
			pdest = outbuffer;
			for (t = 0; t < height; t++) {
				memcpy( pdest + t * width2, psrc + t * width, width );
			}

			// copy in 1 image
			psrc = inbuffer + LittleLong( qtex->offsets[1] );
			pdest = outbuffer + width;
			for (t = 0; t < height / 2; t++) {
				memcpy( pdest + t * width2, psrc + t * width / 2, width  / 2);
			}

			// copy in 2 image
			psrc = inbuffer + LittleLong( qtex->offsets[2] );
			pdest = outbuffer + (height / 2 ) * width2 + width;
			for (t = 0; t < height / 4; t++) {
				memcpy( pdest + t * width2, psrc + t * width / 4, width  / 4);
			}

			// copy in 3 image
			psrc = inbuffer + LittleLong( qtex->offsets[3] );
			pdest = outbuffer + (height / 2 ) * width2 + width + width / 4;
			for (t = 0; t < height / 8; t++) {
				memcpy( pdest + t * width2, psrc + t * width / 8, width  / 8);
			}

			sprintf( filename, "%s.bmp", qtex->name );

			WriteBMPfile( filename, outbuffer, width2, height, inbuffer + LittleLong( qtex->offsets[3] ) + width * height / 64 + 2 );
		}
	}

	return 0;
}	


/*

	qtex = (miptex_t *)lump_p;
	qtex->width = LittleLong(w);
	qtex->height = LittleLong(h);
	strcpy (qtex->name, lumpname); 
	
	lump_p = (byte *)&qtex->offsets[4];
	
	screen_p = byteimage + yl*byteimagewidth + xl;
	linedelta = byteimagewidth - w;

	source = lump_p;
	qtex->offsets[0] = LittleLong(lump_p - (byte *)qtex);

	for (y=yl ; y<yh ; y++)
	{
		for (x=xl ; x<xh ; x++)
		{
			pix = *screen_p;
			*screen_p++ = 0;
//			if (pix == 255)
//				pix = 0;
			*lump_p++ = pix;
		}
		screen_p += linedelta;
	}
	
	//
	// subsample for greater mip levels
	//
	d_red = d_green = d_blue = 0;	// no distortion yet

	for (miplevel = 1 ; miplevel<4 ; miplevel++)
	{
		qtex->offsets[miplevel] = LittleLong(lump_p - (byte *)qtex);
		
		mipstep = 1<<miplevel;
		for (y=0 ; y<h ; y+=mipstep)
		{

			for (x = 0 ; x<w ; x+= mipstep)
			{
				count = 0;
				for (yy=0 ; yy<mipstep ; yy++)
					for (xx=0 ; xx<mipstep ; xx++)
					{
						pixdata[count] = source[ (y+yy)*w + x + xx ];
						count++;
					}
				*lump_p++ = AveragePixels (count);
			}	
		}
	}

	if( do16bit )
		GrabPalette16();
*/