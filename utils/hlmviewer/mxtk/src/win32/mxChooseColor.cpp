//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxChooseColor.cpp
// implementation: Win32 API
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxChooseColor.h>
#include <mx/mxWindow.h>
#include <windows.h>
#include <commdlg.h>



bool
mxChooseColor (mxWindow *parent, int *r, int *g, int *b)
{
	CHOOSECOLOR cc;
	static COLORREF custColors[16];

	BYTE rr = *r;
	BYTE gg = *g;
	BYTE bb = *b;

	memset (&cc, 0, sizeof (CHOOSECOLOR));
	cc.lStructSize = sizeof (CHOOSECOLOR);
	cc.hwndOwner = parent ? (HWND) parent->getHandle ():NULL;
	cc.rgbResult = RGB (rr, gg, bb);
	cc.lpCustColors = custColors;
	cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor (&cc))
	{
		*r = (int) GetRValue (cc.rgbResult);
		*g = (int) GetGValue (cc.rgbResult);
		*b = (int) GetBValue (cc.rgbResult);

		return true;
	}

	return false;
}
