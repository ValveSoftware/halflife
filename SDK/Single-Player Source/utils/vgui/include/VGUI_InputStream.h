//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_FILE_H
#define VGUI_FILE_H

#include<VGUI.h>

namespace vgui
{

class VGUIAPI InputStream
{
public:
	virtual void  seekStart(bool& success)=0;
	virtual void  seekRelative(int count,bool& success)=0;
	virtual void  seekEnd(bool& success)=0;
	virtual int   getAvailable(bool& success)=0;
	virtual uchar readUChar(bool& success)=0;
	virtual void  readUChar(uchar* buf,int count,bool& success)=0;
	virtual void  close(bool& success)=0;
};

}

#endif