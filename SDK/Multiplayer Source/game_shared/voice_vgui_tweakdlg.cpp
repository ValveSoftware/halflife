//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "../cl_dll/hud.h"
#include "../cl_dll/cl_util.h"
#include "../cl_dll/vgui_teamfortressviewport.h"


#include "vgui_actionsignal.h"
#include "voice_vgui_tweakdlg.h"
#include "voice_vgui_tweakdlg.h"
#include "vgui_panel.h"
#include "vgui_scrollbar.h"
#include "vgui_slider.h"
#include "ivoicetweak.h"
#include "vgui_button.h"
#include "vgui_checkbutton2.h"
#include "vgui_helpers.h"


#define ITEM_BORDER					40	// Border between text and scrollbars on left and right.
#define VOICETWEAK_TRANSPARENCY		150


class TweakScroller
{
public:
						TweakScroller();
	void				Init(Panel *pParent, char *pText, int yPos);

	// Get/set value. Values are 0-1.
	float				GetValue();
	void				SetValue(float val);

public:	
	Label				m_Label;
	ScrollBar			m_Scroll;
	Slider				m_Slider;
};


class CVoiceVGUITweakDlg : public CMenuPanel, public ICheckButton2Handler
{
typedef CMenuPanel BaseClass;

public:
						CVoiceVGUITweakDlg();
						~CVoiceVGUITweakDlg();

// CMenuPanel overrides.
public:
	virtual void		Open();
	virtual void		Close();


// ICheckButton2Handler overrides.
public:

	virtual void		StateChanged(CCheckButton2 *pButton);



// Panel overrides.
public:
	virtual void		paintBackground();


private:

	int					m_DlgWidth;
	int					m_DlgHeight;

	Label				m_Label;		
	
	IVoiceTweak			*m_pVoiceTweak;		// Engine voice tweak API.

	TweakScroller		m_MicVolume;
	TweakScroller		m_SpeakerVolume;

	CCheckButton2		m_VoiceModEnable;
	
	Button				m_Button_OK;
};



bool g_bTweakDlgOpen = false;

bool IsTweakDlgOpen()
{
	return g_bTweakDlgOpen;
}



// ------------------------------------------------------------------------ //
// Global functions.
// ------------------------------------------------------------------------ //

static CVoiceVGUITweakDlg g_VoiceTweakDlg;
CMenuPanel* GetVoiceTweakDlg()
{
	return &g_VoiceTweakDlg;
}


class CVoiceTweakOKButton : public ActionSignal
{
public:
	virtual void	actionPerformed(Panel *pPanel)
	{
		gViewPort->HideVGUIMenu();
	}
};
CVoiceTweakOKButton g_OKButtonSignal;



// ------------------------------------------------------------------------ //
// TweakScroller
// ------------------------------------------------------------------------ //

TweakScroller::TweakScroller() :
	m_Label(""),
	m_Scroll(0,0,0,0,false),
	m_Slider(0,0,10,10,false)
{
}


void TweakScroller::Init(Panel *pParent, char *pText, int yPos)
{
	int parentWidth, parentHeight;
	pParent->getSize(parentWidth, parentHeight);

	// Setup the volume scroll bar.
	m_Label.setParent(pParent);
	m_Label.setFont(Scheme::sf_primary1);
	m_Label.setContentAlignment(vgui::Label::a_northwest);
	m_Label.setBgColor(0, 0, 0, 255);
	m_Label.setFgColor(255,255,255,0);
	m_Label.setPos(ITEM_BORDER, yPos);
	m_Label.setSize(parentWidth/2-ITEM_BORDER, 20);
	m_Label.setText(pText);
	m_Label.setVisible(true);

	m_Slider.setRangeWindow(10);
	m_Slider.setRangeWindowEnabled(true);
	
	m_Scroll.setPos(parentWidth/2+ITEM_BORDER, yPos);
	m_Scroll.setSize(parentWidth/2-ITEM_BORDER*2, 20);
	m_Scroll.setSlider(&m_Slider);
	m_Scroll.setParent(pParent);
	m_Scroll.setRange(0, 100);
	m_Scroll.setFgColor(255,255,255,0);
	m_Scroll.setBgColor(255,255,255,0);
}


float TweakScroller::GetValue()
{
	return m_Scroll.getValue() / 100.0f;
}


void TweakScroller::SetValue(float val)
{
	m_Scroll.setValue((int)(val * 100.0f));
}


// ------------------------------------------------------------------------ //
// CVoiceVGUITweakDlg implementation.
// ------------------------------------------------------------------------ //

CVoiceVGUITweakDlg::CVoiceVGUITweakDlg()
	: CMenuPanel(VOICETWEAK_TRANSPARENCY, false, 0, 0, 0, 0),
	m_Button_OK("",0,0),
	m_Label("")
{
	m_pVoiceTweak = NULL;
	m_Button_OK.addActionSignal(&g_OKButtonSignal);
	m_Label.setBgColor(255,255,255,200);
}


CVoiceVGUITweakDlg::~CVoiceVGUITweakDlg()
{
}


void CVoiceVGUITweakDlg::Open()
{
	if(g_bTweakDlgOpen)
		return;
	
	g_bTweakDlgOpen = true;

	m_DlgWidth = ScreenWidth;
	m_DlgHeight = ScreenHeight;

	m_pVoiceTweak = gEngfuncs.pVoiceTweak;

	// Tell the engine to start voice tweak mode (pipe voice output right to speakers).
	m_pVoiceTweak->StartVoiceTweakMode();

	// Set our size.
	setPos((ScreenWidth - m_DlgWidth) / 2, (ScreenHeight - m_DlgHeight) / 2);
	setSize(m_DlgWidth, m_DlgHeight);

	int curY = ITEM_BORDER;
	m_MicVolume.Init(this, gHUD.m_TextMessage.BufferedLocaliseTextString("#Mic_Volume"), curY);
	m_MicVolume.SetValue(m_pVoiceTweak->GetControlFloat(MicrophoneVolume));
	curY = PanelBottom(&m_MicVolume.m_Label);

	m_SpeakerVolume.Init(this, gHUD.m_TextMessage.BufferedLocaliseTextString("#Speaker_Volume"), curY);
	m_SpeakerVolume.SetValue(m_pVoiceTweak->GetControlFloat(OtherSpeakerScale));
	curY = PanelBottom(&m_SpeakerVolume.m_Label);

	m_VoiceModEnable.setParent(this);
	m_VoiceModEnable.SetImages("gfx/vgui/checked.tga", "gfx/vgui/unchecked.tga");
	m_VoiceModEnable.SetText("Enable Voice In This Mod");
	m_VoiceModEnable.setPos(ITEM_BORDER, curY);
	m_VoiceModEnable.SetCheckboxLeft(false);
	m_VoiceModEnable.SetChecked(!!gEngfuncs.pfnGetCvarFloat("voice_modenable"));
	m_VoiceModEnable.SetHandler(this);

	// Setup the OK button.
	int buttonWidth, buttonHeight;
	m_Button_OK.setText(gHUD.m_TextMessage.BufferedLocaliseTextString("#Menu_OK"));
	m_Button_OK.getSize(buttonWidth, buttonHeight);
	m_Button_OK.setPos((m_DlgWidth - buttonWidth) / 2, m_DlgHeight - buttonHeight - 3);
	m_Button_OK.setParent(this);

	// Put the label on the top.
	m_Label.setBgColor(0, 0, 0, 255);
	m_Label.setFgColor(255,255,255,0);
	m_Label.setText(gHUD.m_TextMessage.BufferedLocaliseTextString("#Voice_Properties"));
	int labelWidth, labelHeight;
	m_Label.getSize(labelWidth, labelHeight);
	m_Label.setPos((m_DlgWidth - labelWidth) / 2, 5);
	m_Label.setParent(this);

	BaseClass::Open();
}


void CVoiceVGUITweakDlg::Close()
{
	m_pVoiceTweak->EndVoiceTweakMode();
	g_bTweakDlgOpen = false;

	BaseClass::Close();
}


void CVoiceVGUITweakDlg::paintBackground()
{
	BaseClass::paintBackground();

	// Draw our border.
	int w,h;
	getSize(w,h);

	drawSetColor(128,128,128,1);
	drawOutlinedRect(0, 0, w, h);

	float volume = m_MicVolume.GetValue();
	m_pVoiceTweak->SetControlFloat(MicrophoneVolume, volume);

	m_pVoiceTweak->SetControlFloat(OtherSpeakerScale, m_SpeakerVolume.GetValue());
}


void CVoiceVGUITweakDlg::StateChanged(CCheckButton2 *pButton)
{
	if(pButton == &m_VoiceModEnable)
	{
		if(pButton->IsChecked())
			gEngfuncs.pfnClientCmd("voice_modenable 1");
		else
			gEngfuncs.pfnClientCmd("voice_modenable 0");
	}
}

