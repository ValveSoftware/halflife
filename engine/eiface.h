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
#ifndef EIFACE_H
#define EIFACE_H

#include "archtypes.h"     // DAL

#ifdef HLDEMO_BUILD
#define INTERFACE_VERSION       001
#else  // !HLDEMO_BUILD, i.e., regular version of HL
#define INTERFACE_VERSION		140
#endif // !HLDEMO_BUILD

#include <stdio.h>
#include "custom.h"
#include "cvardef.h"
#include "Sequence.h"
//
// Defines entity interface between engine and DLLs.
// This header file included by engine files and DLL files.
//
// Before including this header, DLLs must:
//		include progdefs.h
// This is conveniently done for them in extdll.h
//

/*
#ifdef _WIN32
#define DLLEXPORT __stdcall
#else
#define DLLEXPORT  __attribute__ ((visibility("default")))
#endif
*/

typedef enum
	{
	at_notice,
	at_console,		// same as at_notice, but forces a ConPrintf, not a message box
	at_aiconsole,	// same as at_console, but only shown if developer level is 2!
	at_warning,
	at_error,
	at_logged		// Server print to console ( only in multiplayer games ).
	} ALERT_TYPE;

// 4-22-98  JOHN: added for use in pfnClientPrintf
typedef enum
	{
	print_console,
	print_center,
	print_chat,
	} PRINT_TYPE;

// For integrity checking of content on clients
typedef enum
{
	force_exactfile,					// File on client must exactly match server's file
	force_model_samebounds,				// For model files only, the geometry must fit in the same bbox
	force_model_specifybounds,			// For model files only, the geometry must fit in the specified bbox
	force_model_specifybounds_if_avail,	// For Steam model files only, the geometry must fit in the specified bbox (if the file is available)
} FORCE_TYPE;

// Returned by TraceLine
typedef struct
	{
	int		fAllSolid;			// if true, plane is not valid
	int		fStartSolid;		// if true, the initial point was in a solid area
	int		fInOpen;
	int		fInWater;
	float	flFraction;			// time completed, 1.0 = didn't hit anything
	vec3_t	vecEndPos;			// final position
	float	flPlaneDist;
	vec3_t	vecPlaneNormal;		// surface normal at impact
	edict_t	*pHit;				// entity the surface is on
	int		iHitgroup;			// 0 == generic, non zero is specific body part
	} TraceResult;

// CD audio status
typedef struct 
{
	int	fPlaying;// is sound playing right now?
	int	fWasPlaying;// if not, CD is paused if WasPlaying is true.
	int	fInitialized;
	int	fEnabled;
	int	fPlayLooping;
	float	cdvolume;
	//BYTE 	remap[100];
	int	fCDRom;
	int	fPlayTrack;
} CDStatus;

#include "../common/crc.h"


// Engine hands this to DLLs for functionality callbacks
typedef struct enginefuncs_s
{
	/**
	*	Precaches a model.
	*	If pszModelName is null, is empty or contains an invalid value in the first character, triggers a host error.
	*	If the maximum number of model precacheable resources has been reached, triggers a host error.
	*	If this is called after ServerActivate, triggers a host error.
	*	@param s Name of the model to precache. Starts in the game directory. This string must life for at least as long as the map itself.
	*	@return Index of the model.
	*/
	int			(*pfnPrecacheModel)			(char* s);

	/**
	*	Precaches a sound.
	*	If pszSoundName is null, is empty or contains an invalid value in the first character, triggers a host error.
	*	If the maximum number of sound precacheable resources has been reached, triggers a host error.
	*	If this is called after ServerActivate, triggers a host error.
	*	@param s Name of the sound to precache. Starts in the sound/ directory. This string must life for at least as long as the map itself.
	*	@return Index of the sound.
	*/
	int			(*pfnPrecacheSound)			(char* s);

	/**
	*	Sets the model of the given entity. Also changes the entity bounds based on the model.
	*	@param e Entity to set the model on.
	*	@param m Name of the model to set. This string must life for at least as long as the map itself.
	*	@see pfnPrecacheModel
	*/
	void		(*pfnSetModel)				(edict_t *e, const char *m);

	/**
	*	Gets the index of the given model.
	*	If the given model was not precached, shuts the game down.
	*	@param m Name of the model whose index is to be returned.
	*	@return Index of the model.
	*/
	int			(*pfnModelIndex)			(const char *m);

	/**
	*	Gets the number of frames in the given model.
	*	If this is a sprite, returns the number of sprite frames.
	*	If this is a studio model, this is all of the submodels in each body part multiplied with each-other.
	*	It represents the number of variations that can be created by changing submodels (e.g. heads, weapons, etc).
	*	Otherwise, returns 1.
	*	@param modelIndex Index of the model whose frame count is to be returned.
	*	@return Frame count of the model.
	*/
	int			(*pfnModelFrames)			(int modelIndex);

	/**
	*	Sets the entity bounds. Also relinks the entity.
	*	entvars_t::mins, entvars_t::maxs and entvars_t::size are changed.
	*	If the bounds are backwards (maxs smaller than mins), a host error is triggered.
	*	@param e Entity whose bounds are to be changed.
	*	@param rgflMin Minimum relative bounds.
	*	@param rgflMax Maximum relative bounds.
	*/
	void		(*pfnSetSize)				(edict_t *e, const float *rgflMin, const float *rgflMax);

	/**
	*	Changes the level. This will append a changelevel command to the server command buffer.
	*	Calling pfnServerExecute will trigger the changelevel.
	*	Subsequent calls made during the same map will be ignored.
	*	@param s1 Name of the level to change to.
	*	@param s2 Name of the landmark to use. If null, no landmark is used.
	*/
	void		(*pfnChangeLevel)			(char* s1, char* s2);

	/**
	*	Does nothing useful. Will trigger a host error if the given entity is not a client.
	*/
	void		(*pfnGetSpawnParms)			(edict_t *ent);

	/**
	*	Does nothing useful. Will trigger a host error if the given entity is not a client.
	*	Will trigger a sys error if the given entity is invalid.
	*/
	void		(*pfnSaveSpawnParms)		(edict_t *ent);

	/**
	*	Converts a direction vector to a yaw angle.
	*	@param Direction vector.
	*	@return Yaw angle.
	*/
	float		(*pfnVecToYaw)				(const float *rgflVector);

	/**
	*	Converts a direction vector to angles.
	*	@param rgflVectorIn Direction vector.
	*	@param rgflVectorOut Angles.
	*/
	void		(*pfnVecToAngles)			(const float *rgflVectorIn, float *rgflVectorOut);

	/**
	*	Moves the given entity to the given destination.
	*	@param ent Entity to move.
	*	@param pflGoal Destination.
	*	@param dist Distance to cover in this movement operation, in units.
	*	@param iMoveType Move type. @see MoveType
	*/
	void		(*pfnMoveToOrigin)			(edict_t *ent, const float *pflGoal, float dist, int iMoveType);

	/**
	*	Changes the entity's yaw angle to approach its ideal yaw.
	*	Yaw is updated at entvars_t::yaw_speed speed to match entvars_t::ideal_yaw.
	*	@param ent Entity whose yaw is so be changed.
	*/
	void		(*pfnChangeYaw)				(edict_t* ent);

	/**
	*	Changes the entity's pitch angle to approach its ideal pitch.
	*	Yaw is updated at entvars_t::pitch_speed speed to match entvars_t::idealpitch.
	*	@param ent Entity whose pitch is to be changed.
	*/
	void		(*pfnChangePitch)			(edict_t* ent);

	/**
	*	Finds an entity by comparing strings.
	*	@param pEdictStartSearchAfter Edict to start searching after.
	*	@param pszField Entity field to compare. Only string fields in entvars_t are considered.
	*	@param pszValue Value to compare to.
	*	@return	If the given field exists, and the given value is not null, and an entity has a matching value, returns the edict of that entity.
	*			Otherwise, returns null.
	*/
	edict_t*	(*pfnFindEntityByString)	(edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue);
	
	/**
	*	@param pEnt Entity whose light value is to be retrieved.
	*	@return	If the given entity is null, returns -1.
	*			If the given entity is a client or the world, returns entvars_t::light_level.
	*			Otherwise, returns the color of the floor that the entity is standing on.
	*/
	int			(*pfnGetEntityIllum)		(edict_t* pEnt);

	/**
	*	Finds an entity in a sphere.
	*	@param pEdictStartSearchAfter Edict to start searching after.
	*	@param org Origin in the world to center the sphere around.
	*	@param rad Sphere radius.
	*	@return The first valid entity in the sphere's radius, or null if no entity can be found.
	*/
	edict_t*	(*pfnFindEntityInSphere)	(edict_t *pEdictStartSearchAfter, const float *org, float rad);

	/**
	*	Finds a client in the Potentially Visible Set.
	*	Returns the world if no client could be found in the entity's PVS.
	*	This function's behavior is unexpected: it will get the next client every 0.1 seconds, and check if the entity can see it.
	*	If so, the client is returned. Otherwise, the world is returned.
	*	Use FNullEnt to check if the result is a valid client.
	*	@param pEdict Entity whose origin and view offset should be used to determine which client is visible.
	*	@return Client, or null if no client could be found.
	*/
	edict_t*	(*pfnFindClientInPVS)		(edict_t *pEdict);

	/**
	*	Find entities in Potentially Visible Set.
	*	This builds a list of entities using entvars_t::chain.
	*	This list is temporary, so store its results elsewhere if it is needed later on.
	*
	*	Note: this operation is expensive as it checks every entity.
	*	Avoid using this unless it is absolutely necessary.
	*	@param pplayer Player whose origin and view offset should be used to determine which entities are visible.
	*	@return First entity in the chain of entities that are visible, or worldspawn if no entities are visible.
	*/
	edict_t* (*pfnEntitiesInPVS)			(edict_t *pplayer);

	/**
	*	Make direction vectors from angles.
	*	@param rgflVector Angles to convert to direction vectors.
	*	The results are stored in gpGlobals->v_forward, gpGlobals->v_right and gpGlobals->v_up.
	*/
	void		(*pfnMakeVectors)			(const float *rgflVector);

	/**
	*	Make direction vectors from angles.
	*	@param rgflVector Angles to convert to direction vectors.
	*	@param forward Stores the forward direction vector.
	*	@param right Stores the right direction vector.
	*	@param up Stores the up direction vector.
	*/
	void		(*pfnAngleVectors)			(const float *rgflVector, float *forward, float *right, float *up);
	
	/**
	*	Allocates an edict for use with an entity.
	*	If the engine is out of edicts, triggers a sys error.
	*	If the engine is not ready for entity instantiation yet (e.g. during restore), triggers a sys error.
	*	@return Newly allocated edict.
	*/
	edict_t*	(*pfnCreateEntity)			(void);

	/**
	*	Immediately removes the given entity.
	*	Increments the edict serial number.
	*	@param e Entity to remove.
	*/
	void		(*pfnRemoveEntity)			(edict_t* e);

	/**
	*	Creates an entity of the class iszClassName.
	*	Note: this will not fall back to invoking the custom entity handler if the given class does not exist.
	*	@param className Name of the class to instantiate.
	*	@return Edict of the entity that was instantiated, or null if no such entity exists.
	*/
	edict_t*	(*pfnCreateNamedEntity)		(int className);

	/**
	*	Makes an entity static. Static entities are copied to the client side and are removed on the server side.
	*	Only valid during map spawn.
	*	@param ent Entity to make static.
	*/
	void		(*pfnMakeStatic)			(edict_t *ent);

	/**
	*	Returns whether the given entity is on the floor.
	*	@param e Entity to check.
	*	@return 1 if the entity is on the floor, 0 otherwise.
	*/
	int			(*pfnEntIsOnFloor)			(edict_t *e);

	/**
	*	Drops the entity to the floor.
	*	The entity will be moved down to the floor, effectively being teleported.
	*
	*	Note: maximum drop distance is 256 units. If the floor is further than that away, the entity will not be moved.
	*	@param e Entity to drop.
	*	@return	-1 if the entity is stuck inside a solid object.
	*			0 if the floor is further than 256 units away.
	*			1 if the entity was dropped to the floor.
	*/
	int			(*pfnDropToFloor)			(edict_t* e);

	/**
	*	Makes the entity walk.
	*
	*	The entity must be capable of flying (FL_FLY) or swimming (FL_SWIM), or be on the ground (FL_ONGROUND).
	*	@param ent Entity to move.
	*	@param yaw Yaw distance. This is the entity's current movement direction in the XY plane.
	*	@param Distance to move, in units.
	*	@param iMove Movement type. @see WalkMove
	*	@return 1 if the move succeeded (no obstacles in the way), 0 otherwise.
	*/
	int			(*pfnWalkMove)				(edict_t *ent, float yaw, float dist, int iMode);

	/**
	*	Sets the origin of the given entity.
	*	@param e Entity whose origin is to be set.
	*	@param rgflOrigin Origin to set.
	*/
	void		(*pfnSetOrigin)				(edict_t *e, const float *rgflOrigin);

	/**
	*	Emits a sounds from the given entity.
	*	@param entity Entity that is emitting the sound.
	*	@param channel Channel to play the sound on.
	*	@param pszSample Sample to play. The sound must be precached.
	*	@param volume Sound volume. Must be a value in the range [ 0, 1 ].
	*	@param attenuation Sound attenuation.
	*	@param fFlags Sound flags.
	*	@param pitch Sound pitch. Must be a value in the range [ 0, 255 ].
	*/
	void		(*pfnEmitSound)				(edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch);
	
	/**
	*	Emits a sounds from the given entity.
	*	@param entity Entity that is emitting the sound.
	*	@param pos Position in the world to play the sound at.
	*	@param pszSample Sample to play. The sound must be precached.
	*	@param volume Sound volume. Must be a value in the range [ 0, 1 ].
	*	@param attenuation Sound attenuation.
	*	@param fFlags Sound flags.
	*	@param pitch Sound pitch. Must be a value in the range [ 0, 255 ].
	*/
	void		(*pfnEmitAmbientSound)		(edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch);
	
	/**
	*	Performs a trace between a starting and ending position.
	*	@param v1 Start position.
	*	@param v2 End position.
	*	@param fNoMonsters Bit vector containing trace flags. @see TraceLineFlag
	*	@param pentToSkip Entity to ignore during the trace.
	*	@param ptr TraceResult instance.
	*/
	void		(*pfnTraceLine)				(const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr);
	
	/**
	*	Traces a toss.
	*	This simulates tossing the entity using its current origin, velocity, angular velocity, angles and gravity.
	*	Note that this does not use the same code as MOVETYPE_TOSS, and may return different results.
	*	@param pent Entity to toss.
	*	@param pentToIgnore Entity to ignore during the trace.
	*	@param ptr TraceResult instance.
	*/
	void		(*pfnTraceToss)				(edict_t* pent, edict_t* pentToIgnore, TraceResult *ptr);
	
	/**
	*	Performs a trace between a starting and ending position, using the given entity's mins and maxs.
	*	This can be any entity, not just monsters.
	*	@param pEdict Entity whose hull will be used.
	*	@param v1 Start position.
	*	@param v2 End position.
	*	@param fNoMonsters Bit vector containing trace flags. @see TraceLineFlag
	*	@param pentToSkip Entity to ignore during the trace.
	*	@param ptr TraceResult instance.
	*	@return true if the trace was entirely in a solid object, or if it hit something.
	*/
	int			(*pfnTraceMonsterHull)		(edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr);
	
	/**
	*	Performs a trace between a starting and ending position, using the specified hull.
	*	@param v1 Start position.
	*	@param v2 End position.
	*	@param fNoMonsters Bit vector containing trace flags. @see TraceLineFlag
	*	@param hullNumber Hull to use.
	*	@param pentToSkip Entity to ignore during the trace.
	*	@param ptr TraceResult instance.
	*/
	void		(*pfnTraceHull)				(const float *v1, const float *v2, int fNoMonsters, int hullNumber, edict_t *pentToSkip, TraceResult *ptr);
	
	/**
	*	Performs a trace between a starting and ending position.
	*	Similar to TraceHull, but will instead perform a trace in the given world hull using the given entity's model's hulls.
	*	For studio models this will use the model's hitboxes.
	*
	*	If the given entity's model is a studio model, uses its hitboxes.
	*	If it's a brush model, the brush model's hull for the given hull number is used (this may differ if custom brush hull sizes are in use).
	*	Otherwise, the entity bounds are converted into a hull.
	*
	*	@param v1 Start position.
	*	@param v2 End position.
	*	@param hullNumber Hull to use.
	*	@param pEntity Entity whose hull will be used.
	*	@param ptr TraceResult instance.
	*/
	void		(*pfnTraceModel)			(const float *v1, const float *v2, int hullNumber, edict_t *pent, TraceResult *ptr);
	
	/**
	*	Used to get texture info.
	*	The given entity must have a brush model set.
	*	If the traceline intersects the model, the texture of the surface it intersected is returned.
	*	Otherwise, returns null.
	*
	*	Is defined to return a const char*. However, the engine actually returns texture_t*, which works because texture_t::name is the first element, so it's identical.
	*	texture_t's layout differs between software and hardware mode, so avoid accessing its members.
	*
	*	@param pTextureEntity Entity whose texture is to be retrieved.
	*	@param v1 Start position.
	*	@param v2 End position.
	*	@return Texture instance, or null if no texture could be found.
	*/
	const char *(*pfnTraceTexture)			(edict_t *pTextureEntity, const float *v1, const float *v2 );
	
	/**
	*	Not implemented. Triggers a sys error.
	*/
	void		(*pfnTraceSphere)			(const float *v1, const float *v2, int fNoMonsters, float radius, edict_t *pentToSkip, TraceResult *ptr);
	
	/**
	*	Get the aim vector for the given entity
	*	Assumes MakeVectors was called with pEntity.vars.angles beforehand.
	*
	*	The aim vector is the autoaim vector used when sv_aim is enabled.
	*	It will snap to entities that are close to the entity's forward vector axis.
	*	@param pEntity Entity to retrieve the aim vector for.
	*	@param speed Unused.
	*	@param rgflReturn The resulting aim vector.
	*/
	void		(*pfnGetAimVector)			(edict_t* ent, float speed, float *rgflReturn);
	
	/**
	*	Issues a command to the server.
	*	The command must end with either a newline ('\n') or a semicolon (';') in order to be considered valid by the engine.
	*	The command will be enqueued for execution at a later point.
	*	@param str Command to execute.
	*/
	void		(*pfnServerCommand)			(char* str);
	
	/**
	*	Executes all pending server commands.
	*	Note: if a changelevel command is in the buffer, this can result in the caller being freed before this call returns.
	*/
	void		(*pfnServerExecute)			(void);
	
	/**
	*	Sends a client command to the given client.
	*	@param pEdict Edict of the client that should execute the command.
	*	@param pszFormat Format string.
	*	@param ... Format arguments.
	*/
	void		(*pfnClientCommand)			(edict_t* pEdict, char* szFmt, ...);
	
	/**
	*	Creates a particle effect.
	*	@param org Origin in the world.
	*	@param dir Direction of the effect.
	*	@param color Color of the effect.
	*	@param count Number of particles to create.
	*/
	void		(*pfnParticleEffect)		(const float *org, const float *dir, float color, float count);
	
	/**
	*	Sets the given light style to the given value.
	*	@param style Style index.
	*	@param val Value to set. This string must live for at least as long as the map itself.
	*/
	void		(*pfnLightStyle)			(int style, char* val);
	
	/**
	*	Gets the index of the given decal.
	*	@param name Name of the decal.
	*	@return Index of the decal, or -1 if the decal couldn't be found.
	*/
	int			(*pfnDecalIndex)			(const char *name);
	
	/**
	*	Gets the contents of the given location in the world.
	*	@param rgflVector Location in the world.
	*	@return Contents of the location in the world. @see Contents
	*/
	int			(*pfnPointContents)			(const float *rgflVector);
	
	/**
	*	Begins a new network message.
	*	@param msg_dest Message type.
	*	@param msg_type Message ID.
	*	@param pOrigin Optional. Origin to use for PVS and PAS checks.
	*	@param pEdict Optional. If it's a message to one client, client to send the message to.
	*
	*	If the message type is to one client, and no client is provided, triggers a sys error.
	*	If the message type is to all clients, and a client is provided, triggers a sys error.
	*	If another message had already been started and was not ended, triggers a sys error.
	*	If an invalid message ID is provided, triggers a sys error.
	*/
	void		(*pfnMessageBegin)			(int msg_dest, int msg_type, const float *pOrigin, edict_t *ed);
	
	/**
	*	Ends a network message.
	*
	*	If no message had been started, triggers a sys error.
	*	If the buffer had overflowed, triggers a sys error.
	*	If the message is a user message, and exceeds 192 bytes, triggers a host error.
	*	If the message has a fixed size and the wrong size was written, triggers a sys error.
	*	If the given client is invalid, triggers a host error.
	*/
	void		(*pfnMessageEnd)			(void);
	
	/**
	*	Writes a single unsigned byte.
	*
	*	If no message had been started, triggers a sys error.
	*/
	void		(*pfnWriteByte)				(int iValue);
	
	/**
	*	Writes a single character.
	*
	*	If no message had been started, triggers a sys error.
	*/
	void		(*pfnWriteChar)				(int iValue);
	
	/**
	*	Writes a single unsigned short.
	*
	*	If no message had been started, triggers a sys error.
	*/
	void		(*pfnWriteShort)			(int iValue);
	
	/**
	*	Writes a single unsigned int.
	*
	*	If no message had been started, triggers a sys error.
	*/
	void		(*pfnWriteLong)				(int iValue);
	
	/**
	*	Writes a single angle value.
	*
	*	If no message had been started, triggers a sys error.
	*/
	void		(*pfnWriteAngle)			(float flValue);
	
	/**
	*	Writes a single coordinate value.
	*
	*	If no message had been started, triggers a sys error.
	*/
	void		(*pfnWriteCoord)			(float flValue);
	
	/**
	*	Writes a single null terminated string.
	*
	*	If no message had been started, triggers a sys error.
	*/
	void		(*pfnWriteString)			(const char *sz);
	
	/**
	*	Writes a single entity index.
	*
	*	If no message had been started, triggers a sys error.
	*/
	void		(*pfnWriteEntity)			(int iValue);

	/**
	*	Registers a cvar.
	*	Sets the flag FCVAR_EXTDLL on the cvar.
	*	@param pCvar Cvar to register.
	*/
	void		(*pfnCVarRegister)			(cvar_t *pCvar);

	/**
	*	Gets the value of a cvar as a float.
	*	@param szVarName Name of the cvar whose value is to be retrieved.
	*	@return Value of the cvar, or 0 if the cvar doesn't exist.
	*/
	float		(*pfnCVarGetFloat)			(const char *szVarName);

	/**
	*	Gets the value of a cvar as a string.
	*	@param szVarName Name of the cvar whose value is to be retrieved.
	*	@return Value of the cvar, or an empty string if the cvar doesn't exist.
	*/
	const char*	(*pfnCVarGetString)			(const char *szVarName);

	/**
	*	Sets the value of a cvar as a float.
	*	@param szVarName Name of the cvar whose value to set.
	*	@param flValue Value to set.
	*/
	void		(*pfnCVarSetFloat)			(const char *szVarName, float flValue);

	/**
	*	Sets the value of a cvar as a string.
	*	@param szVarName Name of the cvar whose value to set.
	*	@param szValue Value to set. The string is copied.
	*/
	void		(*pfnCVarSetString)			(const char *szVarName, const char *szValue);

	/**
	*	Outputs a message to the server console.
	*	If aType is at_logged and this is a multiplayer game, logs the message to the log file.
	*	Otherwise, if the developer cvar is not 0, outputs the message to the console.
	*	@param atype Type of message.
	*	@param szFmt Format string.
	*	@param ... Format arguments.
	*/
	void		(*pfnAlertMessage)			(ALERT_TYPE atype, char *szFmt, ...);

	/**
	*	Obsolete. Will print a message to the server console using pfnAlertMessage indicating if it's being used.
	*/
	void		(*pfnEngineFprintf)			(void *pfile, char *szFmt, ...);

	/**
	*	Allocates memory for CBaseEntity instances.
	*	The memory will be zeroed out.
	*	@param pEdict Entity to allocate memory for.
	*	@param cb Number of bytes to allocate.
	*	@return Pointer to CBaseEntity memory, or null if it could not be allocated.
	*/
	void*		(*pfnPvAllocEntPrivateData)	(edict_t *pEdict, int32 cb);

	/**
	*	@param pEdict Entity whose entity memory is to be retrieved.
	*	@return Pointer to entity CBaseEntity instance, or null if pEdict is null or there is no entity assigned to it.
	*/
	void*		(*pfnPvEntPrivateData)		(edict_t *pEdict);

	/**
	*	Frees the CBaseEntity memory assigned to pEdict.
	*	@param pEdict Entity whose memory should be freed.
	*/
	void		(*pfnFreeEntPrivateData)	(edict_t *pEdict);

	/**
	*	Gets the string assigned to the index.
	*	If the index is invalid, returns a pointer to invalid memory.
	*	@param iString String index whose string should be retrieved.
	*	@return String.
	*/
	const char*	(*pfnSzFromIndex)			(int iString);

	/**
	*	Allocates a string in the string pool.
	*	This will allocate memory from the hunk. If the hunk runs out of memory, will trigger sys error.
	*	Each call allocates new memory. No actual pooling of strings occurs.
	*	@param szValue String to allocate.
	*	@param Index assigned to the string.
	*/
	int			(*pfnAllocString)			(const char *szValue);

	/**
	*	Gets the entvars_t instance assigned to the given edict_t instance.
	*	In effect, returns &pEdict->v.
	*	If pEdict is null, causes a crash.
	*	@param pEdict Edict whose entvars is to be retrieved.
	*	@return entvars.
	*/
	struct entvars_s*	(*pfnGetVarsOfEnt)			(edict_t *pEdict);

	/**
	*	Gets an edict by offset.
	*	This uses the byte offset of the edict to retrieve it.
	*	DO NOT USE THIS. Use pfnPEntityOfEntIndex.
	*	@param iEntOffset Entity offset.
	*	@return Edict at the given offset.
	*/
	edict_t*	(*pfnPEntityOfEntOffset)	(int iEntOffset);

	/**
	*	Gets the entity offset of the edict.
	*	DO NOT USE THIS. Use pfnIndexOfEdict.
	*	@param pEdict Edict whose offset is to be retrieved.
	*	@return Entity offset.
	*/
	int			(*pfnEntOffsetOfPEntity)	(const edict_t *pEdict);

	/**
	*	Gets the entity index of the edict.
	*	@param pEdict Edict whose entity index is to be retrieved.
	*	@return	If pEdict is null, returns 0.
	*			If pEdict is not managed by the engine, triggers a sys error.
	*			Otherwise, returns the entity index.
	*/
	int			(*pfnIndexOfEdict)			(const edict_t *pEdict);

	/**
	*	Gets the edict at the given entity index.
	*	@param iEntIndex Entity index.
	*	@return	If the given index is not valid, returns null.
	*			Otherwise, if the entity at the given index is not in use, returns null.
	*			Otherwise, if the entity at the given index is not a player and does not have a CBaseEntity instance, returns null.
	*			Otherwise, returns the entity.
	*/
	edict_t*	(*pfnPEntityOfEntIndex)		(int iEntIndex);

	/**
	*	Gets the edict of an entvars.
	*	This will enumerate all entities, so this operation can be very expensive.
	*	Use pvars->pContainingEntity if possible.
	*	@param pvars Entvars.
	*	@return Edict.
	*/
	edict_t*	(*pfnFindEntityByVars)		(struct entvars_s* pvars);

	/**
	*	Gets the model pointer of the given entity.
	*	@param pEdict Entity.
	*	@return	Pointer to the model, or null if the entity doesn't have one.
	*			Triggers a sys error if the model wasn't loaded and couldn't be loaded.
	*/
	void*		(*pfnGetModelPtr)			(edict_t* pEdict);

	/**
	*	Registers a user message.
	*	The name of the message is used to find an exported function in the client library.
	*	The format is MsgFunc_<name>.
	*	@param pszName Name of the message. Maximum length is 12, excluding null terminator. Can be a temporary string.
	*	@param iSize Size of the message, in bytes. Maximum size is 192 bytes. Specify -1 for variable length messages.
	*	@return Message ID, or 0 if the message could not be registered.
	*/
	int			(*pfnRegUserMsg)			(const char *pszName, int iSize);

	/**
	*	Does nothing.
	*/
	void		(*pfnAnimationAutomove)		(const edict_t* pEdict, float flTime);

	/**
	*	Gets the bone position and angles for the given entity and bone.
	*	If the given entity is invalid, or does not have a studio model, or the bone index is invalid, will cause invalid accesses to occur.
	*	@param pEdict Entity whose model should be queried.
	*	@param iBone Bone index.
	*	@param rgflOrigin Origin of the bone.
	*	@param rgflAngles Angles of the bone.
	*	TODO: rgflAngles does not appear to be set. Verify results.
	*/
	void		(*pfnGetBonePosition)		(const edict_t* pEdict, int iBone, float *rgflOrigin, float *rgflAngles );
	
	/**
	*	Gets the index of an exported function.
	*	@param pszName Name of the function.
	*	@return Index of the function, or 0 if the function couldn't be found.
	*/
	uint32 (*pfnFunctionFromName)	( const char *pName );
	
	/**
	*	Gets the name of an exported function.
	*	@param function Function index.
	*	@return Function name, or null if no function exists at that index.
	*/
	const char *(*pfnNameForFunction)		( uint32 function );

	/**
	*	Sends a message to the client console.
	*	print_chat outputs to the console, just as print_console.
	*	@param pEdict Client to send the message to.
	*	@param ptype Where to print the message. @see PRINT_TYPE
	*	@param szMsg Message to send.
	*/
	void		(*pfnClientPrintf)			( edict_t* pEdict, PRINT_TYPE ptype, const char *szMsg ); // JOHN: engine callbacks so game DLL can print messages to individual clients
	
	/**
	*	Sends a message to the server console.
	*	The message is output regardless of the value of the developer cvar.
	*/
	void		(*pfnServerPrint)			( const char *szMsg );
	
	/**
	*	@return String containing all of the command arguments, not including the command name.
	*/
	const char *(*pfnCmd_Args)				( void );		// these 3 added 
	
	/**
	*	Gets the command argument at the given index.
	*	Argument 0 is the command name.
	*	@param argc Argument index.
	*	@return Command argument.
	*/
	const char *(*pfnCmd_Argv)				( int argc );	// so game DLL can easily 
	
	/**
	*	@return The number of command arguments. This includes the command name.
	*/
	int			(*pfnCmd_Argc)				( void );		// access client 'cmd' strings
	
	/**
	*	Gets the attachment origin and angles.
	*	If the entity is null, or does not have a studio model, illegal access will occur.
	*	@param pEdict Entity whose model will be queried for the attachment data.
	*	@param iAttachment Attachment index.
	*	@param rgflOrigin Attachment origin.
	*	@param rgflAngles Attachment angles.
	*/
	void		(*pfnGetAttachment)			(const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles );
	
	/**
	*	Initializes the CRC instance.
	*/
	void		(*pfnCRC32_Init)			(CRC32_t *pulCRC);
	
	/**
	*	Processes a buffer and updates the CRC.
	*	@param pulCRC CRC instance.
	*	@param p Buffer to process.
	*	@param len Number of bytes in the buffer to process.
	*/
	void        (*pfnCRC32_ProcessBuffer)   (CRC32_t *pulCRC, void *p, int len);

	/**
	*	Processes a single byte.
	*	@param pulCRC CRC instance.
	*	@param ch Byte.
	*/
	void		(*pfnCRC32_ProcessByte)     (CRC32_t *pulCRC, unsigned char ch);

	/**
	*	Finalizes the CRC instance.
	*	@param pulCRC CRC instance.
	*	@return CRC value.
	*/
	CRC32_t		(*pfnCRC32_Final)			(CRC32_t pulCRC);

	/**
	*	Generates a random long number in the range [ lLow, lHigh ].
	*	@param lLow Lower bound.
	*	@param lHigh Higher bound.
	*	@return Random number, or lLow if lHigh is smaller than or equal to lLow.
	*/
	int32		(*pfnRandomLong)			(int32  lLow,  int32  lHigh);

	/**
	*	Generates a random float number in the range [ flLow, flLow ].
	*	@param flLow Lower bound.
	*	@param flHigh Higher bound.
	*	@return Random number.
	*/
	float		(*pfnRandomFloat)			(float flLow, float flHigh);

	/**
	*	Sets the view of a client to the given entity.
	*	If pClient is not a client, triggers a host error.
	*	Set the view to the client itself to reset it.
	*	@param pClient Client whose view is to be set.
	*	@param pViewent Entity to use as the client's viewpoint.
	*/
	void		(*pfnSetView)				(const edict_t *pClient, const edict_t *pViewent );

	/**
	*	@return The time since the first call to Time.
	*	Used for delta operations that operate in real world time, as opposed to game world time (which will advance frame by frame, and can be paused).
	*/
	float		(*pfnTime)					( void );

	/**
	*	Sets the angles of the given player's crosshairs to the given settings.
	*	Set both to 0 to disable.
	*	@param pClient Client whose crosshair settings should be set.
	*	@param pitch Pitch.
	*	@param yaw Yaw.
	*/
	void		(*pfnCrosshairAngle)		(const edict_t *pClient, float pitch, float yaw);

	/**
	*	Loads a file from disk.
	*	@param filename Name of the file. Path starts in the game directory.
	*	@param pLength If not null, is set to the size of the file, in bytes.
	*	@return Pointer to the file buffer, or null if the file could not be loaded.
	*/
	byte *      (*pfnLoadFileForMe)         (char *filename, int *pLength);

	/**
	*	Frees the buffer provided by pfnLoadFileForMe.
	*	@param buffer Pointer to buffer.
	*	@see pfnLoadFileForMe
	*/
	void        (*pfnFreeFile)              (void *buffer);

	/**
	*	Signals the engine that a section has ended.
	*	Possible values:
	*	_oem_end_training
	*	_oem_end_logo
	*	_oem_end_demo
	*
	*	A disconnect command is sent by this call.
	*/
	void        (*pfnEndSection)            (const char *pszSectionName); // trigger_endsection

	/**
	*	Compares file times.
	*	@param filename1 First file to compare.
	*	@param filename2 Second file to compare.
	*	@param iCompare		If both files are equal, 0.
	*						If the first file is older, -1.
	*						If the second file is older, 1.
	*	@return true if both filenames are non-null, false otherwise.
	*/
	int 		(*pfnCompareFileTime)       (char *filename1, char *filename2, int *iCompare);

	/**
	*	Gets the game directory name.
	*	@param szGetGameDir Buffer to store the game directory name in. Must be at least MAX_PATH bytes large.
	*/
	void        (*pfnGetGameDir)            (char *szGetGameDir);

	/**
	*	Registers a Cvar. Identical to CvarRegister, except it doesn't set the FCVAR_EXTDLL flag.
	*/
	void		(*pfnCvar_RegisterVariable) (cvar_t *variable);

	/**
	*	Fades the given client's volume.
	*	@param pEdict Client.
	*	@param fadePercent Percentage volume to fade out to.
	*	@param fadeOutSeconds How long it takes to fade out, in seconds.
	*	@param holdTime	How long to stay faded out, in seconds.
	*	@param fadeInSeconds How long it takes to fade in, in seconds.
	*/
	void        (*pfnFadeClientVolume)      (const edict_t *pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds);
	
	/**
	*	Sets the client's maximum speed value.
	*	Effectively sets pEdict->v.maxspeed.
	*	@param pEdict Client to set.
	*	@param fNewMaxspeed Maximum speed value.
	*/
	void        (*pfnSetClientMaxspeed)     (const edict_t *pEdict, float fNewMaxspeed);
	
	/**
	*	Creates a fake client (bot).
	*	@param netname Name of the client to show.
	*	@return The fake client, or null if it can't be created.
	*/
	edict_t *	(*pfnCreateFakeClient)		(const char *netname);	// returns NULL if fake client can't be created
	
	/**
	*	Runs player movement for a fake client.
	*	@param fakeclient client to move. Must be a fake client.
	*	@param viewangles Client view angles.
	*	@param forwardmove Velocity X value.
	*	@param sidemove Velocity Y value.
	*	@param upmove Velocity Z value.
	*	@param buttons Buttons that are currently pressed in. Equivalent to player pev.button.
	*	@param impulse Impulse commands to execute. Equivalent to player pev.impulse.
	*	@param msec Time between now and previous RunPlayerMove call.
	*/
	void		(*pfnRunPlayerMove)			(edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec );
	
	/**
	*	Computes the total number of entities currently in existence.
	*	Note: this will calculate the number of entities in real-time. May be expensive if called many times.
	*	@return Number of entities.
	*/
	int			(*pfnNumberOfEntities)		(void);
	
	/**
	*	Gets the given client's info key buffer.
	*	Passing in the world gets the serverinfo.
	*	Passing in null gets the localinfo. Localinfo is not used by the engine itself, only the game.
	*	Note: this function checks the maxplayers value incorrectly and may crash if the wrong edict gets passed in.
	*	@param e Client.
	*	@return Info key buffer.
	*/
	char*		(*pfnGetInfoKeyBuffer)		(edict_t *e);	// passing in NULL gets the serverinfo
	
	/**
	*	Gets the value of the given key from the given buffer.
	*	@param infobuffer Buffer to query.
	*	@param key Key whose value to retrieve.
	*	@return The requested value, or an empty string.
	*/
	char*		(*pfnInfoKeyValue)			(char *infobuffer, char *key);
	
	/**
	*	Sets the value of the given key in the given buffer.
	*	If the given buffer is not the localinfo or serverinfo buffer, triggers a sys error.
	*	@param infobuffer Buffer to modify.
	*	@param key Key whose value to set.
	*	@param value Value to set.
	*/
	void		(*pfnSetKeyValue)			(char *infobuffer, char *key, char *value);
	
	/**
	*	Sets the value of the given key in the given buffer.
	*	This only works for client buffers.
	*	@param clientIndex Entity index of the client.
	*	@param infobuffer Buffer to modify.
	*	@param key Key whose value to set.
	*	@param value Value to set.
	*/
	void		(*pfnSetClientKeyValue)		(int clientIndex, char *infobuffer, char *key, char *value);
	
	/**
	*	Checks if the given filename is a valid map.
	*	@param filename Name of the map to check.
	*	@return true if the map is valid, false otherwise.
	*/
	int			(*pfnIsMapValid)			(char *filename);
	
	/**
	*	Projects a static decal in the world.
	*	@param origin Origin in the world to project the decal at.
	*	@param decalIndex Index of the decal to project.
	*	@param entityIndex Index of the entity to project the decal onto.
	*	@param modelIndex Index of the model to project the decal onto.
	*/
	void		(*pfnStaticDecal)			( const float *origin, int decalIndex, int entityIndex, int modelIndex );
	
	/**
	*	Precaches a file.
	*	If pszFilename is null, is empty or contains an invalid value in the first character, triggers a host error.
	*	If the maximum number of generic precacheable resources has been reached, triggers a host error.
	*	If this is called after ServerActivate, triggers a host error.
	*	@param s Name of the file to precache. Starts in the game directory. This string must life for at least as long as the map itself.
	*	@return Index of the file.
	*/
	int			(*pfnPrecacheGeneric)		(char* s);
	
	/**
	*	Returns the server assigned userid for this player.
	*	Useful for logging frags, etc.
	*	Returns -1 if the edict couldn't be found in the list of clients.
	*	@param e Client.
	*	@return User ID, or -1.
	*/
	int			(*pfnGetPlayerUserId)		(edict_t *e ); // returns the server assigned userid for this player.  useful for logging frags, etc.  returns -1 if the edict couldn't be found in the list of clients
	
	/**
	*	Builds a sound message to send to a client.
	*	@param entity Entity that is playing the sound.
	*	@param channel Channel to play the sound on.
	*	@param sample Sound to play.
	*	@param volume Volume of the sound. Must be in the range [ 0, 1 ].
	*	@param attenuation Attenuation.
	*	@param fFlags Sound flags.
	*	@param pitch Pitch. Must be in the range [ 0, 255 ].
	*	@param msg_dest Message type.
	*	@param msg_type Message ID.
	*	@param pOrigin Origin in the world to use for PAS and PVS messages.
	*	@param ed Client to send the message to for message types that target one client.
	*/
	void		(*pfnBuildSoundMsg)			(edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, edict_t *ed);
	
	/**
	*	@return Whether this is a dedicated server.
	*/
	int			(*pfnIsDedicatedServer)		(void);// is this a dedicated server?
	
	/**
	*	@param szVarName Name of the cvar to retrieve.
	*	@return Cvar pointer, or null if the cvar doesn't exist.
	*/
	cvar_t		*(*pfnCVarGetPointer)		(const char *szVarName);
	
	/**
	*	Returns the server assigned WONid for this player.
	*	Useful for logging frags, etc.
	*	Returns -1 if the edict couldn't be found in the list of clients.
	*	Always returns -1 when using the Steam version.
	*	@param e Client.
	*	@return WON ID, or -1.
	*/
	unsigned int (*pfnGetPlayerWONId)		(edict_t *e); // returns the server assigned WONid for this player.  useful for logging frags, etc.  returns -1 if the edict couldn't be found in the list of clients

	// YWB 8/1/99 TFF Physics additions
	
	/**
	*	Removes a key from the info buffer.
	*	@param s Buffer to modify.
	*	@param key Key to remove.
	*/
	void		(*pfnInfo_RemoveKey)		( char *s, const char *key );
	
	/**
	*	Gets the given physics keyvalue from the given client's buffer.
	*	@param pClient Client whose buffer will be queried.
	*	@param key Key whose value will be retrieved.
	*	@return The value, or an empty string if the key does not exist.
	*/
	const char *(*pfnGetPhysicsKeyValue)	( const edict_t *pClient, const char *key );
	
	/**
	*	Sets the given physics keyvalue in the given client's buffer.
	*	@param pClient Client whose buffer will be modified.
	*	@param key Key whose value will be set.
	*	@param value Value to set.
	*/
	void		(*pfnSetPhysicsKeyValue)	( const edict_t *pClient, const char *key, const char *value );
	
	/**
	*	Gets the physics info string for the given client.
	*	@param pClient whose buffer will be retrieved.
	*	@return Buffer, or an empty string if the client is invalid.
	*/
	const char *(*pfnGetPhysicsInfoString)	( const edict_t *pClient );
	
	/**
	*	Precaches an event.
	*	The client will have to hook the event using cl_enginefunc_t::pfnHookEvent.
	*	@param type Should always be 1.
	*	@param psz Name of the event. Format should be events/<name>.sc, including the directory and extension.
	*	@return Event index. Used with pfnPlaybackEvent
	*	@see cl_enginefunc_t::pfnHookEvent
	*	@see pfnPlaybackEvent
	*/
	unsigned short (*pfnPrecacheEvent)		( int type, const char*psz );
	
	/**
	*	@param flags Event flags.
	*	@param pInvoker Client that triggered the event.
	*	@param eventindex Event index. @see pfnPrecacheEvent
	*	@param delay Delay before the event should be run.
	*	@param origin If not g_vecZero, this is the origin parameter sent to the clients.
	*	@param angles If not g_vecZero, this is the angles parameter sent to the clients.
	*	@param fparam1 Float parameter 1.
	*	@param fparam2 Float parameter 2.
	*	@param iparam1 Integer parameter 1.
	*	@param iparam2 Integer parameter 2.
	*	@param bparam1 Boolean parameter 1.
	*	@param bparam2 Boolean parameter 2.
	*/
	void		(*pfnPlaybackEvent)			( int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );

	/**
	*	Adds the given origin to the current PVS.
	*	@return PVS data.
	*/
	unsigned char *(*pfnSetFatPVS)			( float *org );

	/**
	*	Adds the given origin to the current PAS.
	*	@return PAS data.
	*/
	unsigned char *(*pfnSetFatPAS)			( float *org );

	/**
	*	Checks if the given entity is visible in the given visible set.
	*	@param entity Entity to check.
	*	@param pset Buffer detailing the current visible set.
	*	@return Whether the given entity is visible in the given visible set.
	*/
	int			(*pfnCheckVisibility )		( const edict_t *entity, unsigned char *pset );

	/**
	*	Marks the given field in the given list as set.
	*	@param pFields List of fields.
	*	@param fieldname Field name.
	*/
	void		(*pfnDeltaSetField)			( struct delta_s *pFields, const char *fieldname );

	/**
	*	Marks the given field in the given list as not set.
	*	@param pFields List of fields.
	*	@param fieldname Field name.
	*/
	void		(*pfnDeltaUnsetField)		( struct delta_s *pFields, const char *fieldname );

	/**
	*	Adds a delta encoder.
	*	@param name Name of the delta.lst entry to add the encoder for.
	*	@param conditionalencode Encoder function.
	*/
	void		(*pfnDeltaAddEncoder)		( char *name, void (*conditionalencode)( struct delta_s *pFields, const unsigned char *from, const unsigned char *to ) );
	
	/**
	*	@return	The client index of the client that is currently being handled by an engine callback.
	*			Returns -1 if no client is currently being handled.
	*/
	int			(*pfnGetCurrentPlayer)		( void );
	
	/**
	*	@return true if the given client has cl_lw (weapon prediction) enabled.
	*/
	int			(*pfnCanSkipPlayer)			( const edict_t *player );

	/**
	*	Finds the index of a delta field.
	*	@param pFields List of fields.
	*	@param fieldname Name of the field to find.
	*	@return Index of the delta field, or -1 if the field couldn't be found.
	*/
	int			(*pfnDeltaFindField)		( struct delta_s *pFields, const char *fieldname );

	/**
	*	Marks a delta field as set by index.
	*	If the index is invalid, causes illegal access.
	*	@param pFields List of fields.
	*	@param fieldNumber Index of the field.
	*/
	void		(*pfnDeltaSetFieldByIndex)	( struct delta_s *pFields, int fieldNumber );

	/**
	*	Marks a delta field as not set by index.
	*	If the index is invalid, causes illegal access.
	*	@param pFields List of fields.
	*	@param fieldNumber Index of the field.
	*/
	void		(*pfnDeltaUnsetFieldByIndex)( struct delta_s *pFields, int fieldNumber );

	/**
	*	Used to filter contents checks.
	*	@param mask Mask to check.
	*	@param op Operation to perform during masking.
	*	@see GroupTraceOp
	*/
	void		(*pfnSetGroupMask)			( int mask, int op );

	/**
	*	Creates an instanced baseline. Used to define a baseline for a particular entity type.
	*	@param classname Name of the entity class.
	*	@param baseline Baseline to set.
	*/
	int			(*pfnCreateInstancedBaseline) ( int classname, struct entity_state_s *baseline );

	/**
	*	Directly sets a cvar value.
	*	@param var Cvar.
	*	@param value Value to set.
	*/
	void		(*pfnCvar_DirectSet)		( struct cvar_s *var, char *value );

	/*
	*	Forces the client and server to be running with the same version of the specified file
	*	( e.g., a player model ).
	*	Calling this has no effect in single player
	*	@param type Force type @see FORCE_TYPE
	*	@param mins If not null, the minimum bounds that a model can be.
	*	@param maxs If not null, the maximum bounds that a model can be.
	*	@param filename File to verify. This string must life for at least as long as the map itself.
	*/
	void		(*pfnForceUnmodified)		( FORCE_TYPE type, float *mins, float *maxs, const char *filename );

	/**
	*	Get player statistics.
	*	@param pClient Client to query.
	*	@param ping Current ping.
	*	@param packet_loss Current packet loss, measured in percentage.
	*/
	void		(*pfnGetPlayerStats)		( const edict_t *pClient, int *ping, int *packet_loss );

	/**
	*	Adds a server command.
	*	@param cmd_name Name of the command to add.
	*	@param function Function to invoke when the command is received.
	*/
	void		(*pfnAddServerCommand)		( char *cmd_name, void (*function) (void) );

	// For voice communications, set which clients hear eachother.
	// NOTE: these functions take player entity indices (starting at 1).

	/**
	*	Gets whether the given receiver can hear the given sender.
	*	@param iReceiver Receiver. This is an entity index.
	*	@param iSender Sender. This is an entity index.
	*	@return Whether the given receiver can hear the given sender.
	*/
	qboolean	(*pfnVoice_GetClientListening)(int iReceiver, int iSender);

	/**
	*	Sets whether the given receiver can hear the given sender.
	*	@param iReceiver Receiver. This is an entity index.
	*	@param iSender Sender. This is an entity index.
	*	@param bListen Whether the given receiver can hear the given sender.
	*	@return Whether the setting was changed.
	*/
	qboolean	(*pfnVoice_SetClientListening)(int iReceiver, int iSender, qboolean bListen);

	/**
	*	Gets the player's auth ID.
	*	@param e Client.
	*	@return The player's auth ID, or an empty string. This points to a temporary buffer, copy the results.
	*/
	const char *(*pfnGetPlayerAuthId)		( edict_t *e );

	// PSV: Added for CZ training map
//	const char *(*pfnKeyNameForBinding)		( const char* pBinding );
	
	/**
	*	Gets the sequence that has the given entry name.
	*	@param fileName Ignored.
	*	@param entryName Entry name.
	*	@return Sequence, or null if no such sequence exists.
	*/
	sequenceEntry_s*	(*pfnSequenceGet)			( const char* fileName, const char* entryName );

	/**
	*	Picks a sentence from the given group.
	*	@param groupName Group from which to select a sentence.
	*	@param pickMethod Ignored.
	*	@param picked If not null, this is set to the index of the sentence that was picked.
	*	@return Sentence that was picked, or null if there is no group by that name, or no sentences in the group.
	*/
	sentenceEntry_s*	(*pfnSequencePickSentence)	( const char* groupName, int pickMethod, int *picked );

	/**
	*	LH: Give access to filesize via filesystem
	*	@param filename Name of the file whose size is to be queried
	*	@return File size, or -1 if the file doesn't exist.
	*/
	int			(*pfnGetFileSize)			( char *filename );

	/**
	*	Gets the average wave length in seconds.
	*	@param filepath Name of the sound.
	*	@return Length of the sound file, in seconds.
	*/
	unsigned int (*pfnGetApproxWavePlayLen) (const char *filepath);
	
	// MDC: Added for CZ career-mode
	/**
	*	@return Whether this is a Condition Zero Career match.
	*/
	int			(*pfnIsCareerMatch)			( void );

	/**
	*	BGC
	*	@param label Label
	*	@return Number of characters of the localized string referenced by using "label".
	*/
	int			(*pfnGetLocalizedStringLength)(const char *label);

	// BGC: added to facilitate persistent storage of tutor message decay values for
	// different career game profiles.  Also needs to persist regardless of mp.dll being
	// destroyed and recreated.
	/**
	*	Marks the message with the given ID as having been shown.
	*	@param mid Message ID.
	*/
	void (*pfnRegisterTutorMessageShown)(int mid);

	/**
	*	Gets the number of times the message with the given ID has been shown.
	*	@param mid Message ID.
	*	@return Number of times the message with the given ID has been shown.
	*/
	int (*pfnGetTimesTutorMessageShown)(int mid);

	/**
	*	Processes the tutor message decay buffer.
	*	@param buffer Buffer.
	*	@param bufferLength Size of the buffer, in bytes.
	*/
	void (*ProcessTutorMessageDecayBuffer)(int *buffer, int bufferLength);

	/**
	*	Constructs the tutor message decay buffer.
	*	@param buffer Buffer.
	*	@param bufferLength Size of the buffer, in bytes.
	*/
	void (*ConstructTutorMessageDecayBuffer)(int *buffer, int bufferLength);

	/**
	*	Resets tutor message decay data.
	*/
	void (*ResetTutorMessageDecayData)( void );

	/**
	*	Queries the given client for a cvar value.
	*	The response is sent to NEW_DLL_FUNCTIONS::pfnCvarValue
	*	@param player Player to query.
	*	@param cvarName Cvar to query.
	*/
	void (*pfnQueryClientCvarValue)( const edict_t *player, const char *cvarName );

	/**
	*	Queries the given client for a cvar value.
	*	The response is sent to NEW_DLL_FUNCTIONS::pfnCvarValue2
	*	@param player Player to query.
	*	@param cvarName Cvar to query.
	*	@param requestID Request ID to pass to pfnCvarValue2
	*/
	void (*pfnQueryClientCvarValue2)( const edict_t *player, const char *cvarName, int requestID );

	/**
	*	Checks if a command line parameter was provided.
	*	@param pchCmdLineToken Command key to look for.
	*	@param ppnext If the key was found, this is set to the value.
	*	@return Key index in the command line buffer, or 0 if it wasn't found.
	*/
	int (*pfnCheckParm)( const char *pchCmdLineToken, char **ppnext );
} enginefuncs_t;


// ONLY ADD NEW FUNCTIONS TO THE END OF THIS STRUCT.  INTERFACE VERSION IS FROZEN AT 138

// Passed to pfnKeyValue
typedef struct KeyValueData_s
{
	char	*szClassName;	// in: entity classname
	char	*szKeyName;		// in: name of key
	char	*szValue;		// in: value of key
	int32	fHandled;		// out: DLL sets to true if key-value pair was understood
} KeyValueData;


typedef struct
{
	char		mapName[ 32 ];
	char		landmarkName[ 32 ];
	edict_t	*pentLandmark;
	vec3_t		vecLandmarkOrigin;
} LEVELLIST;
#define MAX_LEVEL_CONNECTIONS	16		// These are encoded in the lower 16bits of ENTITYTABLE->flags

typedef struct 
{
	int			id;				// Ordinal ID of this entity (used for entity <--> pointer conversions)
	edict_t	*pent;			// Pointer to the in-game entity

	int			location;		// Offset from the base data of this entity
	int			size;			// Byte size of this entity's data
	int			flags;			// This could be a short -- bit mask of transitions that this entity is in the PVS of
	string_t	classname;		// entity class name

} ENTITYTABLE;

#define FENTTABLE_PLAYER		0x80000000
#define FENTTABLE_REMOVED		0x40000000
#define FENTTABLE_MOVEABLE		0x20000000
#define FENTTABLE_GLOBAL		0x10000000

typedef struct saverestore_s SAVERESTOREDATA;

#ifdef _WIN32
typedef 
#endif
struct saverestore_s
{
	char		*pBaseData;		// Start of all entity save data
	char		*pCurrentData;	// Current buffer pointer for sequential access
	int			size;			// Current data size
	int			bufferSize;		// Total space for data
	int			tokenSize;		// Size of the linear list of tokens
	int			tokenCount;		// Number of elements in the pTokens table
	char		**pTokens;		// Hash table of entity strings (sparse)
	int			currentIndex;	// Holds a global entity table ID
	int			tableCount;		// Number of elements in the entity table
	int			connectionCount;// Number of elements in the levelList[]
	ENTITYTABLE	*pTable;		// Array of ENTITYTABLE elements (1 for each entity)
	LEVELLIST	levelList[ MAX_LEVEL_CONNECTIONS ];		// List of connections from this level

	// smooth transition
	int			fUseLandmark;
	char		szLandmarkName[20];// landmark we'll spawn near in next level
	vec3_t		vecLandmarkOffset;// for landmark transitions
	float		time;
	char		szCurrentMapName[32];	// To check global entities

} 
#ifdef _WIN32
SAVERESTOREDATA 
#endif
;

typedef enum _fieldtypes
{
	FIELD_FLOAT = 0,		// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_ENTITY,			// An entity offset (EOFFSET)
	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EVARS,			// EVARS *
	FIELD_EDICT,			// edict_t *, or edict_t *  (same thing)
	FIELD_VECTOR,			// Any vector
	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_POINTER,			// Arbitrary data pointer... to be removed, use an array of FIELD_CHARACTER
	FIELD_INTEGER,			// Any integer or enum
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)

	FIELD_TYPECOUNT,		// MUST BE LAST
} FIELDTYPE;

#if !defined(offsetof)  && !defined(GNUC)
#define offsetof(s,m)	(size_t)&(((s *)0)->m)
#endif

#define _FIELD(type,name,fieldtype,count,flags)		{ fieldtype, #name, offsetof(type, name), count, flags }
#define DEFINE_FIELD(type,name,fieldtype)			_FIELD(type, name, fieldtype, 1, 0)
#define DEFINE_ARRAY(type,name,fieldtype,count)		_FIELD(type, name, fieldtype, count, 0)
#define DEFINE_ENTITY_FIELD(name,fieldtype)			_FIELD(entvars_t, name, fieldtype, 1, 0 )
#define DEFINE_ENTITY_GLOBAL_FIELD(name,fieldtype)	_FIELD(entvars_t, name, fieldtype, 1, FTYPEDESC_GLOBAL )
#define DEFINE_GLOBAL_FIELD(type,name,fieldtype)	_FIELD(type, name, fieldtype, 1, FTYPEDESC_GLOBAL )


#define FTYPEDESC_GLOBAL			0x0001		// This field is masked for global entity save/restore

typedef struct 
{
	FIELDTYPE		fieldType;
	char			*fieldName;
	int				fieldOffset;
	short			fieldSize;
	short			flags;
} TYPEDESCRIPTION;

#define ARRAYSIZE(p)		(sizeof(p)/sizeof(p[0]))

typedef struct 
{
	/**
	*	Called when the game loads this DLL.
	*/
	void			(*pfnGameInit)			( void );

	/**
	*	Called by the engine to spawn an entity.
	*	@param pEnt Entity to spawn.
	*	@return 0 if the entity was successfully spawned, or -1 if it should be removed.
	*/
	int				(*pfnSpawn)				( edict_t *pent );

	/**
	*	Called by the engine to run this entity's think function.
	*/
	void			(*pfnThink)				( edict_t *pent );

	/**
	*	Called by the engine to trigger pEntUsed's Use function, using pEntOther as the activator and caller.
	*	Obsolete. This is never called by the engine.
	*/
	void			(*pfnUse)				( edict_t *pentUsed, edict_t *pentOther );

	/**
	*	Called by the engine to run pEntTouched's Touch function with pEntOther as the other entity.
	*/
	void			(*pfnTouch)				( edict_t *pentTouched, edict_t *pentOther );

	/**
	*	Called by the engine to run pEntBlocked's Blocked function with pEntOther as the other entity.
	*/
	void			(*pfnBlocked)			( edict_t *pentBlocked, edict_t *pentOther );

	/**
	*	Called by the engine to run pEntKeyvalue's KeyValue function with pkvd as the keyvalue data.
	*/
	void			(*pfnKeyValue)			( edict_t *pentKeyvalue, KeyValueData *pkvd );

	/**
	*	Called by the engine to save the given entity's state to the given save data block.
	*/
	void			(*pfnSave)				( edict_t *pent, SAVERESTOREDATA *pSaveData );

	/**
	*	Called by the engine to restore the given entity's state from the given save data block.
	*	@param globalEntity Boolean indicating whether this entity has a global name, or was transitioned from another map.
	*/
	int 			(*pfnRestore)			( edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity );

	/**
	*	Called by the engine to set the given entity's absolute bounding box.
	*/
	void			(*pfnSetAbsBox)			( edict_t *pent );

	/**
	*	Called by the engine to save a named block of data to the given save data block.
	*	@param pSaveData Block to save data to.
	*	@param pszName Name of the data block.
	*	@param pBaseData Pointer to the object containing the data.
	*	@param pFields List of type descriptions describing the object's data.
	*	@param fieldCount Number of type descriptions in pFields.
	*/
	void			(*pfnSaveWriteFields)	( SAVERESTOREDATA *, const char *, void *, TYPEDESCRIPTION *, int );
	
	/**
	*	Called by the engine to restore a named block of data from the given save data block.
	*	@param pSaveData Block to restore data from.
	*	@param pszName Name of the data block.
	*	@param pBaseData Pointer to the object containing the data.
	*	@param pFields List of type descriptions describing the object's data.
	*	@param fieldCount Number of type descriptions in pFields.
	*/
	void			(*pfnSaveReadFields)	( SAVERESTOREDATA *, const char *, void *, TYPEDESCRIPTION *, int );

	/**
	*	Called by the engine to save global state.
	*/
	void			(*pfnSaveGlobalState)		( SAVERESTOREDATA * );

	/**
	*	Called by the engine to restore global state.
	*/
	void			(*pfnRestoreGlobalState)	( SAVERESTOREDATA * );

	/**
	*	Called by the engine to reset global state.
	*/
	void			(*pfnResetGlobalState)		( void );

	/**
	*	Called by the engine when a client connects.
	*	Returning false rejects the client's connection. Setting szRejectReason presents that to the client.
	*	Note: network messages cannot be sent at this time.
	*
	*	@param pEntity Entity that represents this client.
	*	@param pszName Netname of the player.
	*	@param pszAddress IP address of the client.
	*	@param szRejectReason Reason why the client was rejected.
	*	@return true if the client should be allowed to connect, false otherwise.
	*/
	qboolean		(*pfnClientConnect)		( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
	
	/**
	*	Called when a client disconnects. This will not be called if the client connection was rejected in pfnClientConnect.
	*	@see pfnClientConnect
	*/
	void			(*pfnClientDisconnect)	( edict_t *pEntity );

	/**
	*	Called by the engine when the player has issued a "kill" command. Only if the player's health is larger than 0.
	*/
	void			(*pfnClientKill)		( edict_t *pEntity );

	/**
	*	Called by the engine when the client has finished connecting.
	*	This is where the player should be spawned and put into the world, or given a spectator position to view from.
	*	Note: network messages cannot be sent at this time.
	*/
	void			(*pfnClientPutInServer)	( edict_t *pEntity );

	/**
	*	Called by the engine when it has received a command from the given client.
	*	Command arguments can be retrieved using enginefuncs_t::pfnCmd_Args, enginefuncs_t::pfnCmd_Argv, enginefuncs_t::pfnCmd_Argc.
	*	@see enginefuncs_t::pfnCmd_Args
	*	@see enginefuncs_t::pfnCmd_Argv
	*	@see enginefuncs_t::pfnCmd_Argc
	*/
	void			(*pfnClientCommand)		( edict_t *pEntity );

	/**
	*	Called by the engine whenever the client's user info string changes.
	*	This includes the player's model.
	*/
	void			(*pfnClientUserInfoChanged)( edict_t *pEntity, char *infobuffer );

	/**
	*	Called when the engine has finished spawning the map.
	*	@param pEdictList Pointer to the list of edicts.
	*	@param edictCount Number of valid edicts.
	*	@param clientMax Maximum number of players that can connect to this server.
	*/
	void			(*pfnServerActivate)	( edict_t *pEdictList, int edictCount, int clientMax );

	/**
	*	Called when the map has ended. This happens before entities are destroyed.
	*/
	void			(*pfnServerDeactivate)	( void );

	/**
	*	Called by the engine before it runs player think.
	*/
	void			(*pfnPlayerPreThink)	( edict_t *pEntity );

	/**
	*	Called by the engine after it runs player think.
	*/
	void			(*pfnPlayerPostThink)	( edict_t *pEntity );

	/**
	*	Called at the start of a server game frame.
	*/
	void			(*pfnStartFrame)		( void );

	/**
	*	Obsolete.
	*/
	void			(*pfnParmsNewLevel)		( void );

	/**
	*	Called by the engine when a level is saved.
	*	Also called when a player has spawned after a saved game has been loaded.
	*/
	void			(*pfnParmsChangeLevel)	( void );

	/*
	*	Returns string describing current .dll.  E.g., TeamFortress 2, Half-Life
	*/
	const char     *(*pfnGetGameDescription)( void );     

	/**
	*	Notify dll about a player customization.
	*/
	void            (*pfnPlayerCustomization) ( edict_t *pEntity, customization_t *pCustom );  

	// Spectator funcs
	/**
	*	Called when a HLTV spectator has connected.
	*/
	void			(*pfnSpectatorConnect)		( edict_t *pEntity );

	/**
	*	Called when a HLTV spectator has disconnected.
	*/
	void			(*pfnSpectatorDisconnect)	( edict_t *pEntity );

	/**
	*	Called when a HLTV spectator's think function has to run.
	*/
	void			(*pfnSpectatorThink)		( edict_t *pEntity );

	/**
	*	Notify game .dll that engine is going to shut down.  Allows mod authors to set a breakpoint.
	*/
	void			(*pfnSys_Error)			( const char *error_string );

	/**
	*	Called by the engine to run player physics.
	*	@param ppmove Pointer to player movement data.
	*	@param server Whether this is the server or client physics code.
	*/
	void			(*pfnPM_Move) ( struct playermove_s *ppmove, qboolean server );

	/**
	*	Called by the engine to initialize the player physics data.
	*/
	void			(*pfnPM_Init) ( struct playermove_s *ppmove );

	/**
	*	Called by the engine to find the texture type of a given texture.
	*	Never actually called.
	*	@param name Name of the texture to look up.
	*	@return Texture type. Should always return a valid value, even if no data is available for the given texture.
	*/
	char			(*pfnPM_FindTextureType)( char *name );

	/**
	*	Set up visibility for the given client.
	*	@param pViewEntity The client's view entity. This is the entity whose origin and view offset should be used as the client's view position.
	*	@param pClient The client.
	*	@param[ out ] pvs Pointer to Potentially Visible Set to use.
	*	@param[ out ] pas Pointer to Potentially Audible Set to use.
	*/
	void			(*pfnSetupVisibility)( struct edict_s *pViewEntity, struct edict_s *pClient, unsigned char **pvs, unsigned char **pas );
	
	/**
	*	Updates the given client's data.
	*	This function can be used to implement first person observer views.
	*	@param ent Client.
	*	@param sendweapons Boolean indicating whether weapon data should be sent.
	*	@param cd Client data to send. This is zeroed before the call to this function.
	*/
	void			(*pfnUpdateClientData) ( const struct edict_s *ent, int sendweapons, struct clientdata_s *cd );
	
	/**
	*	Called by the engine to determine whether the given entity should be added to the given client's list of visible entities.
	*	@param state state Entity state data for the entity that is being added.
	*	@param entIndex Index of the entity being considered for addition.
	*	@param ent Entity being considered for addition.
	*	@param host Client currently being processed.
	*	@param hostflags Host flags. @see HostFlag
	*	@param player true if the entity being added is a player, false otherwise.
	*	@param pSet The PVS provided by pfnSetupVisibility @see pfnSetupVisibility
	*/
	int				(*pfnAddToFullPack)( struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet );
	
	/**
	*	Called by the engine to create a baseline for the given entity.
	*	@param player Boolean indicating whether this is a player.
	*	@param eindex Entity index of this entity.
	*	@param baseline Baseline to fill.
	*	@param entity Entity to make the baseline for.
	*	@param playermodelindex Index of the model "models/player.mdl".
	*	@param player_mins Array of the player minimum bounds for each hull.
	*	@param player_maxs Array of the player maximum bounds for each hull.
	*/
	void			(*pfnCreateBaseline) ( int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs );
	
	/**
	*	Called by the engine to allow the server to register delta encoders.
	*	@see enginefuncs_t::pfnDeltaAddEncoder
	*/
	void			(*pfnRegisterEncoders)	( void );

	/**
	*	Called by the engine to retrieve weapon data.
	*	@param player Player to retrieve weapon info from.
	*	@param info Array of weapon_data_t that should receive the player's weapon data. Is an array of MAX_WEAPONS entries.
	*	@return true if data was added, false otherwise.
	*/
	int				(*pfnGetWeaponData)		( struct edict_s *player, struct weapon_data_s *info );

	/**
	*	Called by the engine when a user command has been received and is about to begin processing.
	*	@param player Player.
	*	@param cmd Command being executed.
	*	@param random_seed The player's current random seed.
	*/
	void			(*pfnCmdStart)			( const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed );
	
	/**
	*	Called by the engine when a user command has finished processing.
	*	@param player Player.
	*/
	void			(*pfnCmdEnd)			( const edict_t *player );

	/**
	*	Return 1 if the packet is valid. Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
	*	size of the response_buffer, so you must zero it out if you choose not to respond.
	*
	*	This can be used to handle commands akin to rcon. The remote server console can send commands that end up here.
	*	Note that anyone can send messages that reach this point, not just rcon.
	*
	*	@param net_from IP address of the sender of this packet.
	*	@param args Arguments provided by the sender.
	*	@param response_buffer Buffer that a response can be written into.
	*	@param response_buffer_size Size of the buffer.
	*	@return true if the packet has been handled, false otherwise.
	*/
	int				(*pfnConnectionlessPacket )	( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );

	/**
	*	Enumerates player hulls. Returns 0 if the hull number doesn't exist, 1 otherwise.
	*	@param hullnumber Hull to retrieve the bounds for.
	*	@param mins Minimum bounds.
	*	@param maxs Maximum bounds.
	*	@return true if the hull was successfully retrieved, false otherwise.
	*/
	int				(*pfnGetHullBounds)	( int hullnumber, float *mins, float *maxs );

	/**
	*	Create baselines for certain "unplaced" items.
	*	@see enginefuncs_t::pfnCreateInstancedBaseline
	*/
	void			(*pfnCreateInstancedBaselines) ( void );

	/**
	*	One of the pfnForceUnmodified files failed the consistency check for the specified player
	*	Return 0 to allow the client to continue, 1 to force immediate disconnection ( with an optional disconnect message of up to 256 characters )
	*	@see enginefuncs_t::pfnForceUnmodified
	*/
	int				(*pfnInconsistentFile)( const struct edict_s *player, const char *filename, char *disconnect_message );

	/**
	*	The game .dll should return 1 if lag compensation should be allowed ( could also just set
	*	 the sv_unlag cvar.
	*	Most games right now should return 0, until client-side weapon prediction code is written
	*	 and tested for them.
	*/
	int				(*pfnAllowLagCompensation)( void );
} DLL_FUNCTIONS;

extern DLL_FUNCTIONS		gEntityInterface;

// Current version.
#define NEW_DLL_FUNCTIONS_VERSION	1

typedef struct
{
	/**
	*	Called when an entity is freed by the engine, right before the object's memory is freed. Calls OnDestroy and runs the destructor.
	*/
	void			(*pfnOnFreeEntPrivateData)(edict_t *pEnt);

	/**
	*	Called when the game unloads this DLL.
	*/
	void			(*pfnGameShutdown)(void);

	/**
	*	Called when the engine believes two entities are about to collide. Return 0 if you
	*	want the two entities to just pass through each other without colliding or calling the
	*	touch function.
	*/
	int				(*pfnShouldCollide)( edict_t *pentTouched, edict_t *pentOther );

	/**
	*	Called when the engine has received a cvar value from the client in response to an enginefuncs_t::pfnQueryClientCvarValue call.
	*	If the client isn't connected, or the cvar didn't exist, the value given is "Bad CVAR request".
	*	@param pEnt Client entity.
	*	@param value Cvar value.
	*/
	void			(*pfnCvarValue)( const edict_t *pEnt, const char *value );

	/**
	*	Called when the engine has received a cvar value from the client in response to a enginefuncs_t::pfnQueryClientCvarValue2 call.
	*	@param requestID The ID given to the pfnQueryClientCvarValue2 function.
	*	@param cvarName Name of the cvar that was queried.
	*	@see pfnCvarValue
	*/
	void			(*pfnCvarValue2)( const edict_t *pEnt, int requestID, const char *cvarName, const char *value );
} NEW_DLL_FUNCTIONS;
typedef int	(*NEW_DLL_FUNCTIONS_FN)( NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion );

// Pointers will be null if the game DLL doesn't support this API.
extern NEW_DLL_FUNCTIONS	gNewDLLFunctions;

typedef int	(*APIFUNCTION)( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion );
typedef int	(*APIFUNCTION2)( DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion );

#endif EIFACE_H
