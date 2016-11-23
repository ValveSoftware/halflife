//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_INPUTSIGNAL_H
#define VGUI_INPUTSIGNAL_H

#include<VGUI.h>

namespace vgui
{

enum MouseCode;
enum KeyCode;
class Panel;

//these are lumped into one for simplicity sake right now
class VGUIAPI InputSignal
{
public:
	virtual void cursorMoved(int x,int y,Panel* panel)=0;
	virtual void cursorEntered(Panel* panel)=0;
	virtual void cursorExited(Panel* panel)=0;
	virtual void mousePressed(MouseCode code,Panel* panel)=0;
	virtual void mouseDoublePressed(MouseCode code,Panel* panel)=0;
	virtual void mouseReleased(MouseCode code,Panel* panel)=0;
	virtual void mouseWheeled(int delta,Panel* panel)=0;
	virtual void keyPressed(KeyCode code,Panel* panel)=0;
	virtual void keyTyped(KeyCode code,Panel* panel)=0;
	virtual void keyReleased(KeyCode code,Panel* panel)=0;
	virtual void keyFocusTicked(Panel* panel)=0;
};

}

#endif