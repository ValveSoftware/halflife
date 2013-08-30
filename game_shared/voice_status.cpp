
#include <stdio.h>
#include <string.h>

#include "wrect.h"
#include "cl_dll.h"
#include "cl_util.h"
#include "cl_entity.h"
#include "const.h"

#include "parsemsg.h" // BEGIN_READ(), ...

#include "voice_status.h"

#pragma warning( disable : 4800  )  // disable forcing int to bool performance warning


static CVoiceStatus *g_pInternalVoiceStatus = NULL;

// ---------------------------------------------------------------------- //
// The voice manager for the client.
// ---------------------------------------------------------------------- //
CVoiceStatus g_VoiceStatus;

CVoiceStatus* GetClientVoice()
{
	return &g_VoiceStatus;
}




int __MsgFunc_VoiceMask(const char *pszName, int iSize, void *pbuf)
{
	if(g_pInternalVoiceStatus)
		g_pInternalVoiceStatus->HandleVoiceMaskMsg(iSize, pbuf);

	return 1;
}

int __MsgFunc_ReqState(const char *pszName, int iSize, void *pbuf)
{
	if(g_pInternalVoiceStatus)
		g_pInternalVoiceStatus->HandleReqStateMsg(iSize, pbuf);

	return 1;
}




int g_BannedPlayerPrintCount;
void ForEachBannedPlayer(char id[16])
{
	char str[256];
	sprintf(str, "Ban %d: %2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x\n",
		g_BannedPlayerPrintCount++,
		id[0], id[1], id[2], id[3], 
		id[4], id[5], id[6], id[7], 
		id[8], id[9], id[10], id[11], 
		id[12], id[13], id[14], id[15]
		);
#ifdef _WIN32
	strupr(str);
#endif
	gEngfuncs.pfnConsolePrint(str);
}


void ShowBannedCallback()
{
	if(g_pInternalVoiceStatus)
	{
		g_BannedPlayerPrintCount = 0;
		gEngfuncs.pfnConsolePrint("------- BANNED PLAYERS -------\n");
		g_pInternalVoiceStatus->GetBanMgr()->ForEachBannedPlayer(ForEachBannedPlayer);
		gEngfuncs.pfnConsolePrint("------------------------------\n");
	}
}









CVoiceStatus::CVoiceStatus()
{
	m_bBanMgrInitialized = false;
	m_LastUpdateServerState = 0;

	m_bTalking = m_bServerAcked = false;

	m_bServerModEnable = -1;

	m_pchGameDir = NULL;
}


CVoiceStatus::~CVoiceStatus()
{

	g_pInternalVoiceStatus = NULL;

	if(m_pchGameDir)
	{
		if(m_bBanMgrInitialized)
		{
			m_BanMgr.SaveState(m_pchGameDir);
		}

		free(m_pchGameDir);
	}

}



void CVoiceStatus::Init( IVoiceStatusHelper *pHelper)
{
	// Setup the voice_modenable cvar.
	gEngfuncs.pfnRegisterVariable("voice_modenable", "1", FCVAR_ARCHIVE);

	gEngfuncs.pfnRegisterVariable("voice_clientdebug", "0", 0);

	gEngfuncs.pfnAddCommand("voice_showbanned", ShowBannedCallback);

	// Cache the game directory for use when we shut down
	const char *pchGameDirT = gEngfuncs.pfnGetGameDirectory();
	m_pchGameDir = (char *)malloc(strlen(pchGameDirT) + 1);
	
	if(m_pchGameDir)
	{
		strcpy(m_pchGameDir, pchGameDirT);
	}

	if(m_pchGameDir)
	{
		m_BanMgr.Init(m_pchGameDir);
		m_bBanMgrInitialized = true;
	}

	assert(!g_pInternalVoiceStatus);
	g_pInternalVoiceStatus = this;

	m_bInSquelchMode = false;

	m_pHelper = pHelper;

	HOOK_MESSAGE(VoiceMask);
	HOOK_MESSAGE(ReqState);


	GetClientVoiceHud()->Init(pHelper,this);
	


}


void CVoiceStatus::Frame(double frametime)
{
	// check server banned players once per second
	if(gEngfuncs.GetClientTime() - m_LastUpdateServerState > 1)
	{
		UpdateServerState(false);
	}



}

void CVoiceStatus::StartSquelchMode()
{
	if(m_bInSquelchMode)
		return;

	m_bInSquelchMode = true;
}

void CVoiceStatus::StopSquelchMode()
{
	m_bInSquelchMode = false;
}

bool CVoiceStatus::IsInSquelchMode()
{
	return m_bInSquelchMode;
}

void CVoiceStatus::UpdateServerState(bool bForce)
{
	// Can't do anything when we're not in a level.
	char const *pLevelName = gEngfuncs.pfnGetLevelName();
	if( pLevelName[0] == 0 )
	{
		if( gEngfuncs.pfnGetCvarFloat("voice_clientdebug") )
		{
			gEngfuncs.pfnConsolePrint( "CVoiceStatus::UpdateServerState: pLevelName[0]==0\n" );
		}

		return;
	}
	
	int bCVarModEnable = !!gEngfuncs.pfnGetCvarFloat("voice_modenable");
	if(bForce || m_bServerModEnable != bCVarModEnable)
	{
		m_bServerModEnable = bCVarModEnable;

		char str[256];
		_snprintf(str, sizeof(str), "VModEnable %d", m_bServerModEnable);
		ServerCmd(str);

		if(gEngfuncs.pfnGetCvarFloat("voice_clientdebug"))
		{
			char msg[256];
			sprintf(msg, "CVoiceStatus::UpdateServerState: Sending '%s'\n", str);
			gEngfuncs.pfnConsolePrint(msg);
		}
	}

	char str[2048];
	sprintf(str, "vban");
	bool bChange = false;

	for(uint32 dw=0; dw < VOICE_MAX_PLAYERS_DW; dw++)
	{	
		uint32 serverBanMask = 0;
		uint32 banMask = 0;
		for(uint32 i=0; i < 32; i++)
		{
			char playerID[16];
			if(!gEngfuncs.GetPlayerUniqueID(i+1, playerID))
				continue;

			if(m_BanMgr.GetPlayerBan(playerID))
				banMask |= 1 << i;

			if(m_ServerBannedPlayers[dw*32 + i])
				serverBanMask |= 1 << i;
		}

		if(serverBanMask != banMask)
			bChange = true;

		// Ok, the server needs to be updated.
		char numStr[512];
		sprintf(numStr, " %x", banMask);
		strcat(str, numStr);
	}

	if(bChange || bForce)
	{
		if(gEngfuncs.pfnGetCvarFloat("voice_clientdebug"))
		{
			char msg[256];
			sprintf(msg, "CVoiceStatus::UpdateServerState: Sending '%s'\n", str);
			gEngfuncs.pfnConsolePrint(msg);
		}

		gEngfuncs.pfnServerCmdUnreliable(str);	// Tell the server..
	}
	else
	{
		if (gEngfuncs.pfnGetCvarFloat("voice_clientdebug"))
		{
			gEngfuncs.pfnConsolePrint( "CVoiceStatus::UpdateServerState: no change\n" );
		}
	}
	
	m_LastUpdateServerState = gEngfuncs.GetClientTime();
}



int CVoiceStatus::GetSpeakerStatus(int iPlayer)
{
	bool bTalking = static_cast<bool>(m_VoicePlayers[iPlayer]);


	char playerID[16];
	qboolean id = gEngfuncs.GetPlayerUniqueID( iPlayer+1, playerID ); 
	if(!id)
		return VOICE_NEVERSPOKEN;
	
	bool bBanned  = m_BanMgr.GetPlayerBan(  playerID );
	bool bNeverSpoken = !m_VoiceEnabledPlayers[iPlayer];


	if(bBanned)
	{
		return VOICE_BANNED;
	}
	else if(bNeverSpoken)
	{
		return VOICE_NEVERSPOKEN;
	}
	else if(bTalking)
	{
		return VOICE_TALKING;
	}
	else
		return VOICE_NOTTALKING;
	
}



void CVoiceStatus::HandleVoiceMaskMsg(int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	uint32 dw;
	for(dw=0; dw < VOICE_MAX_PLAYERS_DW; dw++)
	{
		m_AudiblePlayers.SetDWord(dw, (uint32)READ_LONG());
		m_ServerBannedPlayers.SetDWord(dw, (uint32)READ_LONG());

		if(gEngfuncs.pfnGetCvarFloat("voice_clientdebug"))
		{
			char str[256];
			gEngfuncs.pfnConsolePrint("CVoiceStatus::HandleVoiceMaskMsg\n");
			
			sprintf(str, "    - m_AudiblePlayers[%d] = %lu\n", dw, m_AudiblePlayers.GetDWord(dw));
			gEngfuncs.pfnConsolePrint(str);
			
			sprintf(str, "    - m_ServerBannedPlayers[%d] = %lu\n", dw, m_ServerBannedPlayers.GetDWord(dw));
			gEngfuncs.pfnConsolePrint(str);
		}
	}

	m_bServerModEnable = READ_BYTE();
}

void CVoiceStatus::HandleReqStateMsg(int iSize, void *pbuf)
{
	if(gEngfuncs.pfnGetCvarFloat("voice_clientdebug"))
	{
		gEngfuncs.pfnConsolePrint("CVoiceStatus::HandleReqStateMsg\n");
	}

	UpdateServerState(true);	
}


void CVoiceStatus::UpdateSpeakerStatus(int entindex, bool bTalking)
{
	
	const char *levelName = gEngfuncs.pfnGetLevelName();
	if (levelName && levelName[0])
	{

		if( gEngfuncs.pfnGetCvarFloat("voice_clientdebug") )
		{
			char msg[256];
			_snprintf( msg, sizeof(msg), "CVoiceStatus::UpdateSpeakerStatus: ent %d talking = %d\n", entindex, bTalking );
			gEngfuncs.pfnConsolePrint( msg );
		}

		// Is it the local player talking?
		if( entindex == -1 )
		{
			m_bTalking = bTalking;
			if( bTalking )
			{
				// Enable voice for them automatically if they try to talk.
				gEngfuncs.pfnClientCmd( "voice_modenable 1" );
			}
			if ( !gEngfuncs.GetLocalPlayer() )
			{
				return;
			}

			int entindex = gEngfuncs.GetLocalPlayer()->index;
			GetClientVoiceHud()->UpdateSpeakerStatus(-2,bTalking);
			
			m_VoicePlayers[entindex-1] = m_bTalking;
			m_VoiceEnabledPlayers[entindex-1]= true;
		}
		else if( entindex == -2 )
		{
			m_bServerAcked = bTalking;
		}
		else if(entindex >= 0 && entindex <= VOICE_MAX_PLAYERS)
		{
			int iClient = entindex - 1;
			if(iClient < 0)
				return;

			GetClientVoiceHud()->UpdateSpeakerStatus(entindex,bTalking);
		
			if(bTalking)
			{
				m_VoicePlayers[iClient] = true;
				m_VoiceEnabledPlayers[iClient] = true;
			}
			else
			{
				m_VoicePlayers[iClient] = false;
			}
		}

		GetClientVoiceHud()->RepositionLabels();
	}
}



//-----------------------------------------------------------------------------
// Purpose: returns true if the target client has been banned
// Input  : playerID - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CVoiceStatus::IsPlayerBlocked(int iPlayer)
{
	char playerID[16];
	if (!gEngfuncs.GetPlayerUniqueID(iPlayer, playerID))
		return false;

	return m_BanMgr.GetPlayerBan(playerID);
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the player can't hear the other client due to game rules (eg. the other team)
// Input  : playerID - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CVoiceStatus::IsPlayerAudible(int iPlayer)
{
	return !!m_AudiblePlayers[iPlayer-1];
}

//-----------------------------------------------------------------------------
// Purpose: blocks/unblocks the target client from being heard
// Input  : playerID - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
void CVoiceStatus::SetPlayerBlockedState(int iPlayer, bool blocked)
{
	if (gEngfuncs.pfnGetCvarFloat("voice_clientdebug"))
	{
		gEngfuncs.pfnConsolePrint( "CVoiceStatus::SetPlayerBlockedState part 1\n" );
	}

	char playerID[16];
	if (!gEngfuncs.GetPlayerUniqueID(iPlayer, playerID))
		return;

	if (gEngfuncs.pfnGetCvarFloat("voice_clientdebug"))
	{
		gEngfuncs.pfnConsolePrint( "CVoiceStatus::SetPlayerBlockedState part 2\n" );
	}

	// Squelch or (try to) unsquelch this player.
	if (gEngfuncs.pfnGetCvarFloat("voice_clientdebug"))
	{
		char str[256];
		sprintf(str, "CVoiceStatus::SetPlayerBlockedState: setting player %d ban to %d\n", iPlayer, !m_BanMgr.GetPlayerBan(playerID));
		gEngfuncs.pfnConsolePrint(str);
	}

	m_BanMgr.SetPlayerBan( playerID, blocked );
	UpdateServerState(false);
}

