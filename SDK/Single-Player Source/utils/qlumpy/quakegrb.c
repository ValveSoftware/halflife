/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#include "qlumpy.h"
#include "math.h"

#pragma warning (disable : 4244)

typedef struct
{
	short	ofs, length;
} row_t;

typedef struct
{
	int		width, height;
	int		widthbits, heightbits;
	unsigned char	data[4];
} qtex_t;

typedef struct
{
	int			width, height;
	byte		data[4];			// variably sized
} qpic_t;


// Font stuff

#define NUM_GLYPHS 256
const unsigned kFontMarker = 254;

typedef struct
{
	short startoffset;
	short charwidth;
} charinfo;

typedef struct
{
	int 		width, height;
	int			rowcount;
	int			rowheight;
	charinfo	fontinfo[ NUM_GLYPHS ];
	byte 		data[4];
} qfont_t;


extern qboolean		fTransparent255;


#define SCRN(x,y)       (*(byteimage+(y)*byteimagewidth+x))

void GrabPalette16( void );

extern qboolean do16bit;

/*
==============
GrabRaw

filename RAW x y width height
==============
*/
void GrabRaw (void)
{
	int             x,y,xl,yl,xh,yh,w,h;
	byte            *screen_p;
	int             linedelta;

	GetToken (false);
	xl = atoi (token);
	GetToken (false);
	yl = atoi (token);
	GetToken (false);
	w = atoi (token);
	GetToken (false);
	h = atoi (token);

	if (xl == -1)
	{
		xl = yl = 0;
		w = byteimagewidth;
		h = byteimageheight;
	}

	xh = xl+w;
	yh = yl+h;

	screen_p = byteimage + yl*byteimagewidth + xl;
	linedelta = byteimagewidth - w;

	for (y=yl ; y<yh ; y++)
	{
		for (x=xl ; x<xh ; x++)
		{
			*lump_p++ = *screen_p;
			*screen_p++ = 0;
		}
		screen_p += linedelta;
	}
}



/*
==============
GrabPalette

filename PALETTE [startcolor endcolor]
==============
*/
void GrabPalette (void)
{
	int start, end, length;

	if (TokenAvailable())
	{
		GetToken (false);
		start = atoi (token);
		GetToken (false);
		end = atoi (token);
	}
	else
	{
		start = 0;
		end = 255;
	}

	length = 3*(end-start+1);
	memcpy (lump_p, lbmpalette+start*3, length);
	lump_p += length;
}


/*
==============
GrabPic

filename qpic x y width height
==============
*/
void GrabPic (void)
{
	int             x,y,xl,yl,xh,yh;
	int             width;
	qpic_t 			*header;

	GetToken (false);
	xl = atoi (token);
	GetToken (false);
	yl = atoi (token);
	GetToken (false);
	xh = xl+atoi (token);
	GetToken (false);
	yh = yl+atoi (token);

	if (xl == -1)
	{
		xl = yl = 0;
		xh = byteimagewidth;
		yh = byteimageheight;
	}

	if (xh<xl || yh<yl || xl < 0 || yl<0) // || xh>319 || yh>239)
		Error ("GrabPic: Bad size: %i, %i, %i, %i",xl,yl,xh,yh);

	//
	// fill in header
	//
	header = (qpic_t *)lump_p;
	width = xh-xl;
	header->width = LittleLong(width);
	header->height = LittleLong(yh-yl);

	//
	// start grabbing posts
	//
	lump_p = (byte *)header->data;

	for (y=yl ; y< yh ; y++)
		for (x=xl ; x<xh ; x++)
			*lump_p++ = SCRN(x,y);

	// New for 16bpp display
	if( do16bit )
		GrabPalette16();
}

/*
=============================================================================

COLORMAP GRABBING

=============================================================================
*/

/*
===============
BestColor
===============
*/
byte BestColor (int r, int g, int b, int start, int stop)
{
	int	i;
	int	dr, dg, db;
	int	bestdistortion, distortion;
	int	bestcolor;
	byte	*pal;

//
// let any color go to 0 as a last resort
//
	bestdistortion = ( (int)r*r + (int)g*g + (int)b*b )*2;
	bestcolor = 0;

	pal = lbmpalette + start*3;
	for (i=start ; i<= stop ; i++)
	{
		dr = r - (int)pal[0];
		dg = g - (int)pal[1];
		db = b - (int)pal[2];
		pal += 3;
		distortion = dr*dr + dg*dg + db*db;
		if (distortion < bestdistortion)
		{
			if (!distortion)
				return i;		// perfect match

			bestdistortion = distortion;
			bestcolor = i;
		}
	}

	return bestcolor;
}


/*
==============
GrabColormap

filename COLORMAP levels fullbrights
the first map is an identiy 0-255
the final map is all black except for the fullbrights
the remaining maps are evenly spread
fullbright colors start at the top of the palette.
==============
*/
void GrabColormap (void)
{
	int		levels, brights;
	int		l, c;
	float	frac, red, green, blue;
		
	GetToken (false);
	levels = atoi (token);
	GetToken (false);
	brights = atoi (token);

// identity lump
	for (l=0 ; l<256 ; l++)
		*lump_p++ = l;

// shaded levels
	for (l=1;l<levels;l++)
	{
		frac = 1.0 - (float)l/(levels-1);
		for (c=0 ; c<256-brights ; c++)
		{
			red = lbmpalette[c*3];
			green = lbmpalette[c*3+1];
			blue = lbmpalette[c*3+2];

			red = (int)(red*frac+0.5);
			green = (int)(green*frac+0.5);
			blue = (int)(blue*frac+0.5);
			
//
// note: 254 instead of 255 because 255 is the transparent color, and we
// don't want anything remapping to that
//
			*lump_p++ = BestColor(red,green,blue, 0, 254);
		}
		for ( ; c<256 ; c++)
			*lump_p++ = c;
	}
	
	*lump_p++ = brights;
}

/*
==============
GrabColormap2

experimental -- not used by quake

filename COLORMAP2 range levels fullbrights
fullbright colors start at the top of the palette.
Range can be greater than 1 to allow overbright color tables.

the first map is all 0
the last (levels-1) map is at range
==============
*/
void GrabColormap2 (void)
{
	int		levels, brights;
	int		l, c;
	float	frac, red, green, blue;
	float	range;
	
	GetToken (false);
	range = atof (token);
	GetToken (false);
	levels = atoi (token);
	GetToken (false);
	brights = atoi (token);

// shaded levels
	for (l=0;l<levels;l++)
	{
		frac = range - range*(float)l/(levels-1);
		for (c=0 ; c<256-brights ; c++)
		{
			red = lbmpalette[c*3];
			green = lbmpalette[c*3+1];
			blue = lbmpalette[c*3+2];

			red = (int)(red*frac+0.5);
			green = (int)(green*frac+0.5);
			blue = (int)(blue*frac+0.5);
			
//
// note: 254 instead of 255 because 255 is the transparent color, and we
// don't want anything remapping to that
//
			*lump_p++ = BestColor(red,green,blue, 0, 254);
		}

		// fullbrights allways stay the same
		for ( ; c<256 ; c++)
			*lump_p++ = c;
	}
	
	*lump_p++ = brights;
}

/*
=============================================================================

MIPTEX GRABBING

=============================================================================
*/

typedef struct
{
	char		name[16];
	unsigned	width, height;
	unsigned	offsets[4];		// four mip maps stored
} miptex_t;

byte	pixdata[256];

float 	linearpalette[256][3];
float 	d_red, d_green, d_blue;
int		colors_used;
int		color_used[256];
float	maxdistortion;

byte AddColor( float r, float g, float b )
{
	int i;
	for (i = 0; i < 255; i++)
	{
		if (!color_used[i])
		{
			linearpalette[i][0] = r;
			linearpalette[i][1] = g;
			linearpalette[i][2] = b;
			if (r < 0) r = 0.0;
			if (r > 1.0) r = 1.0;
			lbmpalette[i*3+0] = pow( r, 1.0 / 2.2) * 255;
			if (g < 0) g = 0.0;
			if (g > 1.0) g = 1.0;
			lbmpalette[i*3+1] = pow( g, 1.0 / 2.2) * 255;
			if (b < 0) b = 0.0;
			if (b > 1.0) b = 1.0;
			lbmpalette[i*3+2] = pow( b, 1.0 / 2.2) * 255;
			color_used[i] = 1;
			colors_used++;
			return i;
		}
	}
	return 0;
}

/*
=============
AveragePixels
=============
*/
byte AveragePixels (int count)
{
	float   r,g,b;
	int		i;
	int		vis;
	int		pix;
	float 	dr, dg, db;
	float 	bestdistortion, distortion;
	int		bestcolor;
	byte	*pal;
	
	vis = 0;
	r = g = b = 0;

	for (i=0 ; i<count ; i++)
	{
		pix = pixdata[i];
		r += linearpalette[pix][0];
		g += linearpalette[pix][1];
		b += linearpalette[pix][2];
	}

	r /= count;
	g /= count;
	b /= count;

	r += d_red;
	g += d_green;
	b += d_blue;
	
//
// find the best color
//
//	bestdistortion = r*r + g*g + b*b;
	bestdistortion = 3.0;
	bestcolor = -1;

	for ( i=0; i<255; i++)
	{
		if (color_used[i])
		{
			pix = i;	//pixdata[i];

			dr = r - linearpalette[i][0];
			dg = g - linearpalette[i][1];
			db = b - linearpalette[i][2];

			distortion = dr*dr + dg*dg + db*db;
			if (distortion < bestdistortion)
			{
				if (!distortion)
				{
					d_red = d_green = d_blue = 0;	// no distortion yet
					return pix;		// perfect match
				}

				bestdistortion = distortion;
				bestcolor = pix;
			}
		}
	}


	if (bestdistortion > 0.001 && colors_used < 255)
	{
		// printf("%f %f %f\n", r, g, b );
		bestcolor = AddColor( r, g, b );
		d_red = d_green = d_blue = 0;
		bestdistortion = 0;
	}
	else
	{
		// error diffusion
		d_red = r - linearpalette[bestcolor][0];
		d_green = g - linearpalette[bestcolor][1];
		d_blue = b - linearpalette[bestcolor][2];
	}

	if (bestdistortion > maxdistortion)
		maxdistortion = bestdistortion;
	return bestcolor;
}


/*
==============
GrabMip

filename MIP x y width height
must be multiples of sixteen
==============
*/
void GrabMip (void)
{
	int             i,j,x,y,xl,yl,xh,yh,w,h;
	byte            *screen_p, *source, testpixel;
	int             linedelta;
	miptex_t		*qtex;
	int				miplevel, mipstep;
	int				xx, yy, pix;
	int				count;
	
	GetToken (false);
	xl = atoi (token);
	GetToken (false);
	yl = atoi (token);
	GetToken (false);
	w = atoi (token);
	GetToken (false);
	h = atoi (token);

	if (xl == -1)
	{
		xl = yl = 0;
		w = byteimagewidth;
		h = byteimageheight;
	}

	if ( (w & 15) || (h & 15) )
		Error ("line %i: miptex sizes must be multiples of 16", scriptline);

	xh = xl+w;
	yh = yl+h;

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

	// calculate gamma corrected linear palette
	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < 3; j++)
		{
			float f;
			f = lbmpalette[i*3+j] / 255.0;
			linearpalette[i][j] = pow(f, 2.2 ); // assume textures are done at 2.2, we want to remap them at 1.0
		}
	}

	maxdistortion = 0;
	if (!fTransparent255)
	{
		// figure out what palette entries are actually used
		colors_used = 0;
		for (i = 0; i < 256; i++)
			color_used[i] = 0;

		for (x = 0; x < w; x++)
		{
			for (y = 0; y < h; y++)
			{
				if (!color_used[source[ y*w + x]])
				{
					color_used[source[ y*w + x]] = 1;
					colors_used++;
				}
			}
		}
	}
	else
	{
		// assume palette full if it's a transparent texture
		colors_used = 256;
		for (i = 0; i < 256; i++)
			color_used[i] = 1;
	}
	// printf("colors_used %d : ", colors_used );


	//
	// subsample for greater mip levels
	//

	for (miplevel = 1 ; miplevel<4 ; miplevel++)
	{
		int pixTest;
		d_red = d_green = d_blue = 0;	// no distortion yet
		qtex->offsets[miplevel] = LittleLong(lump_p - (byte *)qtex);
		
		mipstep = 1<<miplevel;
		pixTest = (int)( (float)(mipstep * mipstep) * 0.4 );	// 40% of pixels

		for (y=0 ; y<h ; y+=mipstep)
		{
			for (x = 0 ; x<w ; x+= mipstep)
			{
				count = 0;
				for (yy=0 ; yy<mipstep ; yy++)
					for (xx=0 ; xx<mipstep ; xx++)
					{
						testpixel = source[ (y+yy)*w + x + xx ];
						
						// If 255 is not transparent, or this isn't a transparent pixel, add it in to the image filter
						if ( !fTransparent255 || testpixel != 255 ) {
							pixdata[count] = testpixel;
							count++;
						}
					}
				if ( count <= pixTest )	// Solid pixels account for < 40% of this pixel, make it transparent
				{
					*lump_p++ = 255;
				}
				else
				{
					*lump_p++ = AveragePixels (count);
				}
			}	
		}
	}

	// printf(" %d %f\n", colors_used, maxdistortion );

	if( do16bit )
		GrabPalette16();
}


/*
=============================================================================

PALETTE GRABBING

=============================================================================
*/


void GrabPalette16( void )
{
	int i;

	// Write out palette in 16bit mode
	*(unsigned short *) lump_p = 256;	// palette size
	lump_p += sizeof(short);

	memcpy( lump_p, lbmpalette, 768 );
	lump_p += 768;
}



/*
=============================================================================

FONT GRABBING

=============================================================================
*/


/*
==============
GrabFont

font x y width height startglyph
==============
*/
void GrabFont( void )
{
	int		x, y, y2, xl, x2, yl, xh, yh, i, j;
	int		index, offset;
	int		width;
	int		iCurX;	// current x in destination
	int		iMaxX;  // max x in destination
	
	byte	*pbuf, *pCur;
	qfont_t 			*header;


	iMaxX = 255;
	iCurX = 0;

	// Set up header
	header = (qfont_t *)lump_p;
	memset( header, 0, sizeof(qfont_t) );

	GetToken( false );
	header->width = header->rowheight = atoi( token );  //mwh why does width equal rowheight? 
	header->height = 1;
	lump_p = (byte *)header->data;
	pCur = (byte *)lump_p;
	memset( lump_p, 0xFF, 256 * 160);

	GetToken( false );
	index = atoi( token );

	while( index != -1 )
	{
		// Get/Process source bitmap coordinates
		GetToken (false);
		xl = atoi (token);
		GetToken (false);
		yl = atoi (token);
		GetToken (false);
		xh = xl-1+atoi (token);
		GetToken (false);
		yh = atoi (token) - 1;
		if (xl == -1)
		{
			xl = yl = 0;
			xh = byteimagewidth;
			yh = byteimageheight;
		}

		if( xh<xl || yh<yl || xl < 0 || yl<0 )
			Error( "GrabFont line %1: Bad size: %i, %i, %i, %i", scriptline, xl, yl, xh, yh );

		//
		// Fill in font information
		// Create a bitmap that is up to 256 wide and as tall as we need to accomadate the font.
		// We limit the bitmap to 256 because some 3d boards have problems with textures bigger 
		// than that. 
		//
		for( y=yl; y<yh; y+=header->rowheight+1 )
		{
			// Make sure we're at a marker
			if( y != yl )
			{
				for( y2=y-header->rowheight; y2<yh; y2++ )
					if( kFontMarker == (unsigned) SCRN(xl,y2) )
						break;

				if( y2 == yh )
					break;
				else if( y2 != y )
					Error( "GrabFont line %d: rowheight doesn't seem to match bitmap (%d, %d)\n", scriptline, y, y2 );
			}

			for( x=xl; x<xh; )
			{
				// find next marker
				for( x2=x+1; x2<xh; x2++ )
					if( kFontMarker == (unsigned) SCRN(x2,y) )
						break;

				// check for end of row
				if( x2 == xh )
					break;

				// Set up glyph information
				if( index >= NUM_GLYPHS )
				{
					printf( "GrabFont: Glyph out of range\n" );
					goto getout;
				}
		
				// Fill in glyph info
				header->fontinfo[ index ].charwidth = x2 - x - 1;
				
				// update header				

				// output glyph data
				iCurX += header->fontinfo[index].charwidth;
				
				// Will this glyph fit on this row?
				if (iCurX >= iMaxX)
				{	
					// Nope -- move to next row
					pCur = (byte *)lump_p + 256 * header->rowheight * header->height;
					header->height++;
					iCurX = header->fontinfo[index].charwidth;
				} 
			
				// copy over the glyph bytes
				pbuf = pCur;
				header->fontinfo[ index ].startoffset = pCur - (byte *) header->data;
				

				for(j = 1; j <= header->rowheight; j++)
				{
					byte *psrc = byteimage + (y + j) * byteimagewidth + (x + 1);

					for(i = x + 1; i < x2; i++)
						*pbuf++ = *psrc++;

					pbuf = pCur + j * 256;
				}
				
				// move the lump pointer to point at the next possible glyph
				pCur += header->fontinfo[index].charwidth;
				x = x2;
				index++;
			}
		}

		// Get next ASCII index
getout:
		GetToken (false);
		index = atoi (token);
	}

	// advance the lump pointer so that the last row is saved.
	lump_p += (256 * header->rowheight) * header->height;
	
	// JAY: Round up to the next power of 2 for GL
	offset = header->height * header->rowheight;

	y = (offset>128)?256:(offset>64)?128:(offset>32)?64:(offset>16)?32:16;
	if ( offset != y )
	{
		printf("Rounding font from 256x%d to 256x%d\n", offset, y );
		lump_p += (256 * (y - offset));
	}
	header->rowcount = header->height;
	header->height = y;

	if( do16bit )
		GrabPalette16();
}

