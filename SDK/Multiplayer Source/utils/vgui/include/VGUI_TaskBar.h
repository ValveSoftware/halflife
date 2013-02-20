//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_TASKBAR_H
#define VGUI_TASKBAR_H

#include<VGUI.h>
#include<VGUI_Panel.h>
#include<VGUI_Dar.h>

namespace vgui
{

class Frame;
class Button;

class VGUIAPI TaskBar : public Panel
{
public:
	TaskBar(int x,int y,int wide,int tall);
public:
	virtual void addFrame(Frame* frame);
protected:
	virtual void performLayout();
protected:
	Dar<Frame*>  _frameDar;
	Dar<Button*> _taskButtonDar;
	Panel*       _tray;
};

}

#endif