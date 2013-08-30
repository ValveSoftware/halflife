//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "extdll.h"
#include "entity_state.h"
#include "pm_defs.h"
#include "pm_shared.h"
#include "pm_movevars.h"
#include "hud_iface.h"
#include "com_model.h"
#include "event_api.h"
#include "com_weapons.h"
#include "event_flags.h"
#include "DMC_BSPFile.h"
#include "cl_util.h"

#include "FileSystem.h"

extern IFileSystem *g_pFileSystem;

extern "C" playermove_t *pmove;
extern int g_runfuncs;

// Don't support more than MAX_TELE teleporters ( map still can load tho )
#define MAX_TELES 256

extern Vector g_vecTeleMins[ MAX_TELES ];
extern Vector g_vecTeleMaxs[ MAX_TELES ];
extern int g_iTeleNum;
extern int g_iUser1;
extern bool g_bLoadedTeles;
vec3_t  vecTempAngles;
bool	bChangeAngles;


// We only care about two kinds of entities for now:  Teleporters and their targets
// FIXME:  After loading, store a pointer from teleporter to target instead of looking up all the time.
typedef enum
{
	// Entity is a teleporter
	DMC_TELE = 0,
	// Entity is a target
	DMC_TARGET
} dmc_teletype_t;

typedef struct
{
	// Type of entity
	dmc_teletype_t	type;

	// Classname
	char			classname[ 32 ];

	// What this entity targets
	char			target[ 32 ];

	// If entity is a target, the name tag it uses
	char			targetname[ 32 ];

	// Orientation of the teleporter
	float			angles[3];

	// Target origin
	float			origin[3];

	// Bounding box of the teleporter
	float			absmin[3];
	float			absmax[3];

} dmc_tele_t;

// Teleporter/Target entity database
static dmc_tele_t s_teles[ MAX_TELES ];
static int s_num_teles = 0;

// We'll use this for playing the teleporting sounds locally.
static unsigned short s_usTeleport;

/*
==============================
Dmc_SetKeyValue

Fill in key/values fro the teleporter
==============================
*/
void Dmc_SetKeyValue( dmc_tele_t *pTele, const char *key, const char *value )
{
	float x, y, z;

	if ( !stricmp( key, "classname" ) )
	{
		strcpy( pTele->classname, value );
	}
	else if ( !stricmp( key, "target" ) )
	{
		strcpy( pTele->target, value );
	}
	else if ( !stricmp( key, "targetname" ) )
	{
		strcpy( pTele->targetname, value );
	}
	else if ( !stricmp( key, "angles" ) )
	{
		if ( sscanf( value, "%f %f %f", &x, &y, &z ) == 3 )
		{
			pTele->angles[ 0 ] = x ;
			pTele->angles[ 1 ] = y;
			pTele->angles[ 2 ] = z;
		}
	}
	else if ( !stricmp( key, "origin" ) )
	{
		if ( sscanf( value, "%f %f %f", &x, &y, &z ) == 3 )
		{
			pTele->origin[ 0 ]  = x;
			pTele->origin[ 1 ]  = y;
			pTele->origin[ 2 ]  = z;
		}
	}
}

/*
==============================
Dmc_ParseTeleporter

Evaluate Key/Value pairs for the entity
==============================
*/
char *Dmc_ParseTeleporter( char *buffer, dmc_tele_t *pTele, int *error )
{
	char		key[256];
	char		token[ 1024 ];
	int			n;

	memset( pTele, 0, sizeof( *pTele ) );

	while (1)
	{	
		// Parse key
		buffer = gEngfuncs.COM_ParseFile ( buffer, token );
		if ( token[0] == '}' )
			break;
		
		// Ran out of input buffer?
		if ( !buffer )
		{
			*error = 1;
			break;
		}
		
		// Store off the key
		strcpy ( key, token );

		// Fix heynames with trailing spaces
		n = strlen( key );
		while (n && key[n-1] == ' ')
		{
			key[n-1] = 0;
			n--;
		}

		// Parse value	
		buffer = gEngfuncs.COM_ParseFile ( buffer, token );

		// Ran out of buffer?
		if (!buffer)
		{
			*error = 1;
			break;
		}

		// Hit the end instead of a value?
		if ( token[0] == '}' )
		{
			*error = 1;
			break;
		}

		if ( token[0] == '}' && token[1] == '(' ) 
			int k = 0;

		// Assign k/v pair
		Dmc_SetKeyValue( pTele, key, token );
	}

	// Return what's left in the stream
	return buffer;
}

/*
==============================
Dmc_ProcessEnts

Parse through entity lump looking for teleporters or targets
==============================
*/
void Dmc_ProcessEnts( char *buffer )
{	
	char token[ 1024 ];
	dmc_tele_t	*pTele = NULL;
	int			error = 0;
	
	// parse entities from entity lump of .bsp file
	while ( 1 )
	{
		// parse the opening brace	
		buffer = gEngfuncs.COM_ParseFile ( buffer, token );
		if (!buffer)
			break;
		
		// Didn't find opening brace?
		if ( token[0] != '{' )
		{
			gEngfuncs.Con_Printf ("Dmc_ProcessEnts: found %s when expecting {\n", token );
			return;
		}

		// Assume we're filling in this tele
		pTele = &s_teles[ s_num_teles ];

		// Fill in data
		buffer = Dmc_ParseTeleporter( buffer, pTele, &error );

		// Check for errors and abort if any
		if ( error )
		{
			gEngfuncs.Con_Printf ("Dmc_ProcessEnts: error parsing entities\n" );
			return;
		}

		// Check classname
		if ( stricmp( pTele->classname, "trigger_teleport" ) && stricmp( pTele->classname, "info_teleport_destination" ) )
			continue;

		// Set type based on classname
		if ( !stricmp( pTele->classname, "trigger_teleport" ) )
		{
			pTele->type = DMC_TELE;
		}
		else
		{
			pTele->type = DMC_TARGET;
		}
	
		// If we got to here, we're using the entity
		s_num_teles++;

		// No more room...
		if ( s_num_teles >= MAX_TELES )
			break;
	}
}

/*
==============================
Dmc_LoadEntityLump

Open the .bsp and read in the entity lump
==============================
*/
char *Dmc_LoadEntityLump( const char *filename )
{
	FileHandle_t fp;
	int			i;
	dheader_t	header;
	int			size;
	lump_t		*curLump;
	char		*buffer = NULL;

	fp = g_pFileSystem->Open( filename, "rb" );
	if ( !fp )
		return NULL;

	// Read in the .bsp header
	if ( g_pFileSystem->Read(&header, sizeof(dheader_t), fp) != sizeof(dheader_t) )
	{
		gEngfuncs.Con_Printf("Dmc_LoadEntityLump:  Could not read BSP header for map [%s].\n", filename);
		g_pFileSystem->Close(fp);
		return NULL;
	}

	// Check the version
	i = header.version;
	if ( i != 29 && i != 30)
	{
		g_pFileSystem->Close(fp);
		gEngfuncs.Con_Printf("Dmc_LoadEntityLump:  Map [%s] has incorrect BSP version (%i should be %i).\n", filename, i, BSPVERSION);
		return NULL;
	}

	// Get entity lump
	curLump = &header.lumps[ LUMP_ENTITIES ];
	// and entity lump size
	size = curLump->filelen;

	// Jump to it
	g_pFileSystem->Seek( fp, curLump->fileofs, FILESYSTEM_SEEK_HEAD );

	// Allocate sufficient memmory
	buffer = (char *)malloc( size + 1 );
	if ( !buffer )
	{
		g_pFileSystem->Close(fp);
		gEngfuncs.Con_Printf("Dmc_LoadEntityLump:  Couldn't allocate %i bytes\n", size + 1 );
		return NULL;
	}

	// Read in the entity lump
	g_pFileSystem->Read( buffer, size, fp );

	// Terminate the string
	buffer[ size ] = '\0';

	if ( fp )
	{
		g_pFileSystem->Close(fp);
	}

	return buffer;
}

/*
==============================
Dmc_LoadTeleporters

Load in the .bsp file and process the entities
==============================
*/
void Dmc_LoadTeleporters( const char *map )
{
	char	*buffer = NULL;
	char	filename[ 256 ];

	sprintf( filename, "%s", map );

	// TODO:  Fix Slashes?

	// Reset count
	s_num_teles = 0;

	// Load entity lump
	buffer = Dmc_LoadEntityLump( filename );
	if ( !buffer )
		return;

	// Process buffer and extract teleporters/targets
	Dmc_ProcessEnts( buffer );

	// Discard buffer
	free( buffer );
}

/*
==============================
Dmc_FindTarget

Search entity list for target matching "name"
==============================
*/
dmc_tele_t *Dmc_FindTarget( const char *name, int numtele, dmc_tele_t *pTeles )
{
	int i;
	dmc_tele_t *target;

	// Find the target
	for ( i = 0; i < numtele; i++ )
	{
		target = &pTeles[ i ];
		
		if ( !target )
			continue;

		if ( stricmp( target->targetname, name ) )
			continue;

		return target;
	}

	return NULL;
}

/*
==============================
Dmc_TeleporterTouched

Imparts the desired velocity to the player 
after touching a teleporter.
==============================
*/
void Dmc_TeleporterTouched( int numtele, dmc_tele_t *pTeles, dmc_tele_t *pTele, struct local_state_s *player )
{
	int				i;
	dmc_tele_t		*target;
	pmtrace_t 		tr;
	float			flGravity = pmove->movevars->gravity;

	vec3_t			forward, up, right;

	float			zero[ 3 ] = { 0.0, 0.0, 0.0 };

	// Find the target
	target = Dmc_FindTarget( pTele->target, numtele, pTeles );

	for ( i = 0; i < 3; i++ )
		player->playerstate.origin[ i ] = target->origin[ i ];
	
	player->playerstate.origin[ 2 ] += 27;

	AngleVectors( target->angles, forward, right, up );
	player->client.velocity = forward * 300;
	
	// Play sound if appropriate
	if ( s_usTeleport && g_runfuncs )
	{
		//Adrian - This is a little hack to make the player face 
		//the destination angles as soon as we step out.
		//Check view.cpp for the rest.
		for ( i = 0; i < 3; i++ )
			  vecTempAngles[ i ] = target->angles[ i ];

		bChangeAngles = true;
		
		gEngfuncs.pfnPlaybackEvent( FEV_NOTHOST, NULL, s_usTeleport, 0.0, target->origin, zero, 0.0, 0.0, 0, 0, 0, 0 );
	}
}

/*
==============================
Dmc_TouchTeleporters

See if player is touching a teleporter ( not that kind of touching! ).
==============================
*/
void Dmc_TouchTeleporters (  struct local_state_s *player, dmc_tele_t *pTeles, int numtele )
{
	int			i, j;
	dmc_tele_t	*pTele;
	float		absmin[3], absmax[3];
	float		pmins[ 3 ] = { 13, 13, 24 };
	float		pmaxs[ 3 ] = { 13, 13, 32 };
	vec3_t		LengthVector;
	int			iTeleNum = 0;
	

	// Determine player's bbox
	for ( j = 0; j < 3; j++ )
	{
		absmin[ j ] = player->playerstate.origin[ j ] - pmins[ j ];
		absmax[ j ] = player->playerstate.origin[ j ] + pmaxs[ j ];
	}

	for ( i = 0; i < numtele; i++ )
	{
		pTele = &pTeles[ i ];
		if ( !pTele )
			continue;

		if ( pTele->type != DMC_TELE )
			continue;

		//Adrian - Load all the teleporter Mins and Max size.
		//This comes via an event when the player connects.
		if ( !g_bLoadedTeles )
		{
			for ( int j = 0; j < 3; j++ )
			{
				pTele->absmin[ j ] = g_vecTeleMins[ iTeleNum ][ j ] - 1.0;
				pTele->absmax[ j ] = g_vecTeleMaxs[ iTeleNum ][ j ] + 1.0;
			}
			iTeleNum++;
			
			//Done going thru all the teleporters
			if ( iTeleNum == g_iTeleNum )
				 g_bLoadedTeles = true;	
		}

		if  (  absmin[0] > pTele->absmax[0]
			|| absmin[1] > pTele->absmax[1]
			|| absmin[2] > pTele->absmax[2]
			|| absmax[0] < pTele->absmin[0]
			|| absmax[1] < pTele->absmin[1]
			|| absmax[2] < pTele->absmin[2] )
			continue;

		Dmc_TeleporterTouched( numtele, pTeles, pTele, player );

		break;
	}
}

/*
==============================
Dmc_CheckTeleporters

Load data if needed, otherwise just run checks on player's final position to see if teleporter needs
to impart velocity on the player.
==============================
*/
void Dmc_CheckTeleporters( struct local_state_s *from, struct local_state_s *to )
{
	static char current_level[ 128 ];
	
	// See if we've changed to a new map
	if ( stricmp( current_level, gEngfuncs.pfnGetLevelName() ) )
	{
		strcpy( current_level, gEngfuncs.pfnGetLevelName() );
		Dmc_LoadTeleporters( current_level );

		// Grab sound event
		s_usTeleport = gEngfuncs.pfnPrecacheEvent( 1, "events/teleport.sc" );
	}

	// Run test, only if we're not a spectator
	if ( g_iUser1 == OBS_NONE )
		Dmc_TouchTeleporters( to, s_teles, s_num_teles );
}
