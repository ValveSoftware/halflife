//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxChooseColor.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXCHOOSECOLOR
#define INCLUDED_MXCHOOSECOLOR



class mxWindow;


#ifdef __cplusplus
extern "C" {
#endif

bool mxChooseColor (mxWindow *parent, int *r, int *g, int *b);

#ifdef __cplusplus
}
#endif



#endif // INCLUDED_MXCHOOSECOLOR
