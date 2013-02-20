//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_APP_H
#define VGUI_APP_H

#include<VGUI.h>
#include<VGUI_MouseCode.h>
#include<VGUI_KeyCode.h>
#include<VGUI_Dar.h>
#include<VGUI_Cursor.h>

namespace vgui
{

enum MouseCode;
enum KeyCode;
class Panel;
class TickSignal;
class Scheme;
class TickSignal;
class SurfaceBase;

class VGUIAPI App
{
public:
	App();
	App(bool externalMain);
public:
	static App* getInstance();
	//TODO: the public and public bullshit are all messed up, need to organize
	//TODO: actually all of the access needs to be properly thought out while you are at it
public:
	virtual void    start();
	virtual void    stop();
	virtual void    externalTick();
	virtual bool    wasMousePressed(MouseCode code,Panel* panel);
	virtual bool    wasMouseDoublePressed(MouseCode code,Panel* panel);
	virtual bool    isMouseDown(MouseCode code,Panel* panel);
	virtual bool    wasMouseReleased(MouseCode code,Panel* panel);
	virtual bool    wasKeyPressed(KeyCode code,Panel* panel);
	virtual bool    isKeyDown(KeyCode code,Panel* panel);
	virtual bool    wasKeyTyped(KeyCode code,Panel* panel);
	virtual bool    wasKeyReleased(KeyCode code,Panel* panel);
	virtual void    addTickSignal(TickSignal* s);
	virtual void    setCursorPos(int x,int y);
	virtual void    getCursorPos(int& x,int& y);
	virtual void    setMouseCapture(Panel* panel);
	virtual void    setMouseArena(int x0,int y0,int x1,int y1,bool enabled);
	virtual void    setMouseArena(Panel* panel);
	virtual void    requestFocus(Panel* panel); 
	virtual Panel*  getFocus();
	virtual void    repaintAll();
	virtual void    setScheme(Scheme* scheme);
	virtual Scheme* getScheme();
	virtual void    enableBuildMode();
	virtual long    getTimeMillis();
	virtual char    getKeyCodeChar(KeyCode code,bool shifted);
	virtual void    getKeyCodeText(KeyCode code,char* buf,int buflen);
	virtual int     getClipboardTextCount();
	virtual void    setClipboardText(const char* text,int textLen);
	virtual int     getClipboardText(int offset,char* buf,int bufLen);
	virtual void    reset();
	virtual void    internalSetMouseArena(int x0,int y0,int x1,int y1,bool enabled);
	virtual bool    setRegistryString(const char* key,const char* value);
	virtual bool    getRegistryString(const char* key,char* value,int valueLen);
	virtual bool    setRegistryInteger(const char* key,int value);
	virtual bool    getRegistryInteger(const char* key,int& value);
	virtual void    setCursorOveride(Cursor* cursor);
	virtual Cursor* getCursorOveride();
	virtual void    setMinimumTickMillisInterval(int interval);
public: //bullshit public stuff
	virtual void main(int argc,char* argv[])=0;
	virtual void run(); 
	virtual void internalCursorMoved(int x,int y,SurfaceBase* surfaceBase); //expects input in surface space
	virtual void internalMousePressed(MouseCode code,SurfaceBase* surfaceBase);
	virtual void internalMouseDoublePressed(MouseCode code,SurfaceBase* surfaceBase);
	virtual void internalMouseReleased(MouseCode code,SurfaceBase* surfaceBase);
	virtual void internalMouseWheeled(int delta,SurfaceBase* surfaceBase);
	virtual void internalKeyPressed(KeyCode code,SurfaceBase* surfaceBase);
	virtual void internalKeyTyped(KeyCode code,SurfaceBase* surfaceBase);
	virtual void internalKeyReleased(KeyCode code,SurfaceBase* surfaceBase);
private:
	virtual void init();
	virtual void updateMouseFocus(int x,int y,SurfaceBase* surfaceBase);
	virtual void setMouseFocus(Panel* newMouseFocus);
protected: 
	virtual void surfaceBaseCreated(SurfaceBase* surfaceBase);
	virtual void surfaceBaseDeleted(SurfaceBase* surfaceBase);
	virtual void platTick();
	virtual void internalTick();
protected:
	static App* _instance;
protected:
	bool              _running;
	bool              _externalMain;
	Dar<SurfaceBase*> _surfaceBaseDar;
	Panel*            _keyFocus;
	Panel*            _oldMouseFocus;
	Panel*            _mouseFocus;
	Panel*            _mouseCapture;
	Panel*            _wantedKeyFocus;
	bool              _mousePressed[MOUSE_LAST];
	bool              _mouseDoublePressed[MOUSE_LAST];
	bool              _mouseDown[MOUSE_LAST];
	bool              _mouseReleased[MOUSE_LAST];
	bool              _keyPressed[KEY_LAST];
	bool              _keyTyped[KEY_LAST];
	bool              _keyDown[KEY_LAST];
	bool              _keyReleased[KEY_LAST];
	Dar<TickSignal*>  _tickSignalDar;
	Scheme*           _scheme;
	bool              _buildMode;
	bool              _wantedBuildMode;
	Panel*            _mouseArenaPanel;
	Cursor*           _cursor[Cursor::DefaultCursor::dc_last];
	Cursor*           _cursorOveride;
private:
	long              _nextTickMillis;
	long              _minimumTickMillisInterval;
	friend class SurfaceBase;
};
}

#endif



