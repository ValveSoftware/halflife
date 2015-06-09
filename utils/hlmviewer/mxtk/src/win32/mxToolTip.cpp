//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxToolTip.cpp
// implementation: Win32 API
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxToolTip.h>
#include <mx/mxWidget.h>
#include <windows.h>
#include <commctrl.h>



HWND mx_CreateToolTipControl (void);



void
mxToolTip::add (mxWidget *widget, const char *text)
{
	if (!widget)
		return;

	TOOLINFO ti;

	memset (&ti, 0, sizeof (TOOLINFO));
	ti.cbSize = sizeof (TOOLINFO);
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.uId = (UINT) (HWND) widget->getHandle ();
	ti.lpszText = (LPTSTR) text;

	HWND ctrl = mx_CreateToolTipControl ();
	SendMessage (ctrl, TTM_ADDTOOL, 0, (LPARAM) &ti);
}
