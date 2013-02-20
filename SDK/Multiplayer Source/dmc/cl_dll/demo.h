//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined( DEMOH )
#define DEMOH
#pragma once

// Types of demo messages we can write/parse
enum
{
	TYPE_USER = 0,
};

void Demo_WriteBuffer( int type, int size, unsigned char *buffer );

#endif