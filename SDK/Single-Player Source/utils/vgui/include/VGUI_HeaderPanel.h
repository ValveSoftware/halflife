//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_HEADERPANEL_H
#define VGUI_HEADERPANEL_H

#include<VGUI.h>
#include<VGUI_Panel.h>
#include<VGUI_Dar.h>
#include<VGUI_Cursor.h>

namespace vgui
{

enum MouseCode;
class ChangeSignal;

class VGUIAPI HeaderPanel : public Panel
{

private:

	Dar<Panel*>        _sliderPanelDar;
	Dar<Panel*>        _sectionPanelDar;
	Dar<ChangeSignal*> _changeSignalDar;
	Panel*             _sectionLayer;
	int                _sliderWide;
	bool               _dragging;
	int                _dragSliderIndex;
	int                _dragSliderStartPos;
	int                _dragSliderStartX;

public:

	HeaderPanel(int x,int y,int wide,int tall);

protected:

	virtual void performLayout();

public:

	virtual void addSectionPanel(Panel* panel);
	virtual void setSliderPos(int sliderIndex,int pos);
	virtual int  getSectionCount();
	virtual void getSectionExtents(int sectionIndex,int& x0,int& x1);
	virtual void addChangeSignal(ChangeSignal* s);

public: //bullshit public

	virtual void fireChangeSignal();
	virtual void privateCursorMoved(int x,int y,Panel* panel);
	virtual void privateMousePressed(MouseCode code,Panel* panel);
	virtual void privateMouseReleased(MouseCode code,Panel* panel);

};

}

#endif

