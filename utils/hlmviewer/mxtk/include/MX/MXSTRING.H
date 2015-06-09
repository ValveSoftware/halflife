//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxstring.h
// implementation: all
// last modified:  Apr 28 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXSTRING
#define INCLUDED_MXSTRING



#ifdef __cplusplus
extern "C" {
#endif

int mx_strncasecmp (const char *s1, const char *s2, int count);
int mx_strcasecmp (const char *s1, const char *s2);

char *mx_strlower (char *str);

#ifdef __cplusplus
}
#endif



#endif // INCLUDED_MXPATH
