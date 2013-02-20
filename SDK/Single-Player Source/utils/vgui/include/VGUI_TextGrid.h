//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_TEXTGRID_H
#define VGUI_TEXTGRID_H

#include<stdarg.h>
#include<VGUI.h>
#include<VGUI_Panel.h>

namespace vgui
{

class VGUIAPI TextGrid : public Panel
{
public:	
	TextGrid(int gridWide,int gridTall,int x,int y,int wide,int tall);
public:
	virtual void setGridSize(int wide,int tall);
	virtual void newLine();
	virtual void setXY(int x,int y);
	//virtual void setBgColor(int r,int g,int b);
	//virtual void setFgColor(int r,int g,int b);
	virtual int  vprintf(const char* format,va_list argList);
	virtual int  printf(const char* format,...);
protected:
	virtual void paintBackground();
protected:
	int   _xy[2];
	int   _bgColor[3];
	int   _fgColor[3];
	char* _grid; //[_gridSize[0]*_gridSize[1]*7] ch,br,bg,bb,fr,fg,fb
	int	  _gridSize[2];
};

}

#endif