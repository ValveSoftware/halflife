//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_FRAMESIGNAL_H
#define VGUI_FRAMESIGNAL_H

#include<VGUI.h>

namespace vgui
{

class Frame;

class VGUIAPI FrameSignal
{
public:
	virtual void closing(Frame* frame)=0;
	virtual void minimizing(Frame* frame,bool toTray)=0;
};

}

#endif