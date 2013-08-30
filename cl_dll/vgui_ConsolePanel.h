//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef CONSOLEPANEL_H
#define CONSOLEPANEL_H

#include<stdarg.h>
#include<VGUI_Panel.h>

namespace vgui
{
class TextGrid;
class TextEntry;
}


class ConsolePanel : public vgui::Panel
{
private:
	vgui::TextGrid*  _textGrid;
	vgui::TextEntry* _textEntry;
public:
	ConsolePanel(int x,int y,int wide,int tall);
public:
	virtual void setSize(int wide,int tall);
	virtual int  print(const char* text);
	virtual int  vprintf(const char* format,va_list argList);
	virtual int  printf(const char* format,...);
	virtual void doExecCommand();
};



#endif