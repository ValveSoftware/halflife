//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_SURFACE_H
#define VGUI_SURFACE_H

#include<VGUI.h>
#include<VGUI_SurfaceBase.h>
#include<VGUI_Dar.h>

namespace vgui
{

class Panel;
class Cursor;

class VGUIAPI Surface : public SurfaceBase
{
public:
	Surface(Panel* embeddedPanel);
public:
	virtual void setTitle(const char* title);
	virtual bool setFullscreenMode(int wide,int tall,int bpp);
	virtual void setWindowedMode();
	virtual void setAsTopMost(bool state);
	virtual int  getModeInfoCount();
	virtual void createPopup(Panel* embeddedPanel);
	virtual bool hasFocus();
	virtual bool isWithin(int x,int y);
protected:
	virtual int  createNewTextureID(void);
	virtual void drawSetColor(int r,int g,int b,int a);
	virtual void drawFilledRect(int x0,int y0,int x1,int y1);
	virtual void drawOutlinedRect(int x0,int y0,int x1,int y1);
	virtual void drawSetTextFont(Font* font);
	virtual void drawSetTextColor(int r,int g,int b,int a);
	virtual void drawSetTextPos(int x,int y);
	virtual void drawPrintText(const char* text,int textLen);
	virtual void drawSetTextureRGBA(int id,const char* rgba,int wide,int tall);
	virtual void drawSetTexture(int id);
	virtual void drawTexturedRect(int x0,int y0,int x1,int y1);
	virtual void invalidate(Panel *panel);
	virtual bool createPlat();
	virtual bool recreateContext();
	virtual void enableMouseCapture(bool state);
	virtual void setCursor(Cursor* cursor);
	virtual void swapBuffers();
	virtual void pushMakeCurrent(Panel* panel,bool useInsets);
	virtual void popMakeCurrent(Panel* panel);
	virtual void applyChanges();
protected:
	class SurfacePlat* _plat;
	bool               _needsSwap;
	Panel*             _embeddedPanel;
	Dar<char*>         _modeInfoDar;
	friend class App;
	friend class Panel;
};

}

#endif

