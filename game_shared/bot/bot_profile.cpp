//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Author: Michael S. Booth (mike@turtlerockstudios.com), 2003

#pragma warning( disable : 4530 )					// STL uses exceptions, but we are not compiling with them - ignore warning

#define DEFINE_DIFFICULTY_NAMES
#include "bot_profile.h"
#include "shared_util.h"
#include "simple_checksum.h"

BotProfileManager *TheBotProfiles = NULL;

//--------------------------------------------------------------------------------------------------------------
// A little explanation is in order here.  This file is in game_shared.  It is supposed to be able to be compiled
// into multiple dlls.  However, all the basic utility functions (loading files, parsing strings, etc) are
// accessed different ways in different dlls.  To combat this, we need to redirect some calls depending on
// the dll we're in.
#ifdef GAMEUI_EXPORTS
	#include "../GameUI/EngineInterface.h"
	#include "../cstrike/dlls/weapontype.h"
	#include "../cstrike/dlls/weapontype.cpp"	/// @TODO: remove this CStrike peculiarity from a game_shared file!!!
	void Career_Printf(const char *fmt, ...);
	#define CONSOLE_ECHO Career_Printf
	#define LOAD_FILE_FOR_ME(name, len) (engine->COM_LoadFile( (name), 5, (len) ))
	#define FREE_FILE (engine->COM_FreeFile)
	#define UTIL_IsGame( x ) 0
#else
	#include "extdll.h"
	#include "util.h"
	#include "cbase.h"
	#include "weapons.h"
	#include "soundent.h"
	#include "gamerules.h"
	#include "player.h"
	#include "client.h"
	#include "cmd.h"
	#include "pm_shared.h"
	#include "bot.h"
	#include "bot_util.h"
#endif

//--------------------------------------------------------------------------------------------------------
/**
 * Generates a filename-decorated skin name
 */
static const char * GetDecoratedSkinName( const char *name, const char *filename )
{
#ifdef _WIN32
	const int BufLen = _MAX_PATH + 64;
#else
	const int BufLen = MAX_PATH + 64;
#endif
	static char buf[BufLen];
	snprintf( buf, BufLen, "%s/%s", filename, name );
	return buf;
}

//--------------------------------------------------------------------------------------------------------------
const char* BotProfile::GetWeaponPreferenceAsString( int i ) const
{
	if ( i < 0 || i >= m_weaponPreferenceCount )
		return NULL;

	return WeaponIDToAlias( m_weaponPreference[ i ] );
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if this profile has a primary weapon preference
 */
bool BotProfile::HasPrimaryPreference( void ) const
{
	for( int i=0; i<m_weaponPreferenceCount; ++i )
	{
		int weaponClass = AliasToWeaponClass( WeaponIDToAlias( m_weaponPreference[i] ) );
		
		if (weaponClass == WEAPONCLASS_SUBMACHINEGUN ||
				weaponClass == WEAPONCLASS_SHOTGUN ||
				weaponClass == WEAPONCLASS_MACHINEGUN ||
				weaponClass == WEAPONCLASS_RIFLE ||
				weaponClass == WEAPONCLASS_SNIPERRIFLE)
			return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if this profile has a pistol weapon preference
 */
bool BotProfile::HasPistolPreference( void ) const
{
	for( int i=0; i<m_weaponPreferenceCount; ++i )
		if (AliasToWeaponClass( WeaponIDToAlias( m_weaponPreference[i] ) ) == WEAPONCLASS_PISTOL)
			return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if this profile is valid for the specified team
 */
bool BotProfile::IsValidForTeam( BotProfileTeamType team ) const
{
	return ( team == BOT_TEAM_ANY || m_teams == BOT_TEAM_ANY || team == m_teams );
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Constructor
 */
BotProfileManager::BotProfileManager( void )
{
	m_nextSkin = 0;
	for (int i=0; i<NumCustomSkins; ++i)
	{
		m_skins[i] = NULL;
		m_skinFilenames[i] = NULL;
		m_skinModelnames[i] = NULL;
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Load the bot profile database
 */
void BotProfileManager::Init( const char *filename, unsigned int *checksum )
{
	int dataLength;
	char *dataPointer = (char *)LOAD_FILE_FOR_ME( const_cast<char *>( filename ), &dataLength );
	const char *dataFile = dataPointer;

	if (dataFile == NULL)
	{
		if ( UTIL_IsGame( "czero" ) )
		{
			CONSOLE_ECHO( "WARNING: Cannot access bot profile database '%s'\n", filename );
		}
		return;
	}

	// compute simple checksum
	if (checksum)
	{
		*checksum = ComputeSimpleChecksum( (const unsigned char *)dataPointer, dataLength );
	}

	// keep list of templates used for inheritance
	BotProfileList templateList;

	BotProfile defaultProfile;

	//
	// Parse the BotProfile.db into BotProfile instances
	//
	while( true )
	{
		dataFile = SharedParse( dataFile );
		if (!dataFile)
			break;

		char *token = SharedGetToken();

		bool isDefault = (!stricmp( token, "Default" ));
		bool isTemplate = (!stricmp( token, "Template" ));
		bool isCustomSkin = (!stricmp( token, "Skin" ));

		if ( isCustomSkin )
		{
			const int BufLen = 64;
			char skinName[BufLen];

			// get skin name
			dataFile = SharedParse( dataFile );
			if (!dataFile)
			{
				CONSOLE_ECHO( "Error parsing %s - expected skin name\n", filename );
				FREE_FILE( dataPointer );
				return;
			}
			token = SharedGetToken();
			snprintf( skinName, BufLen, "%s", token );

			// get attribute name
			dataFile = SharedParse( dataFile );
			if (!dataFile)
			{
				CONSOLE_ECHO( "Error parsing %s - expected 'Model'\n", filename );
				FREE_FILE( dataPointer );
				return;
			}
			token = SharedGetToken();
			if (stricmp( "Model", token ))
			{
				CONSOLE_ECHO( "Error parsing %s - expected 'Model'\n", filename );
				FREE_FILE( dataPointer );
				return;
			}

			// eat '='
			dataFile = SharedParse( dataFile );
			if (!dataFile)
			{
				CONSOLE_ECHO( "Error parsing %s - expected '='\n", filename );
				FREE_FILE( dataPointer );
				return;
			}
			token = SharedGetToken();
			if (strcmp( "=", token ))
			{
				CONSOLE_ECHO( "Error parsing %s - expected '='\n", filename );
				FREE_FILE( dataPointer );
				return;
			}

			// get attribute value
			dataFile = SharedParse( dataFile );
			if (!dataFile)
			{
				CONSOLE_ECHO( "Error parsing %s - expected attribute value\n", filename );
				FREE_FILE( dataPointer );
				return;
			}
			token = SharedGetToken();

			const char *decoratedName = GetDecoratedSkinName( skinName, filename );
			bool skinExists = GetCustomSkinIndex( decoratedName ) > 0;
			if ( m_nextSkin < NumCustomSkins && !skinExists )
			{
				// decorate the name
				m_skins[ m_nextSkin ] = CloneString( decoratedName );

				// construct the model filename
				m_skinModelnames[ m_nextSkin ] = CloneString( token );
				m_skinFilenames[ m_nextSkin ] = new char[ strlen(token)*2 + strlen("models/player//.mdl") + 1 ];
				sprintf( m_skinFilenames[ m_nextSkin ], "models/player/%s/%s.mdl", token, token );
				++m_nextSkin;
			}

			// eat 'End'
			dataFile = SharedParse( dataFile );
			if (!dataFile)
			{
				CONSOLE_ECHO( "Error parsing %s - expected 'End'\n", filename );
				FREE_FILE( dataPointer );
				return;
			}
			token = SharedGetToken();
			if (strcmp( "End", token ))
			{
				CONSOLE_ECHO( "Error parsing %s - expected 'End'\n", filename );
				FREE_FILE( dataPointer );
				return;
			}

			continue; // it's just a custom skin - no need to do inheritance on a bot profile, etc.
		}

		// encountered a new profile
		BotProfile *profile;

		if (isDefault)
		{
			profile = &defaultProfile;
		}
		else
		{
			profile = new BotProfile;

			// always inherit from Default
			*profile = defaultProfile;
		}

		// do inheritance in order of appearance
		if (!isTemplate && !isDefault)
		{
			const BotProfile *inherit = NULL;

			// template names are separated by "+"
			while(true)
			{
				char *c = strchr( token, '+' );
				if (c)
					*c = '\000';

				// find the given template name
				for( BotProfileList::iterator iter = templateList.begin(); iter != templateList.end(); ++iter )
				{
					if (!stricmp( (*iter)->GetName(), token ))
					{
						inherit = *iter;
						break;
					}
				}

				if (inherit == NULL)
				{
					CONSOLE_ECHO( "Error parsing '%s' - invalid template reference '%s'\n", filename, token );
					FREE_FILE( dataPointer );
					return;
				}

				// inherit the data
				profile->Inherit( inherit, &defaultProfile );

				if (c == NULL)
					break;
				
				token = c+1;
			}
		}


		// get name of this profile
		if (!isDefault)
		{
			dataFile = SharedParse( dataFile );
			if (!dataFile)
			{
				CONSOLE_ECHO( "Error parsing '%s' - expected name\n", filename );
				FREE_FILE( dataPointer );
				return;
			}
			profile->m_name = CloneString( SharedGetToken() );

			/**
			 * HACK HACK
			 * Until we have a generalized means of storing bot preferences, we're going to hardcode the bot's
			 * preference towards silencers based on his name.
			 */
			if ( profile->m_name[0] % 2 )
			{
				profile->m_prefersSilencer = true;
			}
		}

		// read attributes for this profile
		bool isFirstWeaponPref = true;
		while( true )
		{
			// get next token
			dataFile = SharedParse( dataFile );
			if (!dataFile)
			{
				CONSOLE_ECHO( "Error parsing %s - expected 'End'\n", filename );
				FREE_FILE( dataPointer );
				return;
			}
			token = SharedGetToken();

			// check for End delimiter
			if (!stricmp( token, "End" ))
				break;

			// found attribute name - keep it
			char attributeName[64];
			strcpy( attributeName, token );

			// eat '='
			dataFile = SharedParse( dataFile );
			if (!dataFile)
			{
				CONSOLE_ECHO( "Error parsing %s - expected '='\n", filename );
				FREE_FILE( dataPointer );
				return;
			}

			token = SharedGetToken();
			if (strcmp( "=", token ))
			{
				CONSOLE_ECHO( "Error parsing %s - expected '='\n", filename );
				FREE_FILE( dataPointer );
				return;
			}

			// get attribute value
			dataFile = SharedParse( dataFile );
			if (!dataFile)
			{
				CONSOLE_ECHO( "Error parsing %s - expected attribute value\n", filename );
				FREE_FILE( dataPointer );
				return;
			}
			token = SharedGetToken();

			// store value in appropriate attribute
			if (!stricmp( "Aggression", attributeName ))
			{
				profile->m_aggression = atof(token) / 100.0f;
			}
			else if (!stricmp( "Skill", attributeName ))
			{
				profile->m_skill = atof(token) / 100.0f;
			}
			else if (!stricmp( "Skin", attributeName ))
			{
				profile->m_skin = atoi(token);
				if ( profile->m_skin == 0 )
				{
					// atoi() failed - try to look up a custom skin by name
					profile->m_skin = GetCustomSkinIndex( token, filename );
				}
			}
			else if (!stricmp( "Teamwork", attributeName ))
			{
				profile->m_teamwork = atof(token) / 100.0f;
			}
			else if (!stricmp( "Cost", attributeName ))
			{
				profile->m_cost = atoi(token);
			}
			else if (!stricmp( "VoicePitch", attributeName ))
			{
				profile->m_voicePitch = atoi(token);
			}
			else if (!stricmp( "VoiceBank", attributeName ))
			{
				profile->m_voiceBank = FindVoiceBankIndex( token );
			}
			else if (!stricmp( "WeaponPreference", attributeName ))
			{
				// weapon preferences override parent prefs
				if (isFirstWeaponPref)
				{
					isFirstWeaponPref = false;
					profile->m_weaponPreferenceCount = 0;
				}

				if (!stricmp( token, "none" ))
				{
					profile->m_weaponPreferenceCount = 0;
				}
				else
				{
					if (profile->m_weaponPreferenceCount < BotProfile::MAX_WEAPON_PREFS)
					{
						profile->m_weaponPreference[ profile->m_weaponPreferenceCount++ ] = AliasToWeaponID( token );
					}
				}
			}
			else if (!stricmp( "ReactionTime", attributeName ))
			{
				profile->m_reactionTime = atof(token);

#ifndef GAMEUI_EXPORTS
				// subtract off latency due to "think" update rate.
				// In GameUI, we don't really care.
				profile->m_reactionTime -= g_flBotFullThinkInterval;
#endif

			}
			else if (!stricmp( "AttackDelay", attributeName ))
			{
				profile->m_attackDelay = atof(token);
			}
			else if (!stricmp( "Difficulty", attributeName ))
			{
				// override inheritance
				profile->m_difficultyFlags = 0;

				// parse bit flags
				while(true)
				{
					char *c = strchr( token, '+' );
					if (c)
						*c = '\000';

					for( int i=0; i<NUM_DIFFICULTY_LEVELS; ++i )
						if (!stricmp( BotDifficultyName[i], token ))
							profile->m_difficultyFlags |= (1 << i);

					if (c == NULL)
						break;
					
					token = c+1;
				}
			}
			else if (!stricmp( "Team", attributeName ))
			{
				if ( !stricmp( token, "T" ) )
				{
					profile->m_teams = BOT_TEAM_T;
				}
				else if ( !stricmp( token, "CT" ) )
				{
					profile->m_teams = BOT_TEAM_CT;
				}
				else
				{
					profile->m_teams = BOT_TEAM_ANY;
				}
			}
			else
			{
				CONSOLE_ECHO( "Error parsing %s - unknown attribute '%s'\n", filename, attributeName );
			}
		}

		if (!isDefault)
		{
			if (isTemplate)
			{
				// add to template list
				templateList.push_back( profile );
			}
			else
			{
				// add profile to the master list
				m_profileList.push_back( profile );
			}
		}
	}

	FREE_FILE( dataPointer );

	// free the templates
	for( BotProfileList::iterator iter = templateList.begin(); iter != templateList.end(); ++iter )
		delete *iter;
}

//--------------------------------------------------------------------------------------------------------------
BotProfileManager::~BotProfileManager( void )
{
	Reset();

	VoiceBankList::iterator it;
	for ( it = m_voiceBanks.begin(); it != m_voiceBanks.end(); ++it )
	{
		delete[] *it;
	}
	m_voiceBanks.clear();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Free all bot profiles
 */
void BotProfileManager::Reset( void )
{
	for( BotProfileList::iterator iter = m_profileList.begin(); iter != m_profileList.end(); ++iter )
		delete *iter;

	m_profileList.clear();

	for (int i=0; i<NumCustomSkins; ++i)
	{
		if ( m_skins[i] )
		{
			delete[] m_skins[i];
			m_skins[i] = NULL;
		}
		if ( m_skinFilenames[i] )
		{
			delete[] m_skinFilenames[i];
			m_skinFilenames[i] = NULL;
		}
		if ( m_skinModelnames[i] )
		{
			delete[] m_skinModelnames[i];
			m_skinModelnames[i] = NULL;
		}
	}
}

//--------------------------------------------------------------------------------------------------------
/**
 * Returns custom skin name at a particular index
 */
const char * BotProfileManager::GetCustomSkin( int index )
{
	if ( index < FirstCustomSkin || index > LastCustomSkin )
	{
		return NULL;
	}

	return m_skins[ index - FirstCustomSkin ];
}

//--------------------------------------------------------------------------------------------------------
/**
 * Returns custom skin filename at a particular index
 */
const char * BotProfileManager::GetCustomSkinFname( int index )
{
	if ( index < FirstCustomSkin || index > LastCustomSkin )
	{
		return NULL;
	}

	return m_skinFilenames[ index - FirstCustomSkin ];
}

//--------------------------------------------------------------------------------------------------------
/**
 * Returns custom skin modelname at a particular index
 */
const char * BotProfileManager::GetCustomSkinModelname( int index )
{
	if ( index < FirstCustomSkin || index > LastCustomSkin )
	{
		return NULL;
	}

	return m_skinModelnames[ index - FirstCustomSkin ];
}

//--------------------------------------------------------------------------------------------------------
/**
 * Looks up a custom skin index by filename-decorated name (will decorate the name if filename is given)
 */
int BotProfileManager::GetCustomSkinIndex( const char *name, const char *filename )
{
	const char * skinName = name;
	if ( filename )
	{
		skinName = GetDecoratedSkinName( name, filename );
	}

	for (int i=0; i<NumCustomSkins; ++i)
	{
		if ( m_skins[i] )
		{
			if ( !stricmp( skinName, m_skins[i] ) )
			{
				return FirstCustomSkin + i;
			}
		}
	}
	return 0;
}


//--------------------------------------------------------------------------------------------------------
/**
 * return index of the (custom) bot phrase db, inserting it if needed
 */
int BotProfileManager::FindVoiceBankIndex( const char *filename )
{
	int index = 0;

	VoiceBankList::const_iterator it;
	for ( it = m_voiceBanks.begin(); it != m_voiceBanks.end(); ++it, ++index )
	{
		if ( !stricmp( filename, *it ) )
		{
			return index;
		}
	}

	m_voiceBanks.push_back( CloneString( filename ) );
	return index;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return random unused profile that matches the given difficulty level
 */
const BotProfile *BotProfileManager::GetRandomProfile( BotDifficultyType difficulty, BotProfileTeamType team ) const
{
#ifndef RANDOM_LONG
	return NULL;	// we don't need random profiles when we're not in the game dll
#else
	BotProfileList::const_iterator iter;

	// count up valid profiles
	int validCount = 0;
	for( iter = m_profileList.begin(); iter != m_profileList.end(); ++iter )
	{
		const BotProfile *profile = *iter;

		if (profile->IsDifficulty( difficulty ) && !UTIL_IsNameTaken( profile->GetName() ) && profile->IsValidForTeam( team ))
			++validCount;
	}

	if (validCount == 0)
		return NULL;

	// select one at random
	int which = RANDOM_LONG( 0, validCount-1 );
	for( iter = m_profileList.begin(); iter != m_profileList.end(); ++iter )
	{
		const BotProfile *profile = *iter;

		if (profile->IsDifficulty( difficulty ) && !UTIL_IsNameTaken( profile->GetName() ) && profile->IsValidForTeam( team ))
			if (which-- == 0)
				return profile;
	}

	return NULL;
#endif;
}

