//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: dll-agnostic routines (no dll dependencies here)
//
// $NoKeywords: $
//=============================================================================

// Author: Matthew D. Campbell (matt@turtlerockstudios.com), 2003

#include <port.h>
#include <stdarg.h>
#include "shared_util.h"

#ifndef _WIN32
#define _vsnwprintf vswprintf
#endif

static char s_shared_token[ 1500 ];
static char s_shared_quote = '\"';

//--------------------------------------------------------------------------------------------------------------
/**
 *  Wrapper for vgui::localize()->Find() that doesn't return NULL
 */
#ifdef CLIENT_DLL
#include <vgui/IVGui.h>
#include <vgui_controls/Controls.h>	// for localize()
#include <vgui/ILocalize.h>
wchar_t* SharedFindString( char *asciiIdentifier )
{
	const int BufLen = 1024;
	const int NumBuffers = 4;
	static wchar_t string[NumBuffers][BufLen];
	static int curstring = 0;
	
	wchar_t *identifier = vgui::localize()->Find( asciiIdentifier );
	if ( !identifier )
	{
		identifier = string[curstring];
		curstring = ( curstring + 1 ) % NumBuffers;

		vgui::localize()->ConvertANSIToUnicode( asciiIdentifier, identifier, BufLen*2 );
	}

	return identifier;
}
#endif

//--------------------------------------------------------------------------------------------------------------
wchar_t * SharedWVarArgs(wchar_t *format, ...)
{
	va_list argptr;
	const int BufLen = 1024;
	const int NumBuffers = 4;
	static wchar_t string[NumBuffers][BufLen];
	static int curstring = 0;
	
	curstring = ( curstring + 1 ) % NumBuffers;

	va_start (argptr, format);
	_vsnwprintf( string[curstring], BufLen, format, argptr );
	va_end (argptr);

	return string[curstring];  
}

//--------------------------------------------------------------------------------------------------------------
char * SharedVarArgs(char *format, ...)
{
	va_list argptr;
	const int BufLen = 1024;
	const int NumBuffers = 4;
	static char string[NumBuffers][BufLen];
	static int curstring = 0;
	
	curstring = ( curstring + 1 ) % NumBuffers;

	va_start (argptr, format);
#ifdef _WIN32
	_vsnprintf( string[curstring], BufLen, format, argptr );
#else
	vsnprintf( string[curstring], BufLen, format, argptr );
#endif
	va_end (argptr);

	return string[curstring];  
}

//--------------------------------------------------------------------------------------------------------------
char * BufPrintf(char *buf, int& len, const char *fmt, ...)
{
	if (len <= 0)
		return NULL;

	va_list argptr;

	va_start(argptr, fmt);
	vsnprintf(buf, len, fmt, argptr);
	va_end(argptr);

	len -= strlen(buf);
	return buf + strlen(buf);
}

//--------------------------------------------------------------------------------------------------------------
wchar_t * BufWPrintf(wchar_t *buf, int& len, const wchar_t *fmt, ...)
{
	if (len <= 0)
		return NULL;

	va_list argptr;

	va_start(argptr, fmt);
	_vsnwprintf(buf, len, fmt, argptr);
	va_end(argptr);

	len -= wcslen(buf);
	return buf + wcslen(buf);
}

//--------------------------------------------------------------------------------------------------------------
const wchar_t * NumAsWString( int val )
{
	const int BufLen = 16;
	const int NumBuffers = 4;
	static wchar_t string[NumBuffers][BufLen];
	static int curstring = 0;
	
	curstring = ( curstring + 1 ) % NumBuffers;

	int len = BufLen;
	BufWPrintf( string[curstring], len, L"%d", val );
	return string[curstring];
}

//--------------------------------------------------------------------------------------------------------------
const char * NumAsString( int val )
{
	const int BufLen = 16;
	const int NumBuffers = 4;
	static char string[NumBuffers][BufLen];
	static int curstring = 0;
	
	curstring = ( curstring + 1 ) % NumBuffers;

	int len = BufLen;
	BufPrintf( string[curstring], len, "%d", val );
	return string[curstring];
}

//--------------------------------------------------------------------------------------------------------
/**
 * Returns the token parsed by SharedParse()
 */
char *SharedGetToken( void )
{
	return s_shared_token;
}

//--------------------------------------------------------------------------------------------------------
/**
 * Returns the token parsed by SharedParse()
 */
void SharedSetQuoteChar( char c )
{
	s_shared_quote = c;
}

//--------------------------------------------------------------------------------------------------------
/**
 * Parse a token out of a string
 */
const char *SharedParse( const char *data )
{
	int             c;
	int             len;
	
	len = 0;
	s_shared_token[0] = 0;
	
	if (!data)
		return NULL;
		
// skip whitespace
skipwhite:
	while ( (c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;                    // end of file;
		data++;
	}
	
// skip // comments
	if (c=='/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}
	

// handle quoted strings specially
	if (c == s_shared_quote)
	{
		data++;
		while (len < sizeof( s_shared_token ) - 1)
		{
			c = *data++;
			if (c==s_shared_quote || !c)
			{
				s_shared_token[len] = 0;
				return data;
			}
			s_shared_token[len] = c;
			len++;
		}
	}

// parse single characters
	if (len < sizeof( s_shared_token ) - 1)
	{
		if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c == ',' )
		{
			s_shared_token[len] = c;
			len++;
			s_shared_token[len] = 0;
			return data+1;
		}
	}

// parse a regular word
	while (len < sizeof( s_shared_token ) - 1)
	{
		s_shared_token[len] = c;
		data++;
		len++;
		c = *data;

		if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c == ',' )
			break;

		if (c <= 32)
			break;
	}
	
	s_shared_token[len] = 0;
	return data;
}

//--------------------------------------------------------------------------------------------------------
/**
 * Returns true if additional data is waiting to be processed on this line
 */
bool SharedTokenWaiting( const char *buffer )
{
	const char *p;

	p = buffer;
	while ( *p && *p!='\n')
	{
		if ( !isspace( *p ) || isalnum( *p ) )
			return true;

		p++;
	}

	return false;
}
