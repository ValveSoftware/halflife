//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined( HUD_IFACEH )
#define HUD_IFACEH
#pragma once

#ifdef _WIN32
#ifdef EXPORT
#undef EXPORT
#endif // EXPORT
#define EXPORT _declspec( dllexport )
#else
#define EXPORT __attribute__ ((visibility("default")))
#endif
#define _DLLEXPORT EXPORT

typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);
#include "wrect.h"
#include "../engine/cdll_int.h"
extern cl_enginefunc_t gEngfuncs;

#endif