//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: dll-agnostic routines (no dll dependencies here)
//
// $NoKeywords: $
//=============================================================================

// Author: Matthew D. Campbell (matt@turtlerockstudios.com), 2003

#ifndef SHARED_UTIL_H
#define SHARED_UTIL_H
#ifdef LINUX
#include <string.h>
#include <wchar.h>
#endif
#include <string.h>

//--------------------------------------------------------------------------------------------------------
/**
 * Returns the token parsed by SharedParse()
 */
char *SharedGetToken( void );

//--------------------------------------------------------------------------------------------------------
/**
 * Sets the character used to delimit quoted strings.  Default is '\"'.  Be sure to set it back when done.
 */
void SharedSetQuoteChar( char c );

//--------------------------------------------------------------------------------------------------------
/**
 * Parse a token out of a string
 */
const char *SharedParse( const char *data );

//--------------------------------------------------------------------------------------------------------
/**
 * Returns true if additional data is waiting to be processed on this line
 */
bool SharedTokenWaiting( const char *buffer );

//--------------------------------------------------------------------------------------------------------
/**
 * Simple utility function to allocate memory and duplicate a string
 */
inline char *CloneString( const char *str )
{
	if ( !str )
	{
		char *cloneStr = new char[1];
		cloneStr[0] = '\0';
		return cloneStr;
	}
	char *cloneStr = new char [ strlen(str)+1 ];
	strcpy( cloneStr, str );
	return cloneStr;
}

//--------------------------------------------------------------------------------------------------------
/**
 * Simple utility function to allocate memory and duplicate a wide string
 */
inline wchar_t *CloneWString( const wchar_t *str )
{
	if ( !str )
	{
		wchar_t *cloneStr = new wchar_t[1];
		cloneStr[0] = L'\0';
		return cloneStr;
	}
	wchar_t *cloneStr = new wchar_t [ wcslen(str)+1 ];
	wcscpy( cloneStr, str );
	return cloneStr;
}

//--------------------------------------------------------------------------------------------------------------
/**
 *  snprintf-alike that allows multiple prints into a buffer
 */
char * BufPrintf(char *buf, int& len, const char *fmt, ...);

//--------------------------------------------------------------------------------------------------------------
/**
 *  wide char version of BufPrintf
 */
wchar_t * BufWPrintf(wchar_t *buf, int& len, const wchar_t *fmt, ...);

//--------------------------------------------------------------------------------------------------------------
/**
 *  convenience function that prints an int into a static wchar_t*
 */
const wchar_t * NumAsWString( int val );

//--------------------------------------------------------------------------------------------------------------
/**
 *  convenience function that prints an int into a static char*
 */
const char * NumAsString( int val );

//--------------------------------------------------------------------------------------------------------------
/**
 *  convenience function that composes a string into a static char*
 */
char * SharedVarArgs(char *format, ...);

//--------------------------------------------------------------------------------------------------------------
/**
 *  convenience function that composes a string into a static wchar_t* (Win32-only)
 */
wchar_t * SharedWVarArgs(wchar_t *format, ...);

//--------------------------------------------------------------------------------------------------------------
/**
 *  Wrapper for vgui::localize()->Find() that doesn't return NULL (client-only)
 */
#ifdef CLIENT_DLL
wchar_t* SharedFindString( char *asciiIdentifier );
#endif

#endif // SHARED_UTIL_H
