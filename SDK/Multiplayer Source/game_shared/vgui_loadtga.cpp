//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "../cl_dll/wrect.h"
#include "../cl_dll/cl_dll.h"
#include "vgui.h"
#include "vgui_loadtga.h"
#include "vgui_inputstream.h"


// ---------------------------------------------------------------------- //
// Helper class for loading tga files.
// ---------------------------------------------------------------------- //
class MemoryInputStream : public vgui::InputStream
{
public:
				MemoryInputStream()
				{
					m_pData = NULL;
					m_DataLen = m_ReadPos = 0;
				}
		
	virtual void  seekStart(bool& success)					{m_ReadPos=0; success=true;}
	virtual void  seekRelative(int count,bool& success)		{m_ReadPos+=count; success=true;}
	virtual void  seekEnd(bool& success)					{m_ReadPos=m_DataLen; success=true;}
	virtual int   getAvailable(bool& success)				{success=false; return 0;} // This is what vgui does for files...
	
	virtual uchar readUChar(bool& success)					
	{
		if(m_ReadPos>=0 && m_ReadPos<m_DataLen)
		{
			success=true;
			uchar ret = m_pData[m_ReadPos];
			++m_ReadPos;
			return ret;
		}
		else
		{
			success=false;
			return 0;
		}
	}

	virtual void  readUChar(uchar* buf,int count,bool& success)
	{
		for(int i=0; i < count; i++)
			buf[i] = readUChar(success);
	}

	virtual void  close(bool& success) 
	{
		m_pData = NULL;
		m_DataLen = m_ReadPos = 0;
	}

	uchar		*m_pData;
	int			m_DataLen;
	int			m_ReadPos;
};

vgui::BitmapTGA* vgui_LoadTGA(char const *pFilename)
{
	MemoryInputStream stream;
	
	stream.m_pData = gEngfuncs.COM_LoadFile((char*)pFilename, 5, &stream.m_DataLen);
	if(!stream.m_pData)
		return NULL;
	
	stream.m_ReadPos = 0;
	vgui::BitmapTGA *pRet = new vgui::BitmapTGA(&stream, true);
	gEngfuncs.COM_FreeFile(stream.m_pData);
	
	return pRet;
}

vgui::BitmapTGA* vgui_LoadTGANoInvertAlpha(char const *pFilename)
{
	MemoryInputStream stream;
	
	stream.m_pData = gEngfuncs.COM_LoadFile((char*)pFilename, 5, &stream.m_DataLen);
	if(!stream.m_pData)
		return NULL;
	
	stream.m_ReadPos = 0;
	vgui::BitmapTGA *pRet = new vgui::BitmapTGA(&stream, false);
	gEngfuncs.COM_FreeFile(stream.m_pData);
	
	return pRet;
}
