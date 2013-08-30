#include "extdll.h"
#include "entity_state.h"
#include "pm_defs.h"
#include "pm_movevars.h"
#include "hud_iface.h"
#include "com_model.h"
#include "event_api.h"
#include "com_weapons.h"
#include "event_flags.h"
#include "Ricochet_BSPFile.h"

extern "C" playermove_t *pmove;
extern int g_runfuncs;

// Don't support more than MAX_PADS pads ( map still can load, but we'll just have some pads that don't predict. )
#define MAX_PADS 256

// We only care about two kinds of entities for now:  Jump pads and their targets
// FIXME:  After loading, store a pointer from pad to target instead of looking up all the time.
typedef enum
{
	// Entity is a jump pad
	RIC_PAD = 0,
	// Entity is a target
	RIC_TARGET
} ric_padtype_t;

typedef struct
{
	// Type of entity
	ric_padtype_t	type;

	// Classname
	char			classname[ 32 ];

	// Model name
	char			modelname[ 32 ];

	// What this entity targets
	char			target[ 32 ];

	// If entity is a target, the name tag it uses
	char			targetname[ 32 ];

	// Orientation of the pad
	float			angles[3];

	// Target origin
	float			origin[3];

	// Bounding box of the pad
	float			absmin[3];
	float			absmax[3];

	// Model associated with the pad
	struct	model_s *model;

	float			height;
} ric_pad_t;

// Pad/Target entity database
static ric_pad_t s_pads[ MAX_PADS ];
static int s_num_pads = 0;

// We'll use this for playing the jump sounds locally.
static unsigned short s_usJump;

/*
==============================
Ricochet_SetKeyValue

Fill in key/values fro the pad
==============================
*/
void Ricochet_SetKeyValue( ric_pad_t *pad, const char *key, const char *value )
{
	float x, y, z;

	if ( !stricmp( key, "classname" ) )
	{
		strcpy( pad->classname, value );
	}
	else if ( !stricmp( key, "target" ) )
	{
		strcpy( pad->target, value );
	}
	else if ( !stricmp( key, "targetname" ) )
	{
		strcpy( pad->targetname, value );
	}
	else if ( !stricmp( key, "model" ) )
	{
		strcpy( pad->modelname, value );
	}
	else if ( !stricmp( key, "height" ) )
	{
		pad->height = atof( value );
	}
	else if ( !stricmp( key, "angles" ) )
	{
		if ( sscanf( value, "%f %f %f", &x, &y, &z ) == 3 )
		{
			pad->angles[ 0 ] = x ;
			pad->angles[ 1 ] = y;
			pad->angles[ 2 ] = z;
		}
	}
	else if ( !stricmp( key, "origin" ) )
	{
		if ( sscanf( value, "%f %f %f", &x, &y, &z ) == 3 )
		{
			pad->origin[ 0 ]  = x;
			pad->origin[ 1 ]  = y;
			pad->origin[ 2 ]  = z;
		}
	}
}

/*
==============================
Ricochet_ParsePad

Evaluate Key/Value pairs for the entity
==============================
*/
char *Ricochet_ParsePad( char *buffer, ric_pad_t *pad, int *error )
{
	char		key[256];
	char		token[ 1024 ];
	int			n;

	memset( pad, 0, sizeof( *pad ) );

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

		// Assign k/v pair
		Ricochet_SetKeyValue( pad, key, token );
	}

	// Return what's left in the stream
	return buffer;
}

/*
==============================
Ricochet_ProcessEnts

Parse through entity lump looking for pads or targets
==============================
*/
void Ricochet_ProcessEnts( char *buffer )
{	
	int i;
	char token[ 1024 ];
	ric_pad_t	*pad = NULL;
	int			error = 0;
	
	// parse entities from entity lump of .bsp file
	while (1)
	{
		// parse the opening brace	
		buffer = gEngfuncs.COM_ParseFile ( buffer, token );
		if (!buffer)
			break;
		
		// Didn't find opening brace?
		if ( token[0] != '{' )
		{
			gEngfuncs.Con_Printf ("Ricochet_ProcessEnts: found %s when expecting {\n", token );
			return;
		}

		// Assume we're filling in this pad
		pad = &s_pads[ s_num_pads ];

		// Fill in data
		buffer = Ricochet_ParsePad( buffer, pad, &error );

		// Check for errors and abort if any
		if ( error )
		{
			gEngfuncs.Con_Printf ("Ricochet_ProcessEnts: error parsing entities\n" );
			return;
		}

		// Check classname
		if ( stricmp( pad->classname, "trigger_jump" ) && stricmp( pad->classname, "info_target" ) )
			continue;

		// Set type based on classname
		if ( !stricmp( pad->classname, "trigger_jump" ) )
		{
			pad->type = RIC_PAD;
		}
		else
		{
			pad->type = RIC_TARGET;
		}

		// Load up the model
		pad->model = gEngfuncs.CL_LoadModel( pad->modelname, NULL );
		if ( pad->model )
		{
			// Fill in abs bbox
			for ( i = 0; i < 3; i++ )
			{
				pad->absmin[ i ] = pad->model->mins[ i ] - 1.0;
				pad->absmax[ i ] = pad->model->maxs[ i ] + 1.0;
			}
		}

		// If we got to here, we're using the entity
		s_num_pads++;

		// No more room...
		if ( s_num_pads >= MAX_PADS )
			break;
	}
}

/*
==============================
Ricochet_LoadEntityLump

Open the .bsp and read in the entity lump
==============================
*/
char *Ricochet_LoadEntityLump( const char *filename )
{
	int			i;
	dheader_t	header;
	int			size = 0;
	lump_t		*curLump;
	char		*fileBuffer = NULL, *buffer, *entlump;

	fileBuffer = (char *)gEngfuncs.COM_LoadFile((char *)filename, 5, &size);
	if (size < sizeof(dheader_t))
		return NULL;

	// Read in the .bsp header
	memcpy(&header, fileBuffer, sizeof(dheader_t));

	// Check the version
	i = header.version;
	if ( i != 29 && i != 30)
	{
		gEngfuncs.COM_FreeFile(fileBuffer);
		gEngfuncs.Con_Printf("Ricochet_LoadEntityLump:  Map [%s] has incorrect BSP version (%i should be %i).\n", filename, i, BSPVERSION);
		return NULL;
	}

	// Get entity lump
	curLump = &header.lumps[ LUMP_ENTITIES ];
	// and entity lump size
	size = curLump->filelen;

	// Jump to it
	entlump = fileBuffer + curLump->fileofs;

	// Allocate sufficient memmory
	buffer = (char *)malloc( size + 1 );
	if ( !buffer )
	{
		gEngfuncs.COM_FreeFile(fileBuffer);
		gEngfuncs.Con_Printf("Ricochet_LoadEntityLump:  Couldn't allocate %i bytes\n", size + 1 );
		return NULL;
	}

	// Read in the entity lump
	memcpy( buffer, entlump, size );

	// Terminate the string
	buffer[ size ] = '\0';

	if (fileBuffer)
	{
		gEngfuncs.COM_FreeFile(fileBuffer);
	}

	return buffer;
}

/*
==============================
Ricochet_LoadJumpPads

Load in the .bsp file and process the entities
==============================
*/
void Ricochet_LoadJumpPads( const char *map )
{
	char	*buffer = NULL;
	char	filename[ 256 ];

	sprintf( filename, "%s/%s", gEngfuncs.pfnGetGameDirectory(), map );

	// TODO:  Fix Slashes?

	// Reset count
	s_num_pads = 0;

	// Load entity lump
	buffer = Ricochet_LoadEntityLump( filename );
	if ( !buffer )
		return;

	// Process buffer and extract pads/targets
	Ricochet_ProcessEnts( buffer );

	// Discard buffer
	free( buffer );
}

/*
==============================
Ricochet_FindTarget

Search entity list for target matching "name"
==============================
*/
ric_pad_t *Ricochet_FindTarget( const char *name, int numpads, ric_pad_t *pads )
{
	int i;
	ric_pad_t *target;

	// Find the target
	for ( i = 0; i < numpads; i++ )
	{
		target = &pads[ i ];
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
Ricochet_PadTouched

Register impact ( impart velocity on player and if final function call, play appropriate jump sound )
==============================
*/
void Ricochet_PadTouched( int numpads, ric_pad_t *pads, ric_pad_t *pad, struct local_state_s *player )
{
	int				i;
	ric_pad_t		*target;
	float			origin[ 3 ];
	pmtrace_t 		tr;
	float			flGravity = pmove->movevars->gravity;

	float			vecMidPoint[3];
	float			end[ 3 ];

	// Ricochet jump pads use default jump height
	float			flHeight = 150;

	float			zero[ 3 ] = { 0.0, 0.0, 0.0 };

	// Find the target
	target = Ricochet_FindTarget( pad->target, numpads, pads );

	// Target now points to target pad
	for ( i = 0; i < 3; i++ )
	{
		origin[ i ] = player->playerstate.origin[ i ];
		
		// Get a rough idea of how high to launch
		vecMidPoint[ i ] = origin[ i ] + ( target->origin[ i ] - origin[ i ]) * 0.5;
		end[ i ] = vecMidPoint[ i ];
	}

	if ( pad->height != 0.0 )
	{
		flHeight = pad->height;
	}

	// Move up by height
	end[ 2 ] += flHeight;

	// See if we can reach the apex from the midpoint
	gEngfuncs.pEventAPI->EV_PlayerTrace( vecMidPoint, end, PM_STUDIO_BOX, -1, &tr );

	// Use the end point of the trace as the midpoint of the actual toss
	for ( i = 0; i < 3; i++ )
	{
		vecMidPoint[i] = tr.endpos[i];
	}

	// Subtract some units so we don't hit the ceiling)
	vecMidPoint[2] -= 15;

	// How high should we travel to reach the apex
	float distance1 = fabs(vecMidPoint[2] - origin[2]);
	float distance2 = fabs(vecMidPoint[2] - target->origin[2]);

	// How long will it take to travel this distance
	float time1 = sqrt( distance1 / (0.5 * flGravity) );
	float time2 = sqrt( distance2 / (0.5 * flGravity) );
	if (time1 < 0.1)
		return;

	// Determine how hard to launch to get there in time.
	float vecTargetVel[3];
	
	for ( i = 0; i < 3; i++ )
	{
		vecTargetVel[ i ] = (target->origin[ i ] - origin[ i ]) / (time1 + time2);
	}

	// Adjust upward velocity needed
	vecTargetVel[ 2 ] = flGravity * time1;

	// Fill in needed velocity
	for ( i = 0; i < 3; i++ )
	{
		player->client.velocity[i] = vecTargetVel[i];
	}

	// Play sound if appropriate
	if ( s_usJump && g_runfuncs )
	{
		gEngfuncs.pfnPlaybackEvent( FEV_NOTHOST, NULL, s_usJump, 0.0, zero, zero, 0.0, 0.0, 0, 0, 0, 0 );
	}
}

/*
==============================
Ricochet_TouchPads

See if player's resting position impacts any jump pads
==============================
*/
void Ricochet_TouchPads (  struct local_state_s *player, ric_pad_t *pads, int numpads )
{
	int			i, j;
	ric_pad_t	*pad;
	float		absmin[3], absmax[3];
	physent_t	pe;
	hull_t		*hull;
	int			num;
	float		test[3];
	float		pmins[ 3 ] = { 16, 16, 36 };

	// Determine player's bbox
	for ( j = 0; j < 3; j++ )
	{
		absmin[ j ] = player->playerstate.origin[ j ] - pmins[ j ];
		absmax[ j ] = player->playerstate.origin[ j ] + pmins[ j ];
	}

	// Cycle through pads looking for a match
	for ( i = 0; i < numpads; i++ )
	{
		pad = &pads[ i ];
		if ( !pad )
			continue;

		// Target entities don't make us jump
		if ( pad->type != RIC_PAD )
			continue;

		// Trivial reject?
		if ( absmin[0] > pad->absmax[0]
			|| absmin[1] > pad->absmax[1]
			|| absmin[2] > pad->absmax[2]
			|| absmax[0] < pad->absmin[0]
			|| absmax[1] < pad->absmin[1]
			|| absmax[2] < pad->absmin[2] )
			continue;
			
		// Set up physent for the test case
		pe.model	= pad->model;
		pe.origin	= pad->origin;
		
		// Use standing player hull
		pmove->usehull = 0;

		// Make sure it's a brush model, of course
		if ( !pe.model || (modtype_t)pmove->PM_GetModelType( pe.model ) != mod_brush )
			continue;

		// Get the hull
		hull = (hull_t *)pmove->PM_HullForBsp( &pe, test );
		num = hull->firstclipnode;

		// Offset the origin by the offset appropriate for this hull.
		for ( j = 0; j < 3; j++ )
		{
			test[ j ] = player->playerstate.origin[ j ] - test[ j ];
		}

		// Test the player's hull for intersection with this model
		if ( pmove->PM_HullPointContents ( hull, num, test ) != CONTENTS_SOLID )
		{
			continue;
		}
		
		// TOUCHED!!!
		Ricochet_PadTouched( numpads, pads, pad, player );

		// Only touch one pad at a time
		break;
	}
}

/*
==============================
Ricochet_CheckJumpPads

Load data if needed, otherwise just run checks on player's final position to see if jump pad needs
 to impart velocity on the player.
==============================
*/
void Ricochet_CheckJumpPads( struct local_state_s *from, struct local_state_s *to )
{
	static char current_level[ 128 ];
	
	// See if we've changed to a new map
	if ( stricmp( current_level, gEngfuncs.pfnGetLevelName() ) )
	{
		strcpy( current_level, gEngfuncs.pfnGetLevelName() );
		Ricochet_LoadJumpPads( current_level );

		// Grab sound event
		s_usJump = gEngfuncs.pfnPrecacheEvent( 1, "events/jump.sc" );
	}

	// Not while spectating
	if ( to->client.iuser1 )
		return;

	// Run test
	Ricochet_TouchPads( to, s_pads, s_num_pads );
}