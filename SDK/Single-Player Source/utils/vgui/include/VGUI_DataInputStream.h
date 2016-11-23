//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_DATAINPUTSTREAM_H
#define VGUI_DATAINPUTSTREAM_H

#include<VGUI.h>
#include<VGUI_InputStream.h>

namespace vgui
{

class VGUIAPI DataInputStream : virtual public InputStream
{
private:
	InputStream* _is;
public:
	DataInputStream(InputStream* is);
public:
	virtual void  seekStart(bool& success);
	virtual void  seekRelative(int count,bool& success);
	virtual void  seekEnd(bool& success);
	virtual int   getAvailable(bool& success);
	//virtual uchar readUChar(bool& success);
	virtual void  readUChar(uchar* buf,int count,bool& success);
	virtual void  close(bool& success);
	virtual void  close();
public:
	virtual bool   readBool(bool& success);
	virtual char   readChar(bool& success);
	virtual uchar  readUChar(bool& success);
	virtual short  readShort(bool& success);
	virtual ushort readUShort(bool& success);
	virtual int    readInt(bool& success);
	virtual uint   readUInt(bool& success);
	virtual long   readLong(bool& success);
	virtual ulong  readULong(bool& success);
	virtual float  readFloat(bool& success);
	virtual double readDouble(bool& success);
	virtual void   readLine(char* buf,int bufLen,bool& success);
};

}

#endif