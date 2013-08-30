//========= Copyright 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// There are hud.h's coming out of the woodwork so this ensures that we get the right one.
#if defined(THREEWAVE) || defined(DMC_BUILD)
	#include "../dmc/cl_dll/hud.h"
#elif defined(CZERO)
	#include "../czero/cl_dll/hud.h"
#elif defined(CSTRIKE)
	#include "../cstrike/cl_dll/hud.h"
#elif defined(DOD)
	#include "../dod/cl_dll/hud.h"
#elif defined(BLUESHIFT)
	#include "../blueshift/cl_dll/hud.h"
#else
	#include "../cl_dll/hud.h"
#endif

#include "cl_util.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "demo.h"
#include "demo_api.h"
#include "r_efx.h"
#include "entity_types.h"
#include "shared_util.h"

#include "voice_status.h"
#include "voice_status_hud.h"

using namespace vgui;

#include <vgui/IVGui.h>
#include <vgui/IImage.h>

#include <vgui/ILocalize.h>

extern int cam_thirdperson;


#define VOICE_MODEL_INTERVAL		0.3
//#define SCOREBOARD_BLINK_FREQUENCY	0.3	// How often to blink the scoreboard icons.
#define SQUELCHOSCILLATE_PER_SECOND	2.0f



// ---------------------------------------------------------------------- //
// The voice manager for the client.
// ---------------------------------------------------------------------- //
CVoiceStatusHud g_VoiceStatusHud;

IVoiceHud* GetClientVoiceHud()
{
	return &g_VoiceStatusHud;
}



// ---------------------------------------------------------------------- //
// CVoiceLabel.
// ---------------------------------------------------------------------- //
void CVoiceLabel::SetLocation( const char *location )
{
	if ( !location || !*location )
	{
		if ( m_locationString )
		{
			delete[] m_locationString;
			m_locationString = NULL;
			RebuildLabelText();
		}
		return;
	}

	const wchar_t *newLocation = vgui::localize()->Find( location );
	if ( newLocation )
	{
		// localized version
		if ( m_locationString && wcscmp( newLocation, m_locationString ) )
		{
			delete[] m_locationString;
			m_locationString = NULL;
		}

		if ( !m_locationString )
		{
			m_locationString = CloneWString( newLocation );
			RebuildLabelText();
		}
	}
	else
	{
		// just convert the ANSI version to Unicode
		wchar_t *tmpBuf = new wchar_t[ strlen(location) + 1 ];
		localize()->ConvertANSIToUnicode( location, tmpBuf, sizeof(tmpBuf) );

		if ( m_locationString && wcscmp( tmpBuf, m_locationString ) )
		{
			delete[] m_locationString;
			m_locationString = NULL;
		}

		if ( !m_locationString )
		{
			m_locationString = CloneWString( tmpBuf );
			RebuildLabelText();
		}

		delete[] tmpBuf;
	}
}

void CVoiceLabel::SetPlayerName( const char *name )
{
	if ( m_playerName )
	{
		delete[] m_playerName;
		m_playerName = NULL;
	}

	if ( name )
	{
		m_playerName = CloneString( name );
	}

	RebuildLabelText();
}

void CVoiceLabel::RebuildLabelText()
{
	const int BufLen = 512;
	wchar_t buf[BufLen] = L"";
	if ( m_playerName )
	{
		wchar_t wsPlayer[BufLen] = L"";
		
		localize()->ConvertANSIToUnicode(m_playerName, wsPlayer, sizeof(wsPlayer));

		const wchar_t *formatStr = L"%ls   ";
		if ( m_locationString )
		{
			formatStr = localize()->Find( "#Voice_Location" );
			if ( !formatStr )
				formatStr = L"%ls/%ls   ";
		}
		_snwprintf( buf, BufLen, formatStr, wsPlayer, m_locationString );
	}
	m_pLabel->SetText( buf );
	//gEngfuncs.Con_DPrintf( "CVoiceLabel::RebuildLabelText() - [%ls]\n", buf );
}



// ---------------------------------------------------------------------- //
// CVoiceStatus.
// ---------------------------------------------------------------------- //

CVoiceStatusHud::CVoiceStatusHud()
{
}


CVoiceStatusHud::~CVoiceStatusHud()
{
}


int CVoiceStatusHud::Init(IVoiceStatusHelper *pHelper, IVoiceStatus *pStatus)
{
	m_VoiceHeadModel = NULL;
	
	m_pHelper = pHelper;
	m_pStatus = pStatus;

	gHUD.AddHudElem(this);
	m_iFlags = HUD_ACTIVE;

	m_pLocalPlayerTalkIcon = new vgui::ImagePanel( NULL, "LocalPlayerIcon");
	m_pLocalPlayerTalkIcon->SetParent( gViewPortInterface->GetViewPortPanel() );
	m_pLocalPlayerTalkIcon->SetVisible( false );
	m_pLocalPlayerTalkIcon->SetImage( scheme()->GetImage("gfx/vgui/icntlk_pl", false) );

	return 1;
}


int CVoiceStatusHud::VidInit()
{
	// Figure out the voice head model height.
	m_VoiceHeadModelHeight = 45;
	char *pFile = (char *)gEngfuncs.COM_LoadFile("scripts/voicemodel.txt", 5, NULL);
	if(pFile)
	{
		char token[4096];
		gEngfuncs.COM_ParseFile(pFile, token);
		if(token[0] >= '0' && token[0] <= '9')
		{
			m_VoiceHeadModelHeight = (float)atof(token);
		}

		gEngfuncs.COM_FreeFile(pFile);
	}

	m_VoiceHeadModel = gEngfuncs.pfnSPR_Load("sprites/voiceicon.spr");
	return TRUE;
}

void CVoiceStatusHud::CreateEntities()
{
	if(!m_VoiceHeadModel)
		return;

	cl_entity_t *localPlayer = gEngfuncs.GetLocalPlayer();

	int iOutModel = 0;
	for(int i=0; i < VOICE_MAX_PLAYERS; i++)
	{
		if(m_pStatus->GetSpeakerStatus(i)!=CVoiceStatus::VOICE_TALKING)
		{
			continue;
		}

		cl_entity_s *pClient = gEngfuncs.GetEntityByIndex(i+1);
		
		// Don't show an icon if the player is not in our PVS.
		if(!pClient || pClient->curstate.messagenum < localPlayer->curstate.messagenum)
			continue;

		// Don't show an icon for dead or spectating players (ie: invisible entities).
		if(pClient->curstate.effects & EF_NODRAW)
			continue;

		// Don't show an icon for the local player unless we're in thirdperson mode.
		if(pClient == localPlayer && !cam_thirdperson)
			continue;

		cl_entity_s *pEnt = &m_VoiceHeadModels[iOutModel];
		++iOutModel;

		memset(pEnt, 0, sizeof(*pEnt));

		pEnt->curstate.rendermode = kRenderTransAdd;
		pEnt->curstate.renderamt = 255;
		pEnt->baseline.renderamt = 255;
		pEnt->curstate.renderfx = kRenderFxNoDissipation;
		pEnt->curstate.framerate = 1;
		pEnt->curstate.frame = 0;
		pEnt->model = (struct model_s*)gEngfuncs.GetSpritePointer(m_VoiceHeadModel);
		pEnt->angles[0] = pEnt->angles[1] = pEnt->angles[2] = 0;
		pEnt->curstate.scale = 0.5f;
		
		pEnt->origin[0] = pEnt->origin[1] = 0;
		pEnt->origin[2] = 45;

		VectorAdd(pEnt->origin, pClient->origin, pEnt->origin);

		// Tell the engine.
		gEngfuncs.CL_CreateVisibleEntity(ET_NORMAL, pEnt);
	}
}




void CVoiceStatusHud::UpdateLocation(int entindex, const char *location)
{
	int iClient = entindex - 1;
	if(iClient < 0)
		return;

	CVoiceLabel *pLabel = FindVoiceLabel( iClient );
	if ( !pLabel )
		return;

	pLabel->SetLocation( location );

	RepositionLabels();
}

CVoiceLabel* CVoiceStatusHud::FindVoiceLabel(int clientindex)
{
	for(int i=0; i < m_Labels.Count(); i++)
	{
		if(m_Labels[i]->GetClientIndex() == clientindex)
			return m_Labels[i];
	}

	return NULL;
}


CVoiceLabel* CVoiceStatusHud::GetFreeVoiceLabel()
{
	CVoiceLabel *lab =  FindVoiceLabel(-1);
	if( !lab )
	{
		lab = new CVoiceLabel();
		m_Labels.AddToTail( lab );
	}
	return lab;
}


void CVoiceStatusHud::RepositionLabels()
{
	// find starting position to draw from, along right-hand side of screen
	int y = ScreenHeight / 2;
	
	// Reposition active labels.
	for(int i = 0; i < m_Labels.Count(); i++)
	{
		CVoiceLabel *pLabel = m_Labels[i];

		int textWide, textTall;
		pLabel->GetContentSize( textWide, textTall );
		pLabel->SetBounds( ScreenWidth - textWide - 8, y  ); // if you adjust the x pos also play with VoiceVGUILabel in voice_status_hud.h

		y += textTall + 2;
	}
}


void CVoiceStatusHud::UpdateSpeakerStatus(int entindex, bool bTalking)
{
	if( entindex == -2 )  // this is the local player
	{
		if( bTalking )
		{
			int sizeX,sizeY;
			IImage *image = m_pLocalPlayerTalkIcon->GetImage();
			if( image )
			{
				image->GetContentSize( sizeX, sizeY );

				int local_xPos = ScreenWidth - sizeX - 10;
				int local_yPos = ScreenHeight - m_pHelper->GetAckIconHeight() - sizeY;
				m_pLocalPlayerTalkIcon->SetPos( local_xPos, local_yPos );
				m_pLocalPlayerTalkIcon->SetVisible( true );
			}
		}
		else
		{
			m_pLocalPlayerTalkIcon->SetVisible( false );
		}

	}
	else // a remote player, draw a label for them
	{
		if(entindex >= 0 && entindex <= MAX_PLAYERS)
		{
			int iClient = entindex - 1;
			if(iClient < 0)
				return;

			CVoiceLabel *pLabel = FindVoiceLabel(iClient);
			if(bTalking)
			{
				// If we don't have a label for this guy yet, then create one.
				if(!pLabel)
				{
					if(pLabel = GetFreeVoiceLabel())
					{
						// Get the name from the engine.
						hud_player_info_t info;
						memset(&info, 0, sizeof(info));
						gEngfuncs.pfnGetPlayerInfo(entindex, &info);

						int color[3];
						m_pHelper->GetPlayerTextColor( entindex, color );

						pLabel->SetFgColor( Color(255, 255, 255, 255) );
						pLabel->SetBgColor( Color(color[0], color[1], color[2], 180) );
						pLabel->SetPlayerName( info.name );
						pLabel->SetLocation( m_pHelper->GetPlayerLocation( entindex ) );
						pLabel->SetClientIndex( iClient );
						if ( m_pHelper )
						{
							if ( m_pHelper->CanShowSpeakerLabels() )
							{
								pLabel->SetVisible(true);
							}
						}
						else
						{
							pLabel->SetVisible( true );
						}

					}
				}
			}
			else
			{
				// If we have a label for this guy, kill it.
				if(pLabel)
				{
					pLabel->SetVisible(false);
					pLabel->SetClientIndex( -1 );
				}
			}
		}
	}

	RepositionLabels();
}


