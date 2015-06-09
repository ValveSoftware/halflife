//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxPcx.h
// implementation: all
// last modified:  Apr 15 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXPCX
#define INCLUDED_MXPCX



#ifndef INCLUDED_MXIMAGE
#include <mx/mxImage.h>
#endif



typedef struct
{
    byte manufacturer;   /* 10 = ZSoft .pcx */
    byte version;        /* 0 = Version 2.5 of PC Paintbrush 
                            2 = Version 2.8 w/palette information 
                            3 = Version 2.8 w/o palette information 
                            4 = PC Paintbrush for Windows(Plus for
                                Windows uses Ver 5) 
                            5 = Version 3.0 and > of PC Paintbrush
                                and PC Paintbrush +, includes
                                Publisher's Paintbrush . Includes
                                24-bit .PCX files */
    byte encoding;       /* 1 = .pcx rle encoding */
    byte bitsPerPixel;   /* 1, 2, 4, 8 per plane */
    word xmin;
    word ymin;
    word xmax;
    word ymax;
    word hDpi;
    word vDpi;
    byte colorMap[48];
    byte reserved;       /* should be set to 0 */
    byte numPlanes;      /* number of color planes */
    word bytesPerLine;   /* MUST be EVEN number */
    word paletteInfo;    /* 1 = color, 2 = grayscale */
    word hScreenSize;  
    word vScreenSize;
    byte filler[54];     /* set all to 0 */
} mxPcxHeader;



mxImage *mxPcxRead (const char *filename);
bool mxPcxWrite (const char *filename, mxImage *image);



#endif // INCLUDED_MXPCX
