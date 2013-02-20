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
#if !defined ( DEMO_APIH )
#define DEMO_APIH
#ifdef _WIN32
#pragma once
#endif

typedef struct demo_api_s
{
	int		( *IsRecording )	( void );
	int		( *IsPlayingback )	( void );
	int		( *IsTimeDemo )		( void );
	void	( *WriteBuffer )	( int size, unsigned char *buffer );
} demo_api_t;

extern demo_api_t demoapi;

#endif
