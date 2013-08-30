//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VOICE_STATUS_HUD_H
#define VOICE_STATUS_HUD_H
#pragma once

#include <UtlVector.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <color.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Controls.h>

#ifdef CZERO
#include "vgui/hl_base/iviewport.h"
#else
#include <game_controls/iviewport.h>
#endif

#include "voice_common.h"
#include "cl_entity.h"
#include "voice_banmgr.h"


//-----------------------------------------------------------------------------
// Purpose: a label which displays a name and a speaking icon
//-----------------------------------------------------------------------------
class CVoiceLabel
{
public:
	CVoiceLabel() 
	{
		m_pLabel = new VoiceVGUILabel( NULL, "VoiceLabel", "");
		m_pLabel->SetParent( gViewPortInterface->GetViewPortPanel() );
		m_pLabel->SetProportional(true);
		m_pLabel->SetScheme("ClientScheme");
		vgui::SETUP_PANEL(m_pLabel);
		m_clientindex = -1; // -1 means unassigned
		m_locationString = NULL;
		m_playerName = NULL;
	}

	~CVoiceLabel()
	{
		m_pLabel->MarkForDeletion();
	}

	// pass throughs for various label calls
	void SetFgColor( Color c ) { m_pLabel->SetFgColor( c ); }
	void SetBgColor( Color c ) { m_pLabel->SetBgColor( c ); }
	void SetVisible( bool state ) { m_pLabel->SetVisible( state ); }
	bool GetVisible() { return m_pLabel->IsVisible(); }

	void GetContentSize( int &wide, int &tall ) { m_pLabel->GetContentSize( wide, tall ); if( tall < 32 ) tall =32; }
	void SetBounds( int x, int y ) { 
		m_pLabel->SetPos( x, y );
		int wide, tall;
		m_pLabel->GetContentSize( wide, tall );
		m_pLabel->SetSize( wide, tall );
	}

	void SetClientIndex( int in ) { m_clientindex = in; }
	int  GetClientIndex() { return m_clientindex; }

	void SetLocation( const char *location );
	void SetPlayerName( const char *name );

private:

	//-----------------------------------------------------------------------------
	// Purpose: inner class that overrides ApplySchemeSettings() for the label so an image can be loaded, also saves colors away
	//   so ApplySchemeSettings() doesn't override them
	//-----------------------------------------------------------------------------
	class VoiceVGUILabel : public vgui::Label
	{
	public:
		VoiceVGUILabel( vgui::Panel *parent, const char *name, const char *text): Label(parent, name, text) {}


	private:
		// VGUI2 overrides
		virtual void ApplySchemeSettings(vgui::IScheme *pScheme)
		{
			Label::ApplySchemeSettings(pScheme);
			SetTextImageIndex(1);
			SetImagePreOffset( 1, 2); // shift the text over a little
			// you need to load the image here, after Label::ApplySchemeSettings()(as applysettings nulls out all existing images)
			SetImageAtIndex( 0, vgui::scheme()->GetImage( "gfx/vgui/speaker4", false), 1 );
		}

	};

	void RebuildLabelText();

	VoiceVGUILabel		*m_pLabel; // the label with the user name and icon
	int					m_clientindex;	// Client index of the speaker. -1 if this label isn't being used.
	wchar_t				*m_locationString; // localized location string.  NULL if the location is "".
	char				*m_playerName;
};



//-----------------------------------------------------------------------------
// Purpose: Handles the displaying of labels on the hud and icons above players in game when they talk
//-----------------------------------------------------------------------------
class CVoiceStatusHud : public IVoiceHud, public CHudBase 
{
public:
				CVoiceStatusHud();
	virtual		~CVoiceStatusHud();

	// CHudBase overrides.	
	// Initialize the cl_dll's voice manager.
	virtual int Init(IVoiceStatusHelper *pHelper, IVoiceStatus *pStatus);
	
	// ackPosition is the bottom position of where CVoiceStatus will draw the voice acknowledgement labels.
	virtual int VidInit();

	// Call from the HUD_CreateEntities function so it can add sprites above player heads.
	void	CreateEntities();

	void UpdateLocation(int entindex, const char *location);

	void UpdateSpeakerStatus(int entindex, bool bTalking);


	CVoiceLabel*	FindVoiceLabel(int clientindex);	// Find a CVoiceLabel representing the specified speaker. 
														// Returns NULL if none.
														// entindex can be -1 if you want a currently-unused voice label.
	CVoiceLabel*	GetFreeVoiceLabel();				// Get an unused voice label. Returns NULL if none.
	void			RepositionLabels();


private:

	cl_entity_s		m_VoiceHeadModels[VOICE_MAX_PLAYERS];			// These aren't necessarily in the order of players. They are just
																	// a place for it to put data in during CreateEntities.
	HSPRITE				m_VoiceHeadModel;		// Voice head model (goes above players who are speaking).
	float				m_VoiceHeadModelHeight;	// Height above their head to place the model.

	IVoiceStatusHelper *m_pHelper;
	IVoiceStatus *m_pStatus;

	// Labels telling who is speaking.
	CUtlVector<CVoiceLabel *> m_Labels;

	vgui::ImagePanel *m_pLocalPlayerTalkIcon;
};



#endif // VOICE_STATUS_HUD_H
