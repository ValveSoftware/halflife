//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Author: Michael S. Booth (mike@turtlerockstudios.com), 2003

#ifndef _BOT_PROFILE_H_
#define _BOT_PROFILE_H_

#pragma warning( disable : 4786 )	// long STL names get truncated in browse info.
#ifdef LINUX
#include <strings.h>
#include <stdio.h>
#endif

#undef min
#undef max
#include <list>
#include <vector>
#include "bot_constants.h"

enum
{
	FirstCustomSkin = 100,
	NumCustomSkins = 100,
	LastCustomSkin = FirstCustomSkin + NumCustomSkins - 1,
};

enum BotProfileTeamType
{
	BOT_TEAM_T,
	BOT_TEAM_CT,
	BOT_TEAM_ANY
};

//--------------------------------------------------------------------------------------------------------------
/**
 * A BotProfile describes the "personality" of a given bot
 */
class BotProfile
{
public:
	BotProfile( void )
	{
		m_name = NULL;
		m_aggression = 0.0f;
		m_skill = 0.0f;
		m_teamwork = 0.0f;
		m_weaponPreferenceCount = 0;
		m_cost = 0;
		m_skin = 0;
		m_difficultyFlags = 0;
		m_voicePitch = 100;
		m_reactionTime = 0.3f;
		m_attackDelay = 0.0f;
		m_teams = BOT_TEAM_ANY;
		m_voiceBank = 0;
		m_prefersSilencer = false;
	}

	const char *GetName( void ) const					{ return m_name; }		///< return bot's name
	float GetAggression( void ) const					{ return m_aggression; }
	float GetSkill( void ) const							{ return m_skill; }
	float GetTeamwork( void ) const						{ return m_teamwork; }

	int GetWeaponPreference( int i ) const		{ return m_weaponPreference[ i ]; }
	const char *GetWeaponPreferenceAsString( int i ) const;
	int GetWeaponPreferenceCount( void ) const	{ return m_weaponPreferenceCount; }
	bool HasPrimaryPreference( void ) const;		///< return true if this profile has a primary weapon preference
	bool HasPistolPreference( void ) const;			///< return true if this profile has a pistol weapon preference

	int GetCost( void ) const									{ return m_cost; }
	int GetSkin( void ) const									{ return m_skin; }
	bool IsDifficulty( BotDifficultyType diff ) const;		///< return true if this profile can be used for the given difficulty level
	int GetVoicePitch( void ) const						{ return m_voicePitch; }
	float GetReactionTime( void ) const				{ return m_reactionTime; }
	float GetAttackDelay( void ) const				{ return m_attackDelay; }
	int GetVoiceBank() const									{ return m_voiceBank; }

	bool IsValidForTeam( BotProfileTeamType team ) const;

	bool PrefersSilencer() const							{ return m_prefersSilencer; }
private:	
	friend class BotProfileManager;			///< for loading profiles

	void Inherit( const BotProfile *parent, const BotProfile *baseline );	///< copy values from parent if they differ from baseline

	char *m_name;												///< the bot's name
	float m_aggression;									///< percentage: 0 = coward, 1 = berserker
	float m_skill;											///< percentage: 0 = terrible, 1 = expert
	float m_teamwork;										///< percentage: 0 = rogue, 1 = complete obeyance to team, lots of comm

	enum { MAX_WEAPON_PREFS = 16 };
	int m_weaponPreference[ MAX_WEAPON_PREFS ];	///< which weapons this bot likes to use, in order of priority
	int m_weaponPreferenceCount;

	int m_cost;													///< reputation point cost for career mode
	int m_skin;													///< "skin" index
	unsigned char m_difficultyFlags;		///< bits set correspond to difficulty levels this is valid for
	int m_voicePitch;										///< the pitch shift for bot chatter (100 = normal)
	float m_reactionTime;								///< our reaction time in seconds
	float m_attackDelay;								///< time in seconds from when we notice an enemy to when we open fire
	BotProfileTeamType m_teams;					///< teams for which this profile is valid

	bool m_prefersSilencer;							///< does the bot prefer to use silencers?

	int m_voiceBank;										///< Index of the BotChatter.db voice bank this profile uses (0 is the default)
};
typedef std::list<BotProfile *> BotProfileList;


inline bool BotProfile::IsDifficulty( BotDifficultyType diff ) const
{
	return (m_difficultyFlags & (1 << diff)) ? true : false;
}

/**
 * Copy in data from parent if it differs from the baseline
 */
inline void BotProfile::Inherit( const BotProfile *parent, const BotProfile *baseline )
{
	if (parent->m_aggression != baseline->m_aggression)
		m_aggression = parent->m_aggression;

	if (parent->m_skill != baseline->m_skill)
		m_skill = parent->m_skill;

	if (parent->m_teamwork != baseline->m_teamwork)
		m_teamwork = parent->m_teamwork;

	if (parent->m_weaponPreferenceCount != baseline->m_weaponPreferenceCount)
	{
		m_weaponPreferenceCount = parent->m_weaponPreferenceCount;
		for( int i=0; i<parent->m_weaponPreferenceCount; ++i )
			m_weaponPreference[i] = parent->m_weaponPreference[i];
	}

	if (parent->m_cost != baseline->m_cost)
		m_cost = parent->m_cost;

	if (parent->m_skin != baseline->m_skin)
		m_skin = parent->m_skin;

	if (parent->m_difficultyFlags != baseline->m_difficultyFlags)
		m_difficultyFlags = parent->m_difficultyFlags;

	if (parent->m_voicePitch != baseline->m_voicePitch)
		m_voicePitch = parent->m_voicePitch;

	if (parent->m_reactionTime != baseline->m_reactionTime)
		m_reactionTime = parent->m_reactionTime;

	if (parent->m_attackDelay != baseline->m_attackDelay)
		m_attackDelay = parent->m_attackDelay;

	if (parent->m_teams != baseline->m_teams)
		m_teams = parent->m_teams;

	if (parent->m_voiceBank != baseline->m_voiceBank)
		m_voiceBank = parent->m_voiceBank;
}




//--------------------------------------------------------------------------------------------------------------
/**
 * The BotProfileManager defines the interface to accessing BotProfiles
 */
class BotProfileManager
{
public:
	BotProfileManager( void );
	~BotProfileManager( void );

	void Init( const char *filename, unsigned int *checksum = NULL );
	void Reset( void );

	/// given a name, return a profile
	const BotProfile *GetProfile( const char *name, BotProfileTeamType team ) const
	{
		for( BotProfileList::const_iterator iter = m_profileList.begin(); iter != m_profileList.end(); ++iter )
			if ( !stricmp( name, (*iter)->GetName() ) && (*iter)->IsValidForTeam( team ) )
				return *iter;

		return NULL;
	}

	const BotProfileList *GetProfileList( void ) const		{ return &m_profileList; }		///< return list of all profiles

	const BotProfile *GetRandomProfile( BotDifficultyType difficulty, BotProfileTeamType team ) const;			///< return random unused profile that matches the given difficulty level

	const char * GetCustomSkin( int index );				///< Returns custom skin name at a particular index
	const char * GetCustomSkinModelname( int index );		///< Returns custom skin modelname at a particular index
	const char * GetCustomSkinFname( int index );			///< Returns custom skin filename at a particular index
	int GetCustomSkinIndex( const char *name, const char *filename = NULL );	///< Looks up a custom skin index by name

	typedef std::vector<char *> VoiceBankList;
	const VoiceBankList* GetVoiceBanks() const { return &m_voiceBanks; }
	int FindVoiceBankIndex( const char *filename );		///< return index of the (custom) bot phrase db, inserting it if needed

protected:
	BotProfileList m_profileList;							///< the list of all bot profiles

	VoiceBankList m_voiceBanks;

	char *m_skins[ NumCustomSkins ];						///< Custom skin names
	char *m_skinModelnames[ NumCustomSkins ];				///< Custom skin modelnames
	char *m_skinFilenames[ NumCustomSkins ];				///< Custom skin filenames
	int m_nextSkin;											///< Next custom skin to allocate
};

/// the global singleton for accessing BotProfiles
extern BotProfileManager *TheBotProfiles;


#endif // _BOT_PROFILE_H_
