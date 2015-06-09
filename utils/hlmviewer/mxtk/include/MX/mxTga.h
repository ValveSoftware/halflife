//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxTga.h
// implementation: all
// last modified:  Apr 15 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXTGA
#define INCLUDED_MXTGA



#ifndef INCLUDED_MXIMAGE
#include <mx/mxImage.h>
#endif



mxImage *mxTgaRead (const char *filename);
bool mxTgaWrite (const char *filename, mxImage *image);



#endif // INCLUDED_MXTGA
