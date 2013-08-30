//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_FRAME_H
#define VGUI_FRAME_H

#include<VGUI.h>
#include<VGUI_Panel.h>
#include<VGUI_Dar.h>

namespace vgui
{

class Button;
class FrameSignal;

class VGUIAPI Frame : public Panel
 {
 public:
  Frame(int x,int y,int wide,int tall);
 public:
  virtual void   setSize(int wide,int tall);
  virtual void   setInternal(bool state);
  virtual void   paintBackground();
  virtual bool   isInternal();
  virtual Panel* getClient();
  virtual void   setTitle(const char* title);
  virtual void   getTitle(char* buf,int bufLen);
  virtual void	 setMoveable(bool state);
  virtual void   setSizeable(bool state);
  virtual bool   isMoveable();
  virtual bool   isSizeable();
  virtual void   addFrameSignal(FrameSignal* s);
  virtual void   setVisible(bool state);
  virtual void   setMenuButtonVisible(bool state);
  virtual void   setTrayButtonVisible(bool state);
  virtual void   setMinimizeButtonVisible(bool state);
  virtual void   setMaximizeButtonVisible(bool state);
  virtual void   setCloseButtonVisible(bool state);
 public: //bullshit public
  virtual void fireClosingSignal();
  virtual void fireMinimizingSignal();
 protected:
  char*             _title;
  bool              _internal;
  bool              _sizeable;
  bool              _moveable;
  Panel*            _topGrip;
  Panel*            _bottomGrip;
  Panel*            _leftGrip;
  Panel*            _rightGrip;
  Panel*            _topLeftGrip;
  Panel*            _topRightGrip;
  Panel*            _bottomLeftGrip;
  Panel*            _bottomRightGrip;
  Panel*            _captionGrip;
  Panel*            _client;
  Button*           _trayButton;
  Button*           _minimizeButton;
  Button*           _maximizeButton;
  Button*           _closeButton;
  Button*           _menuButton;
  Dar<FrameSignal*> _frameSignalDar;
  Frame*            _resizeable;
 };

}

#endif
