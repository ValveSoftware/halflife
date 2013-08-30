//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_MESSAGEBOX_H
#define VGUI_MESSAGEBOX_H

#include<VGUI.h>
#include<VGUI_Frame.h>
#include<VGUI_Dar.h>


namespace vgui
{

class Label;
class Button;
class ActionSignal;

class VGUIAPI MessageBox : public Frame
{

private:
	
	Label*             _messageLabel;
	Button*            _okButton;
	Dar<ActionSignal*> _actionSignalDar;

public:

	MessageBox(const char* title,const char* text,int x,int y);

protected:
	
	virtual void performLayout();

public:
	
	virtual void addActionSignal(ActionSignal* s);
	virtual void fireActionSignal();

};

}





#endif