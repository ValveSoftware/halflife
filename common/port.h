// port.h: portability helper
//
//////////////////////////////////////////////////////////////////////

#if !defined PORT_H
#define PORT_H

#include "archtypes.h"     // DAL

#ifdef _WIN32

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define WIN32_EXTRA_LEAN

#include "winsani_in.h"
#include <windows.h>
#include "winsani_out.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define snprintf _snprintf
#define vsnprintf _vsnprintf

#else // _WIN32

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // exit()
#include <string.h> // strncpy()
#include <ctype.h> // tolower()
#include <limits.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/ioctl.h>

typedef unsigned char BYTE;
typedef short int WORD;
typedef unsigned int DWORD;
typedef int32 LONG;
//typedef uint32 ULONG;
#ifndef ARCHTYPES_H
typedef uint32 ULONG;
#endif
typedef void *HANDLE;
#ifndef HMODULE
typedef void *HMODULE;
#endif
typedef char * LPSTR;

#define __cdecl


//const int MAX_PATH = PATH_MAX;
#define MAX_PATH PATH_MAX

#ifdef LINUX
typedef struct POINT_s
{
	int x;
	int y;
} POINT;
typedef void *HINSTANCE;
typedef void *HWND;
typedef void *HDC;
typedef void *HGLRC;

typedef struct RECT_s
{
	int left;
	int right;
	int top;
	int bottom;
} RECT;
#endif


#ifdef __cplusplus

//#undef FALSE
//#undef TRUE

#ifdef OSX
//#else
//const bool	FALSE		= false;
//const bool	TRUE		= true;
#endif
#endif

#ifndef NULL
	#ifdef __cplusplus
		#define NULL    0
	#else
		#define NULL    ((void *)0)
	#endif
#endif

#ifdef __cplusplus
inline int		ioctlsocket( int d, int cmd, uint32 *argp )	{ return ioctl( d, cmd, argp ); }
inline int		closesocket( int fd )								{ return close( fd ); }
inline char *	GetCurrentDirectory( size_t size, char * buf )		{ return getcwd( buf, size ); }
inline int		WSAGetLastError()									{ return errno; }

inline void		DebugBreak( void ) { exit( 1 ); }
#endif

extern char g_szEXEName[ 4096 ];

#define _snprintf snprintf

#if defined(OSX)
#define SO_ARCH_SUFFIX ".dylib"
#else
#if defined ( __x86_64__ )
#define SO_ARCH_SUFFIX "_amd64.so"
#else
#define SO_ARCH_SUFFIX ".so"
#endif
#endif
#endif

#endif // PORT_H
