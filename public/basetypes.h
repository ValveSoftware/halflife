//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef BASETYPES_H
#define BASETYPES_H
#ifdef _WIN32
#pragma once
#endif

#include "archtypes.h"

// stdio.h
#ifndef NULL
#define NULL 0
#endif

// Just to try to help with cross-platformability
#ifndef stackalloc
#define stackalloc _alloca
#endif

// Remove warnings from warning level 4.
#pragma warning(disable : 4514) // warning C4514: 'acosl' : unreferenced inline function has been removed
#pragma warning(disable : 4100) // warning C4100: 'hwnd' : unreferenced formal parameter
#pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#pragma warning(disable : 4512) // warning C4512: 'InFileRIFF' : assignment operator could not be generated
#pragma warning(disable : 4611) // warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable
#pragma warning(disable : 4706) // warning C4706: assignment within conditional expression
#pragma warning(disable : 4710) // warning C4710: function 'x' not inlined
#pragma warning(disable : 4702) // warning C4702: unreachable code
#pragma warning(disable : 4505) // unreferenced local function has been removed
#pragma warning(disable : 4239) // nonstandard extension used : 'argument' ( conversion from class Vector to class Vector& )
#pragma warning(disable : 4097) // typedef-name 'BaseClass' used as synonym for class-name 'CFlexCycler::CBaseFlex'
#pragma warning(disable : 4324) // Padding was added at the end of a structure


// In case this ever changes
#define M_PI			3.14159265358979323846

// C functions for external declarations that call the appropriate C++ methods
#ifndef EXPORT
#ifdef _WIN32
#define EXPORT	_declspec( dllexport )
#else
#define EXPORT	__attribute__ ((visibility("default")))
#endif
#endif

#include "minmax.h"

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
typedef int BOOL;
#endif
#ifndef WIN32
typedef uint32 ULONG;
#endif
typedef unsigned char BYTE;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned short ucs2;

typedef unsigned int string_t;	// from engine's pr_comp.h;
// vec3_t in the engine are float array's, and in the dlls the Vector class.

typedef float vec_t;

// FIXME: Remove
typedef vec_t vec2_t[2];


// FIXME: this should move 
#ifndef __cplusplus
#define true TRUE
#define false FALSE
#endif

//-----------------------------------------------------------------------------
// look for NANs, infinities, and underflows. 
// This assumes the ANSI/IEEE 754-1985 standard
//-----------------------------------------------------------------------------

#ifdef __cplusplus
inline uint32 const& FloatBits( vec_t const& f )
{
	return *reinterpret_cast<uint32 const*>(&f);
}

inline vec_t BitsToFloat( uint32 i )
{
	return *reinterpret_cast<vec_t*>(&i);
}

inline bool IsFinite( vec_t f )
{
	return ((FloatBits(f) & 0x7F800000) != 0x7F800000);
}


#define FLOAT32_NAN_BITS     (uint32)0x7FC00000	// not a number!
#define FLOAT32_NAN          BitsToFloat( FLOAT32_NAN_BITS )

#define VEC_T_NAN FLOAT32_NAN

#endif

// portability / compiler settings

#if defined(_WIN32) && !defined(WINDED)

#if defined(_M_IX86)
#define __i386__	1
#endif

//#pragma warning(disable : 4100)		// unreferenced formal parameter
//#pragma warning(disable : 4115)     // named type definition in parentheses
//#pragma warning(disable : 4127)     // conditional expression is constant
//#pragma warning(disable : 4201)     // nameless struct/union.
//#pragma warning(disable : 4214)     // bit field types other than int
#pragma warning(disable : 4244)     // type conversion warning.
#pragma warning(disable : 4305)		// truncation from 'const double ' to 'float '
//#pragma warning(disable : 4514)     // unreferenced inline function has been removed.
//#pragma warning(disable : 4706)		// assignment within conditional expression

#endif // defined(_WIN32) && !defined(WINDED)

// JoshA: Unifying with Linux build here
#if !defined( id386 )
	#if defined __i386__ && defined(_WIN32) && _MSC_VER <= 1200
		#define id386	1
	#else
		#define id386	0
	#endif  // __i386__
#endif // id386

#ifndef UNUSED
#define UNUSED(x)	(x = x)	// for pesky compiler / lint warnings
#endif

typedef struct vrect_s
{
	int				x,y,width,height;
	struct vrect_s			*pnext;
} vrect_t;

//#endif

#endif // BASETYPES_H
