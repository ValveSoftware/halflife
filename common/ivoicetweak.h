//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef IVOICETWEAK_H
#define IVOICETWEAK_H
#ifdef _WIN32
#pragma once
#endif

// These provide access to the voice controls.
typedef enum
{
	MicrophoneVolume=0,			// values 0-1.
	OtherSpeakerScale,			// values 0-1. Scales how loud other players are.
	MicBoost,					// 20 db gain to voice input
} VoiceTweakControl;


typedef struct IVoiceTweak_s
{
	// These turn voice tweak mode on and off. While in voice tweak mode, the user's voice is echoed back
	// without sending to the server. 
	int				(*StartVoiceTweakMode)();	// Returns 0 on error.
	void			(*EndVoiceTweakMode)();
	
	// Get/set control values.
	void			(*SetControlFloat)(VoiceTweakControl iControl, float value);
	float			(*GetControlFloat)(VoiceTweakControl iControl);

    int             (*GetSpeakingVolume)();
} IVoiceTweak;


#endif // IVOICETWEAK_H
