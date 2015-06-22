/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

//
//  vstdlib.h
//
// vstdlib contains definitions/macros bound to ANSI C functions
//
// Used mainly to avoid POSIX warning/deprecation or other notifications.
//
// NOTE:
//
//	+ This will only prevent from generating POSIX warnings.
//
//	- This will not disable CRT warnings, if using Microsoft C Runtime libraries.
//
//		+ To disable CRT warnings, add   /D "_CRT_SECURE_NO_WARNINGS"   
//		  to command line
//
//		+ To disable CRT deprecation warnings, add   /D "_CRT_SECURE_NO_DEPRECATE"
//		  to command line
//

#ifndef VSTDLIB_H
#define VSTDLIB_H

#ifdef _WIN32
#pragma once
#endif

#ifndef NULL
#if !defined ( __cplusplus )
#define NULL ((void *)0)
#else // defined ( __cplusplus )
#define NULL 0
#endif // !defined ( __cplusplus )
#endif // !defined ( NULL )

//=============================================================================
//
// Purpose: V_* definitions. 
//			
//			Primarily used for portability. Also used
//		    to prevent POSIX deprecation warnings.
//
// NOTE: If you plan to use a function from C library that is not part
//		 of the following list, please do the the following:
//		
//		| (1) Define a new macro using the following structure: 
//		|
//		| #define V_%	%	
//		|
//		|	  [ where % represents the name of the C function ]
//		|
//		| (2) Proceed to the end of the file and define a second macro, 
//		|	  using the following structure: 
//		|
//		| #define Q_%	V_%
//		|
//		|     [ Where % represents the name of the C function ]
//
//=============================================================================

/* Runtime assert functions <cassert> */

#define V_assert		assert

/* Character handling functions <cctype> */

#define V_isalnum		isalnum
#define V_isalpha		isalpha
#define V_iscntrl		iscntrl
#define V_isdigit		isdigit
#define V_isgraph		isgraph
#define V_islower		islower
#define V_isprint		isprint
#define V_ispunct		ispunct
#define V_isspace		isspace
#define V_isupper		isupper
#define V_isxdigit		isxdigit
#define V_tolower		tolower
#define V_toupper		toupper

/* errno macro <cerrno> */
#define V_errno			errno


/* Math functions <cmath> */

#define V_cos			cos
#define V_sin			sin
#define V_tan			tan
#define V_acos			acos
#define V_asin			asin
#define V_atan			atan
#define V_atan2			atan2
#define V_cosh			cosh
#define V_sinh			sinh
#define V_tanh			tanh
#define V_exp			exp
#define V_frexp			frexp
#define V_ldexp			ldexp
#define V_log			log
#define V_log10			log10
#define V_modf			modf
#define V_pow			pow
#define V_sqrt			sqrt
#define V_ceil			ceil
#define V_floor			floor
#define V_fmod			fmod
#define V_fabs(x)		((x) > 0 ? (x) : 0 - (x))
#define V_abs(x)		((x) > 0 ? (x) : 0 - (x))
#define V_labs(x)		((x) > 0L ? (x) : 0L - (x))

// C99 pow
#define V_powf			powf
#define V_powl			powl


/* Standard IO functions <cstdio> */

#define V_remove		remove
#define V_rename		rename

#define V_fflush		fflush
#define V_fopen			fopen
#define V_fclose		fclose
#define V_setbuf		setbuf
#define V_setvbuf		setvbuf
#define V_fread			fread
#define V_fwrite		fwrite
#define V_fgetpos		fgetpos
#define V_fseek			fseek
#define V_fsetpos		fsetpos
#define V_ftell			ftell
#define V_rewind		rewind
#define V_clearerr		clearerr
#define V_feof			feof
#define V_ferror		ferror
#define V_perror		perror

#define V_fprintf		fprintf
#define V_fscanf		fscanf
#define V_printf		printf
#define V_scanf			scanf
#define V_snprintf		_snprintf
#define V_sprintf		sprintf
#define V_sscanf		sscanf
#define V_vprintf		vprintf
#define V_vsnprintf		_vsnprintf
#define V_vsprintf		vsprintf

#define V_fgetc			fgetc
#define V_fgets			fgets
#define V_fputc			fputc
#define V_fputs			fputs
#define V_getc			getc
#define V_getchar		getchar
#define V_gets			gets
#define V_putc			putc
#define V_putchar		putchar
#define V_puts			puts
#define V_ungetc		ungetc

#define V_unlink		_unlink

/* stdlib functions <cstdlib> */

#define V_atof			atof
#define V_atoi			atoi
#define V_atol			atol

#define V_strtod		strtod
#define V_strtol		strtol
#define V_strtoul		strtoul

#define V_rand			rand
#define V_srand			srand

#define V_calloc		calloc
#define V_free			free
#define V_malloc		malloc
#define V_realloc		realloc

#define V_exit			exit
#define V_getenv		getenv

#define V_bsearch		bsearch
#define V_qsort			qsort

#if defined ( WIN32 ) || defined ( _WIN32 )
#define V_itoa			_itoa_s
#endif


/* String functions <cstring> */

#define V_memchr		memchr
#define V_memcmp		memcmp
#define V_memcpy		memcpy
#define V_memmove		memmove
#define V_memset		memset

#define V_strcat		strcat
#define V_strchr		strchr
#define V_strcmp		strcmp
#define V_strcoll		strcoll
#define V_strcpy		strcpy
#define V_strerror		strerror
#define V_strlen		strlen
#define V_strncat		strncat
#define V_strncmp		strncmp
#define V_strncpy		strncpy

#define V_strpbrk		strpbrk
#define V_strrchr		strrchr
#define V_strspn		strspn
#define V_strstr		strstr
#define V_strtok		strtok

#define V_stricmp		_stricmp
#define V_strnicmp		_strnicmp

#if defined ( WIN32 ) || defined ( _WIN32 )
#define V_strupr		_strupr_s
#endif

#if defined ( WIN32 ) || defined ( _WIN32 )
#define V_strdup		_strdup
#endif

#if defined ( WIN32 ) || defined ( _WIN32 )
#define V_strcmpi		_strcmpi
#endif


/* Time functions <ctime> */

#define V_time			time


/* MISCELLANEOUS FUNCTIONS */

// min/max functions

#define V_max(a, b)		(((a) > (b)) ? (a) : (b))
#define V_min(a, b)		(((a) < (b)) ? (a) : (b))


//=============================================================================
// Purpose: Q_* definitions. Use these instead of the V_* ones.
//=============================================================================

/* Runtime assert functions <cassert> */

#define Q_assert		V_assert

/* Character handling functions <cctype> */

#define Q_isalnum		V_isalnum
#define Q_isalpha		V_isalpha
#define Q_iscntrl		V_iscntrl
#define Q_isdigit		V_isdigit
#define Q_isgraph		V_isgraph
#define Q_islower		V_islower
#define Q_isprint		V_isprint
#define Q_ispunct		V_ispunct
#define Q_isspace		V_isspace
#define Q_isupper		V_isupper
#define Q_isxdigit		V_isxdigit
#define Q_tolower		V_tolower
#define Q_toupper		V_toupper

/* errno macro <cerrno> */
#define Q_errno			V_errno

/* Math functions <cmath> */

#define Q_cos			V_cos
#define Q_sin			V_sin
#define Q_tan			V_tan
#define Q_acos			V_acos
#define Q_asin			V_asin
#define Q_atan			V_atan
#define Q_atan2			V_atan2
#define Q_cosh			V_cosh
#define Q_sinh			V_sinh
#define Q_tanh			V_tanh
#define Q_exp			V_exp
#define Q_frexp			V_frexp
#define Q_ldexp			V_ldexp
#define Q_log			V_log
#define Q_log10			V_log10
#define Q_modf			V_modf
#define Q_pow			V_pow
#define Q_sqrt			V_sqrt
#define Q_ceil			V_ceil
#define Q_floor			V_floor
#define Q_fmod			V_fmod
#define Q_fabs			V_fabs
#define Q_abs			V_abs
#define Q_labs			V_labs

// C99 pow
#define Q_powf			V_powf
#define Q_powl			V_powl

/* Standard IO functions <cstdio> */

#define Q_remove		V_remove
#define Q_rename		V_rename

#define Q_fflush		V_fflush
#define Q_fopen			V_fopen
#define Q_fclose		V_fclose
#define Q_setbuf		V_setbuf
#define Q_setvbuf		V_setvbuf
#define Q_fread			V_fread
#define Q_fwrite		V_fwrite
#define Q_fgetpos		V_fgetpos
#define Q_fseek			V_fseek
#define Q_fsetpos		V_fsetpos
#define Q_ftell			V_ftell
#define Q_rewind		V_rewind

#define Q_clearerr		V_clearerr
#define Q_feof			V_feof
#define Q_ferror		V_ferror
#define Q_perror		V_perror

#define Q_fprintf		V_fprintf
#define Q_fscanf		V_fscanf
#define Q_printf		V_printf
#define Q_scanf			V_scanf
#define Q_snprintf		V_snprintf
#define Q_sprintf		V_sprintf
#define Q_sscanf		V_sscanf
#define Q_vprintf		V_vprintf
#define Q_vsnprintf		V_vsnprintf
#define Q_vsprintf		V_vsprintf

#define Q_fgetc			V_fgetc
#define Q_fgets			V_fgets
#define Q_fputc			V_fputc
#define Q_fputs			V_fputs
#define Q_getc			V_getc
#define Q_getchar		V_getchar
#define Q_gets			V_gets
#define Q_putc			V_putc
#define Q_putchar		V_putchar
#define Q_puts			V_puts
#define Q_ungetc		V_ungetc

#define Q_unlink		V_unlink

/* stdlib functions <cstdlib> */

#define Q_atof			V_atof
#define Q_atoi			V_atoi
#define Q_atol			V_atol

#define Q_strtod		V_strtod
#define Q_strtol		V_strtol
#define Q_strtoul		V_strtoul

#define Q_rand			V_rand
#define Q_srand			V_srand

#define Q_calloc		V_calloc
#define Q_free			V_free
#define Q_malloc		V_malloc
#define Q_realloc		V_realloc

#define Q_exit			V_exit
#define Q_getenv		V_getenv

#define Q_bsearch		V_bsearch
#define Q_qsort			V_qsort

#define Q_itoa			V_itoa

/* String functions <cstring> */

#define Q_memchr		V_memchr
#define Q_memcmp		V_memcmp
#define Q_memcpy		V_memcpy
#define Q_memmove		V_memmove
#define Q_memset		V_memset

#define Q_strcat		V_strcat
#define Q_strchr		V_strchr
#define Q_strcmp		V_strcmp
#define Q_strcoll		V_strcoll
#define Q_strcpy		V_strcpy
#define Q_strerror		V_strerror
#define Q_strlen		V_strlen

#define Q_strncat		V_strncat
#define Q_strncmp		V_strncmp
#define Q_strncpy		V_strncpy
#define Q_strpbrk		V_strpbrk
#define Q_strrchr		V_strrchr
#define Q_strspn		V_strspn
#define Q_strstr		V_strstr
#define Q_strtok		V_strtok

#define Q_stricmp		V_stricmp
#define Q_strnicmp		V_strnicmp

#if defined ( WIN32 ) || defined ( _WIN32 )
#define Q_strupr		V_strupr
#endif

#if defined ( WIN32 ) || defined ( _WIN32 )
#define Q_strdup		V_strdup
#endif

#if defined ( WIN32 ) || defined ( _WIN32 )
#define Q_strcmpi		V_strcmpi
#endif

/* Time functions <ctime> */

#define Q_time			V_time

/* MISCELLANEOUS FUNCTIONS */

// min/max functions

#define Q_max			V_max
#define Q_min			V_min

#endif // VSTDLIB_H
