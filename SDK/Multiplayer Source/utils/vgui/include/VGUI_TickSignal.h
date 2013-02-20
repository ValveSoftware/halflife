//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_TICKSIGNAL_H
#define VGUI_TICKSIGNAL_H

#include<VGUI.h>

namespace vgui
{
class VGUIAPI TickSignal
 {
 public:
  virtual void ticked()=0;
 };
}

#endif