//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_SLIDER2_H
#define VGUI_SLIDER2_H
#ifdef _WIN32
#pragma once
#endif

#include<VGUI.h>
#include<VGUI_Panel.h>
#include<VGUI_Dar.h>

namespace vgui
{

enum MouseCode;
class IntChangeSignal;

class VGUIAPI Slider2 : public Panel
{
private:
	bool                  _vertical;
	bool                  _dragging;
	int                   _nobPos[2];
	int                   _nobDragStartPos[2];
	int                   _dragStartPos[2];
	Dar<IntChangeSignal*> _intChangeSignalDar;
	int                   _range[2];
	int                   _value;
	int                   _rangeWindow;
	bool                  _rangeWindowEnabled;
	int                   _buttonOffset;
public:
	Slider2(int x,int y,int wide,int tall,bool vertical);
public:
	virtual void setValue(int value);
	virtual int  getValue();
	virtual bool isVertical();
	virtual void addIntChangeSignal(IntChangeSignal* s);
    virtual void setRange(int min,int max);
	virtual void getRange(int& min,int& max);
	virtual void setRangeWindow(int rangeWindow);
	virtual void setRangeWindowEnabled(bool state);
	virtual void setSize(int wide,int tall);
	virtual void getNobPos(int& min, int& max);
	virtual bool hasFullRange();
	virtual void setButtonOffset(int buttonOffset);
private:
	virtual void recomputeNobPosFromValue();
	virtual void recomputeValueFromNobPos();
public: //bullshit public
	virtual void privateCursorMoved(int x,int y,Panel* panel);
	virtual void privateMousePressed(MouseCode code,Panel* panel);
	virtual void privateMouseReleased(MouseCode code,Panel* panel);
protected:
    virtual void fireIntChangeSignal();
	virtual void paintBackground();
};

}

#endif // VGUI_SLIDER2_H
