//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined( SYS_DEDH )
#define SYS_DEDH
#ifdef _WIN32
#pragma once
#endif

typedef void (*SleepType)(int);
long Sys_LoadLibrary( char *lib );
void Sys_FreeLibrary( long library );
void *Sys_GetProcAddress( long library, const char *name );
void Sys_Printf(char *fmt, ...);
void Sys_ErrorMessage( int level, const char *msg );

#endif // SYS_DEDH