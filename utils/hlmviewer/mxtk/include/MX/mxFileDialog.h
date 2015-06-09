//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxFileDialog.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXFILEDIALOG
#define INCLUDED_MXFILEDIALOG



class mxWindow;



#ifdef __cplusplus
extern "C" {
#endif

const char *mxGetOpenFileName (mxWindow *parent, const char *path, const char *filter);
const char *mxGetSaveFileName (mxWindow *parent, const char *path, const char *filter);

#ifdef __cplusplus
}
#endif



#endif // INCLUDED_MXFILEDIALOG
