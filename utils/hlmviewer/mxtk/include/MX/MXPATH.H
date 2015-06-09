//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxpath.h
// implementation: all
// last modified:  Apr 28 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXPATH
#define INCLUDED_MXPATH



#ifdef __cplusplus
extern "C" {
#endif

bool mx_setcwd (const char *path);
const char *mx_getcwd ();

const char *mx_getpath (const char *filename);
const char *mx_getextension (const char *filename);

const char *mx_gettemppath ();

#ifdef __cplusplus
}
#endif



#endif // INCLUDED_MXPATH
