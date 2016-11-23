//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "hud.h"
#include "cl_util.h"
#include "demo.h"
#include "demo_api.h"
#include <memory.h>

#define DLLEXPORT __declspec( dllexport )

extern "C" 
{
	void DLLEXPORT Demo_ReadBuffer( int size, unsigned char *buffer );
}

/*
=====================
Demo_WriteBuffer

Write some data to the demo stream
=====================
*/
void Demo_WriteBuffer( int type, int size, unsigned char *buffer )
{
	int pos = 0;
	unsigned char buf[ 32 * 1024 ];
	*( int * )&buf[pos] = type;
	pos+=sizeof( int );

	memcpy( &buf[pos], buffer, size );

	// Write full buffer out
	gEngfuncs.pDemoAPI->WriteBuffer( size + sizeof( int ), buf );
}

/*
=====================
Demo_ReadBuffer

Engine wants us to parse some data from the demo stream
=====================
*/
void DLLEXPORT Demo_ReadBuffer( int size, unsigned char *buffer )
{
	int type;
	int i = 0;

	type = *( int * )buffer;
	i += sizeof( int );
	switch ( type )
	{
	case TYPE_USER:
		break;
	default:
		gEngfuncs.Con_DPrintf( "Unknown demo buffer type, skipping.\n" );
		break;
	}
}