//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_CHANGESIGNAL_H
#define VGUI_CHANGESIGNAL_H

#include<VGUI.h>

namespace vgui
{

class Panel;

class VGUIAPI ChangeSignal
{
public:
	virtual void valueChanged(Panel* panel)=0;
};

}

#endif