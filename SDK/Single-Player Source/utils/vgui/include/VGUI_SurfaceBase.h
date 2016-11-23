//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_SURFACEBASE_H
#define VGUI_SURFACEBASE_H

#include<VGUI.h>
#include<VGUI_Dar.h>

namespace vgui
{

class Panel;
class Cursor;
class Font;
class App;
class ImagePanel;

class VGUIAPI SurfaceBase
{
public:
	SurfaceBase(Panel* embeddedPanel);
protected:
	~SurfaceBase();
public:
	virtual Panel* getPanel();
	virtual void   requestSwap();
	virtual void   resetModeInfo();
	virtual int    getModeInfoCount();
	virtual bool   getModeInfo(int mode,int& wide,int& tall,int& bpp);
	virtual App*   getApp();
	virtual void   setEmulatedCursorVisible(bool state);
	virtual void   setEmulatedCursorPos(int x,int y);
public:
	virtual void setTitle(const char* title)=0;
	virtual bool setFullscreenMode(int wide,int tall,int bpp)=0;
	virtual void setWindowedMode()=0;
	virtual void setAsTopMost(bool state)=0;
	virtual void createPopup(Panel* embeddedPanel)=0;
	virtual bool hasFocus()=0;
	virtual bool isWithin(int x,int y)=0;
	virtual int  createNewTextureID(void)=0;
protected:
	virtual void addModeInfo(int wide,int tall,int bpp);
protected:
	virtual void drawSetColor(int r,int g,int b,int a)=0;
	virtual void drawFilledRect(int x0,int y0,int x1,int y1)=0;
	virtual void drawOutlinedRect(int x0,int y0,int x1,int y1)=0;
	virtual void drawSetTextFont(Font* font)=0;
	virtual void drawSetTextColor(int r,int g,int b,int a)=0;
	virtual void drawSetTextPos(int x,int y)=0;
	virtual void drawPrintText(const char* text,int textLen)=0;
	virtual void drawSetTextureRGBA(int id,const char* rgba,int wide,int tall)=0;
	virtual void drawSetTexture(int id)=0;
	virtual void drawTexturedRect(int x0,int y0,int x1,int y1)=0;
	virtual void invalidate(Panel *panel)=0;
	virtual void enableMouseCapture(bool state)=0;
	virtual void setCursor(Cursor* cursor)=0;
	virtual void swapBuffers()=0;
	virtual void pushMakeCurrent(Panel* panel,bool useInsets)=0;
	virtual void popMakeCurrent(Panel* panel)=0;
	virtual void applyChanges()=0;
protected:
	bool       _needsSwap;
	App*       _app;
	Panel*     _embeddedPanel;
	Dar<char*> _modeInfoDar;
	ImagePanel* _emulatedCursor;
	Cursor*     _currentCursor;
friend class App;
friend class Panel;
};

}

#endif

