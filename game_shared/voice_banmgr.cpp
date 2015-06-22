//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "../public/vstdlib/warnings.h"

#include <string.h>
#include <stdio.h>
#include "voice_banmgr.h"

#include "../public/vstdlib/vstdlib.h"


#define BANMGR_FILEVERSION	1
char const *g_pBanMgrFilename = "voice_ban.dt";



// Hash a player ID to a byte.
unsigned char HashPlayerID(char const playerID[16])
{
	unsigned char curHash = 0;

	for(int i=0; i < 16; i++)
		curHash += (unsigned char)playerID[i];

	return curHash;
}



CVoiceBanMgr::CVoiceBanMgr()
{
	Clear();
}


CVoiceBanMgr::~CVoiceBanMgr()
{
	Term();
}


bool CVoiceBanMgr::Init(char const *pGameDir)
{
	Term();

	char filename[512];
	Q_snprintf(filename, sizeof(filename), "%s/%s", pGameDir, g_pBanMgrFilename);

	// Load in the squelch file.
	FILE *fp = Q_fopen(filename, "rb");
	if(fp)
	{
		int version;
		Q_fread(&version, 1, sizeof(version), fp);
		if(version == BANMGR_FILEVERSION)
		{
			Q_fseek(fp, 0, SEEK_END);
			int nIDs = (Q_ftell(fp) - sizeof(version)) / 16;
			Q_fseek(fp, sizeof(version), SEEK_SET);

			for(int i=0; i < nIDs; i++)
			{
				char playerID[16];
				Q_fread(playerID, 1, 16, fp);
				AddBannedPlayer(playerID);
			}			
		}

		Q_fclose(fp);
	}

	return true;
}


void CVoiceBanMgr::Term()
{
	// Free all the player structures.
	for(int i=0; i < 256; i++)
	{
		BannedPlayer *pListHead = &m_PlayerHash[i];
		BannedPlayer *pNext;
		for(BannedPlayer *pCur=pListHead->m_pNext; pCur != pListHead; pCur=pNext)
		{
			pNext = pCur->m_pNext;
			delete pCur;
		}
	}

	Clear();
}


void CVoiceBanMgr::SaveState(char const *pGameDir)
{
	// Save the file out.
	char filename[512];
	Q_snprintf(filename, sizeof(filename), "%s/%s", pGameDir, g_pBanMgrFilename);

	FILE *fp = Q_fopen(filename, "wb");
	if(fp)
	{
		int version = BANMGR_FILEVERSION;
		Q_fwrite(&version, 1, sizeof(version), fp);

		for(int i=0; i < 256; i++)
		{
			BannedPlayer *pListHead = &m_PlayerHash[i];
			for(BannedPlayer *pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
			{
				Q_fwrite(pCur->m_PlayerID, 1, 16, fp);
			}
		}

		Q_fclose(fp);
	}
}


bool CVoiceBanMgr::GetPlayerBan(char const playerID[16])
{
	return !!InternalFindPlayerSquelch(playerID);
}


void CVoiceBanMgr::SetPlayerBan(char const playerID[16], bool bSquelch)
{
	if(bSquelch)
	{
		// Is this guy already squelched?
		if(GetPlayerBan(playerID))
			return;
	
		AddBannedPlayer(playerID);
	}
	else
	{
		BannedPlayer *pPlayer = InternalFindPlayerSquelch(playerID);
		if(pPlayer)
		{
			pPlayer->m_pPrev->m_pNext = pPlayer->m_pNext;
			pPlayer->m_pNext->m_pPrev = pPlayer->m_pPrev;
			delete pPlayer;
		}
	}
}


void CVoiceBanMgr::ForEachBannedPlayer(void (*callback)(char id[16]))
{
	for(int i=0; i < 256; i++)
	{
		for(BannedPlayer *pCur=m_PlayerHash[i].m_pNext; pCur != &m_PlayerHash[i]; pCur=pCur->m_pNext)
		{
			callback(pCur->m_PlayerID);
		}
	}
}


void CVoiceBanMgr::Clear()
{
	// Tie off the hash table entries.
	for(int i=0; i < 256; i++)
		m_PlayerHash[i].m_pNext = m_PlayerHash[i].m_pPrev = &m_PlayerHash[i];
}


CVoiceBanMgr::BannedPlayer* CVoiceBanMgr::InternalFindPlayerSquelch(char const playerID[16])
{
	int index = HashPlayerID(playerID);
	BannedPlayer *pListHead = &m_PlayerHash[index];
	for(BannedPlayer *pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
	{
		if(Q_memcmp(playerID, pCur->m_PlayerID, 16) == 0)
			return pCur;
	}

	return NULL;
}


CVoiceBanMgr::BannedPlayer* CVoiceBanMgr::AddBannedPlayer(char const playerID[16])
{
	BannedPlayer *pNew = new BannedPlayer;
	if(!pNew)
		return NULL;

	int index = HashPlayerID(playerID);
	Q_memcpy(pNew->m_PlayerID, playerID, 16);
	pNew->m_pNext = &m_PlayerHash[index];
	pNew->m_pPrev = m_PlayerHash[index].m_pPrev;
	pNew->m_pPrev->m_pNext = pNew->m_pNext->m_pPrev = pNew;
	return pNew;
}

