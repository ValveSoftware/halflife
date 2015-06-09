//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxstring.cpp
// implementation: all
// last modified:  May 04 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxstring.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



int
mx_strncasecmp (const char *s1, const char *s2, int count)
{
#ifdef WIN32
	return _strnicmp (s1, s2, count);
#else
	return strncasecmp (s1, s2, count);
#endif
}



int
mx_strcasecmp (const char *s1, const char *s2)
{
#ifdef WIN32
	return _stricmp (s1, s2);
#else
	return strcasecmp (s1, s2);
#endif
}




char *
mx_strlower (char *str)
{
	int i;
	for (i = strlen (str) - 1; i >= 0; i--)
		str[i] = tolower (str[i]);

	return str;
}
