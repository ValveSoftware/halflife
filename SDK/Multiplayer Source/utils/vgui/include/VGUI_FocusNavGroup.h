//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_FOCUSNAVGROUP_H
#define VGUI_FOCUSNAVGROUP_H

#include<VGUI.h>
#include<VGUI_Dar.h>

namespace vgui
{

class Panel;

class VGUIAPI FocusNavGroup
{
public:
	FocusNavGroup();
protected:
	virtual void addPanel(Panel* panel);
	virtual void requestFocusPrev();
	virtual void requestFocusNext();
	virtual void setCurrentPanel(Panel* panel);
protected:
	Dar<Panel*> _panelDar;
	int         _currentIndex;
friend class Panel;
};
}

#endif