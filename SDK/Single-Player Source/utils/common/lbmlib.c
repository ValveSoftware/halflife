/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

// lbmlib.c

#include <WINDOWS.H>
#include <STDIO.H>

#include "cmdlib.h"
#include "lbmlib.h"



/*
============================================================================

						LBM STUFF

============================================================================
*/


#define FORMID ('F'+('O'<<8)+((int)'R'<<16)+((int)'M'<<24))
#define ILBMID ('I'+('L'<<8)+((int)'B'<<16)+((int)'M'<<24))
#define PBMID  ('P'+('B'<<8)+((int)'M'<<16)+((int)' '<<24))
#define BMHDID ('B'+('M'<<8)+((int)'H'<<16)+((int)'D'<<24))
#define BODYID ('B'+('O'<<8)+((int)'D'<<16)+((int)'Y'<<24))
#define CMAPID ('C'+('M'<<8)+((int)'A'<<16)+((int)'P'<<24))


bmhd_t  bmhd;

int    Align (int l)
{
	if (l&1)
		return l+1;
	return l;
}



/*
================
=
= LBMRLEdecompress
=
= Source must be evenly aligned!
=
================
*/

byte  *LBMRLEDecompress (byte *source,byte *unpacked, int bpwidth)
{
	int     count;
	byte    b,rept;

	count = 0;

	do
	{
		rept = *source++;

		if (rept > 0x80)
		{
			rept = (rept^0xff)+2;
			b = *source++;
			memset(unpacked,b,rept);
			unpacked += rept;
		}
		else if (rept < 0x80)
		{
			rept++;
			memcpy(unpacked,source,rept);
			unpacked += rept;
			source += rept;
		}
		else
			rept = 0;               // rept of 0x80 is NOP

		count += rept;

	} while (count<bpwidth);

	if (count>bpwidth)
		Error ("Decompression exceeded width!\n");


	return source;
}


#define BPLANESIZE      128
byte    bitplanes[9][BPLANESIZE];       // max size 1024 by 9 bit planes


/*
=================
=
= MungeBitPlanes8
=
= This destroys the bit plane data!
=
=================
*/

void MungeBitPlanes8 (int width, byte *dest)
{
	*dest=width;	// shut up the compiler warning
	Error ("MungeBitPlanes8 not rewritten!");
#if 0
asm     les     di,[dest]
asm     mov     si,-1
asm     mov     cx,[width]
mungebyte:
asm     inc     si
asm     mov     dx,8
mungebit:
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*7 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*6 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*5 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*4 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*3 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*2 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*1 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*0 +si],1
asm     rcl     al,1
asm     stosb
asm     dec     cx
asm     jz      done
asm     dec     dx
asm     jnz     mungebit
asm     jmp     mungebyte

done:
#endif
}


void MungeBitPlanes4 (int width, byte *dest)
{
	*dest=width;	// shut up the compiler warning
	Error ("MungeBitPlanes4 not rewritten!");
#if 0

asm     les     di,[dest]
asm     mov     si,-1
asm     mov     cx,[width]
mungebyte:
asm     inc     si
asm     mov     dx,8
mungebit:
asm     xor     al,al
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*3 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*2 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*1 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*0 +si],1
asm     rcl     al,1
asm     stosb
asm     dec     cx
asm     jz      done
asm     dec     dx
asm     jnz     mungebit
asm     jmp     mungebyte

done:
#endif
}


void MungeBitPlanes2 (int width, byte *dest)
{
	*dest=width;	// shut up the compiler warning
	Error ("MungeBitPlanes2 not rewritten!");
#if 0
asm     les     di,[dest]
asm     mov     si,-1
asm     mov     cx,[width]
mungebyte:
asm     inc     si
asm     mov     dx,8
mungebit:
asm     xor     al,al
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*1 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*0 +si],1
asm     rcl     al,1
asm     stosb
asm     dec     cx
asm     jz      done
asm     dec     dx
asm     jnz     mungebit
asm     jmp     mungebyte

done:
#endif
}


void MungeBitPlanes1 (int width, byte *dest)
{
	*dest=width;	// shut up the compiler warning
	Error ("MungeBitPlanes1 not rewritten!");
#if 0
asm     les     di,[dest]
asm     mov     si,-1
asm     mov     cx,[width]
mungebyte:
asm     inc     si
asm     mov     dx,8
mungebit:
asm     xor     al,al
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*0 +si],1
asm     rcl     al,1
asm     stosb
asm     dec     cx
asm     jz      done
asm     dec     dx
asm     jnz     mungebit
asm     jmp     mungebyte

done:
#endif
}

int LoadBMP (const char* szFile, BYTE** ppbBits, BYTE** ppbPalette)
{
	int i, rc = 0;
	FILE *pfile = NULL;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	RGBQUAD rgrgbPalette[256];
	ULONG cbBmpBits;
	BYTE* pbBmpBits;
	byte  *pb, *pbPal = NULL;
	ULONG cbPalBytes;
	ULONG biTrueWidth;

	// Bogus parameter check
	if (!(ppbPalette != NULL && ppbBits != NULL))
		{ fprintf(stderr, "invalid BMP file\n"); rc = -1000; goto GetOut; }

	// File exists?
	if ((pfile = fopen(szFile, "rb")) == NULL)
		{ fprintf(stderr, "unable to open BMP file\n"); rc = -1; goto GetOut; }
	
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
		{ fprintf(stderr, "invalid BMP file header\n");  rc = -3000; goto GetOut; }

	// Bogus bit depth?  Only 8-bit supported.
	if (bmih.biBitCount != 8)
		{ fprintf(stderr, "BMP file not 8 bit\n");  rc = -4; goto GetOut; }
	
	// Bogus compression?  Only non-compressed supported.
	if (bmih.biCompression != BI_RGB)
		{ fprintf(stderr, "invalid BMP compression type\n"); rc = -5; goto GetOut; }

	// Figure out how many entires are actually in the table
	if (bmih.biClrUsed == 0)
		{
		bmih.biClrUsed = 256;
		cbPalBytes = (1 << bmih.biBitCount) * sizeof( RGBQUAD );
		}
	else 
		{
		cbPalBytes = bmih.biClrUsed * sizeof( RGBQUAD );
		}

	// Read palette (bmih.biClrUsed entries)
	if (fread(rgrgbPalette, cbPalBytes, 1/*count*/, pfile) != 1)
		{ rc = -6; goto GetOut; }

	// convert to a packed 768 byte palette
	pbPal = malloc(768);
	if (pbPal == NULL)
		{ rc = -7; goto GetOut; }

	pb = pbPal;

	// Copy over used entries
	for (i = 0; i < (int)bmih.biClrUsed; i++)
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
	cbBmpBits = bmfh.bfSize - ftell(pfile);
	pb = malloc(cbBmpBits);
	if (fread(pb, cbBmpBits, 1/*count*/, pfile) != 1)
		{ rc = -7; goto GetOut; }

	pbBmpBits = malloc(cbBmpBits);

	// data is actually stored with the width being rounded up to a multiple of 4
	biTrueWidth = (bmih.biWidth + 3) & ~3;
	
	// reverse the order of the data.
	pb += (bmih.biHeight - 1) * biTrueWidth;
	for(i = 0; i < bmih.biHeight; i++)
	{
		memmove(&pbBmpBits[biTrueWidth * i], pb, biTrueWidth);
		pb -= biTrueWidth;
	}

	pb += biTrueWidth;
	free(pb);

	bmhd.w = (WORD)bmih.biWidth;
	bmhd.h = (WORD)bmih.biHeight;
	// Set output parameters
	*ppbPalette = pbPal;
	*ppbBits = pbBmpBits;

GetOut:
	if (pfile) 
		fclose(pfile);

	return rc;
}


int WriteBMPfile (char *szFile, byte *pbBits, int width, int height, byte *pbPalette)
{
	int i, rc = 0;
	FILE *pfile = NULL;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	RGBQUAD rgrgbPalette[256];
	ULONG cbBmpBits;
	BYTE* pbBmpBits;
	byte  *pb, *pbPal = NULL;
	ULONG cbPalBytes;
	ULONG biTrueWidth;

	// Bogus parameter check
	if (!(pbPalette != NULL && pbBits != NULL))
		{ rc = -1000; goto GetOut; }

	// File exists?
	if ((pfile = fopen(szFile, "wb")) == NULL)
		{ rc = -1; goto GetOut; }

	biTrueWidth = ((width + 3) & ~3);
	cbBmpBits = biTrueWidth * height;
	cbPalBytes = 256 * sizeof( RGBQUAD );

	// Bogus file header check
	bmfh.bfType = MAKEWORD( 'B', 'M' );
	bmfh.bfSize = sizeof bmfh + sizeof bmih + cbBmpBits + cbPalBytes;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof bmfh + sizeof bmih + cbPalBytes;

	// Write file header
	if (fwrite(&bmfh, sizeof bmfh, 1/*count*/, pfile) != 1)
		{ rc = -2; goto GetOut; }

	// Size of structure
	bmih.biSize = sizeof bmih;
	// Width
	bmih.biWidth = biTrueWidth;
	// Height
	bmih.biHeight = height;
	// Only 1 plane 
	bmih.biPlanes = 1;
	// Only 8-bit supported.
	bmih.biBitCount = 8;
	// Only non-compressed supported.
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;

	// huh?
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;

	// Always full palette
	bmih.biClrUsed = 256;
	bmih.biClrImportant = 0;
	
	// Write info header
	if (fwrite(&bmih, sizeof bmih, 1/*count*/, pfile) != 1)
		{ rc = -3; goto GetOut; }
	

	// convert to expanded palette
	pb = pbPalette;

	// Copy over used entries
	for (i = 0; i < (int)bmih.biClrUsed; i++)
	{
		rgrgbPalette[i].rgbRed   = *pb++;
		rgrgbPalette[i].rgbGreen = *pb++;
		rgrgbPalette[i].rgbBlue  = *pb++;
        rgrgbPalette[i].rgbReserved = 0;
	}

	// Write palette (bmih.biClrUsed entries)
	cbPalBytes = bmih.biClrUsed * sizeof( RGBQUAD );
	if (fwrite(rgrgbPalette, cbPalBytes, 1/*count*/, pfile) != 1)
		{ rc = -6; goto GetOut; }


	pbBmpBits = malloc(cbBmpBits);

	pb = pbBits;
	// reverse the order of the data.
	pb += (height - 1) * width;
	for(i = 0; i < bmih.biHeight; i++)
	{
		memmove(&pbBmpBits[biTrueWidth * i], pb, width);
		pb -= width;
	}

	// Write bitmap bits (remainder of file)
	if (fwrite(pbBmpBits, cbBmpBits, 1/*count*/, pfile) != 1)
		{ rc = -7; goto GetOut; }

	free(pbBmpBits);

GetOut:
	if (pfile) 
		fclose(pfile);

	return rc;
}

/*
=================
=
= LoadLBM
=
=================
*/

void LoadLBM (char *filename, byte **picture, byte **palette)
{
	byte    *LBMbuffer, *picbuffer, *cmapbuffer;
	int             y,p,planes;
	byte    *LBM_P, *LBMEND_P;
	byte    *pic_p;
	byte    *body_p;
	unsigned        rowsize;

	int    formtype,formlength;
	int    chunktype,chunklength;
	void    (*mungecall) (int, byte *);

// qiet compiler warnings
	picbuffer = NULL;
	cmapbuffer = NULL;
	mungecall = NULL;

//
// load the LBM
//
	LoadFile (filename, (void **)&LBMbuffer);

//
// parse the LBM header
//
	LBM_P = LBMbuffer;
	if ( *(int *)LBMbuffer != LittleLong(FORMID) )
	   Error ("No FORM ID at start of file!\n");

	LBM_P += 4;
	formlength = BigLong( *(int *)LBM_P );
	LBM_P += 4;
	LBMEND_P = LBM_P + Align(formlength);

	formtype = LittleLong(*(int *)LBM_P);

	if (formtype != ILBMID && formtype != PBMID)
		Error ("Unrecognized form type: %c%c%c%c\n", formtype&0xff
		,(formtype>>8)&0xff,(formtype>>16)&0xff,(formtype>>24)&0xff);

	LBM_P += 4;

//
// parse chunks
//

	while (LBM_P < LBMEND_P)
	{
		chunktype = LBM_P[0] + (LBM_P[1]<<8) + (LBM_P[2]<<16) + (LBM_P[3]<<24);
		LBM_P += 4;
		chunklength = LBM_P[3] + (LBM_P[2]<<8) + (LBM_P[1]<<16) + (LBM_P[0]<<24);
		LBM_P += 4;

		switch ( chunktype )
		{
		case BMHDID:
			memcpy (&bmhd,LBM_P,sizeof(bmhd));
			bmhd.w = BigShort(bmhd.w);
			bmhd.h = BigShort(bmhd.h);
			bmhd.x = BigShort(bmhd.x);
			bmhd.y = BigShort(bmhd.y);
			bmhd.pageWidth = BigShort(bmhd.pageWidth);
			bmhd.pageHeight = BigShort(bmhd.pageHeight);
			break;

		case CMAPID:
			cmapbuffer = malloc (768);
			memset (cmapbuffer, 0, 768);
			memcpy (cmapbuffer, LBM_P, chunklength);
			break;

		case BODYID:
			body_p = LBM_P;

			pic_p = picbuffer = malloc (bmhd.w*bmhd.h);
			if (formtype == PBMID)
			{
			//
			// unpack PBM
			//
				for (y=0 ; y<bmhd.h ; y++, pic_p += bmhd.w)
				{
					if (bmhd.compression == cm_rle1)
						body_p = LBMRLEDecompress ((byte *)body_p
						, pic_p , bmhd.w);
					else if (bmhd.compression == cm_none)
					{
						memcpy (pic_p,body_p,bmhd.w);
						body_p += Align(bmhd.w);
					}
				}

			}
			else
			{
			//
			// unpack ILBM
			//
				planes = bmhd.nPlanes;
				if (bmhd.masking == ms_mask)
					planes++;
				rowsize = (bmhd.w+15)/16 * 2;
				switch (bmhd.nPlanes)
				{
				case 1:
					mungecall = MungeBitPlanes1;
					break;
				case 2:
					mungecall = MungeBitPlanes2;
					break;
				case 4:
					mungecall = MungeBitPlanes4;
					break;
				case 8:
					mungecall = MungeBitPlanes8;
					break;
				default:
					Error ("Can't munge %i bit planes!\n",bmhd.nPlanes);
				}

				for (y=0 ; y<bmhd.h ; y++, pic_p += bmhd.w)
				{
					for (p=0 ; p<planes ; p++)
						if (bmhd.compression == cm_rle1)
							body_p = LBMRLEDecompress ((byte *)body_p
							, bitplanes[p] , rowsize);
						else if (bmhd.compression == cm_none)
						{
							memcpy (bitplanes[p],body_p,rowsize);
							body_p += rowsize;
						}

					mungecall (bmhd.w , pic_p);
				}
			}
			break;
		}

		LBM_P += Align(chunklength);
	}

	free (LBMbuffer);

	*picture = picbuffer;
	*palette = cmapbuffer;
}


/*
============================================================================

							WRITE LBM

============================================================================
*/

/*
==============
=
= WriteLBMfile
=
==============
*/

void WriteLBMfile (char *filename, byte *data, int width, int height, byte *palette)
{
	byte    *lbm, *lbmptr;
	int    *formlength, *bmhdlength, *cmaplength, *bodylength;
	int    length;
	bmhd_t  basebmhd;

	lbm = lbmptr = malloc (width*height+1000);

//
// start FORM
//
	*lbmptr++ = 'F';
	*lbmptr++ = 'O';
	*lbmptr++ = 'R';
	*lbmptr++ = 'M';

	formlength = (int*)lbmptr;
	lbmptr+=4;                      // leave space for length

	*lbmptr++ = 'P';
	*lbmptr++ = 'B';
	*lbmptr++ = 'M';
	*lbmptr++ = ' ';

//
// write BMHD
//
	*lbmptr++ = 'B';
	*lbmptr++ = 'M';
	*lbmptr++ = 'H';
	*lbmptr++ = 'D';

	bmhdlength = (int *)lbmptr;
	lbmptr+=4;                      // leave space for length

	memset (&basebmhd,0,sizeof(basebmhd));
	basebmhd.w = BigShort((short)width);
	basebmhd.h = BigShort((short)height);
	basebmhd.nPlanes = (BYTE)BigShort(8);
	basebmhd.xAspect = (BYTE)BigShort(5);
	basebmhd.yAspect = (BYTE)BigShort(6);
	basebmhd.pageWidth = BigShort((short)width);
	basebmhd.pageHeight = BigShort((short)height);

	memcpy (lbmptr,&basebmhd,sizeof(basebmhd));
	lbmptr += sizeof(basebmhd);

	length = lbmptr-(byte *)bmhdlength-4;
	*bmhdlength = BigLong(length);
	if (length&1)
		*lbmptr++ = 0;          // pad chunk to even offset

//
// write CMAP
//
	*lbmptr++ = 'C';
	*lbmptr++ = 'M';
	*lbmptr++ = 'A';
	*lbmptr++ = 'P';

	cmaplength = (int *)lbmptr;
	lbmptr+=4;                      // leave space for length

	memcpy (lbmptr,palette,768);
	lbmptr += 768;

	length = lbmptr-(byte *)cmaplength-4;
	*cmaplength = BigLong(length);
	if (length&1)
		*lbmptr++ = 0;          // pad chunk to even offset

//
// write BODY
//
	*lbmptr++ = 'B';
	*lbmptr++ = 'O';
	*lbmptr++ = 'D';
	*lbmptr++ = 'Y';

	bodylength = (int *)lbmptr;
	lbmptr+=4;                      // leave space for length

	memcpy (lbmptr,data,width*height);
	lbmptr += width*height;

	length = lbmptr-(byte *)bodylength-4;
	*bodylength = BigLong(length);
	if (length&1)
		*lbmptr++ = 0;          // pad chunk to even offset

//
// done
//
	length = lbmptr-(byte *)formlength-4;
	*formlength = BigLong(length);
	if (length&1)
		*lbmptr++ = 0;          // pad chunk to even offset

//
// write output file
//
	SaveFile (filename, lbm, lbmptr-lbm);
	free (lbm);
}

