//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxBmp.h
// implementation: all
// last modified:  Apr 15 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXBMP
#define INCLUDED_MXBMP



#ifndef INCLUDED_MXIMAGE
#include <mx/mxImage.h>
#endif


#ifdef WIN32
#include <pshpack2.h>
#endif

typedef struct
{
	word bfType; 
    int bfSize;
	word bfReserved1; 
	word bfReserved2;
	int bfOffBits;
} mxBitmapFileHeader;

typedef struct tagMxBitmapFileHeader
{
	int biSize; 
	int biWidth; 
	int biHeight; 
	word biPlanes; 
	word biBitCount;
	int biCompression; 
	int biSizeImage; 
	int biXPelsPerMeter; 
	int biYPelsPerMeter; 
	int biClrUsed; 
	int biClrImportant; 
} mxBitmapInfoHeader; 
 
typedef struct
{
	byte rgbBlue;
	byte rgbGreen; 
    byte rgbRed;
	byte rgbReserved;
} mxBitmapRGBQuad; 

#ifdef WIN32
#include <poppack.h>
#endif



#ifdef __cplusplus
extern "C" {
#endif

mxImage *mxBmpRead (const char *filename);
bool mxBmpWrite (const char *filename, mxImage *image);

#ifdef __cplusplus
}
#endif



#endif // INCLUDED_MXBMP
