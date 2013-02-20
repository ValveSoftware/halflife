//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// enginecallback.h
#ifndef INC_ENGINECALLBACKH
#define INC_ENGINECALLBACKH

typedef enum
{
	// A dedicated server with no ability to start a client
	ca_dedicated, 		
	// Full screen console with no connection
	ca_disconnected, 
	// Challenge requested, waiting for response or to resend connection request.
	ca_connecting,     
	// valid netcon, talking to a server, waiting for server data
	ca_connected,		
	// valid netcon, autodownloading
	ca_uninitialized,   
	// d/l complete, ready game views should be displayed
	ca_active			
} cactive_t;

#include "engine_launcher_api.h"

extern engine_api_t engineapi;

#endif // !INC_ENGINECALLBACKH
