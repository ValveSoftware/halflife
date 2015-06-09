//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxAccelerator.cpp
// implementation: Win32 API
// last modified:  Jun 13 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxAccelerator.h>
#include <mx/mxWidget.h>
#include <windows.h>
#include <commctrl.h>



static ACCEL g_AccelTable[50] = { 0 };
HACCEL g_hAccel= 0;
static int g_numAccel = 0;



void
mxAccelerator::add (int key, int flags, int cmd)
{
	if (g_numAccel < 50)
	{
		g_AccelTable[g_numAccel].fVirt = flags;
		g_AccelTable[g_numAccel].key = key;
		g_AccelTable[g_numAccel].cmd = cmd;
	}
	g_numAccel++;
}



void
mxAccelerator::loadTable ()
{
	g_hAccel = CreateAcceleratorTable (g_AccelTable, g_numAccel);
}



void
mxAccelerator::removeAll ()
{
	g_numAccel = 0;
	if (g_hAccel)
	{
		DestroyAcceleratorTable (g_hAccel);
	}
	g_hAccel = 0;
}
