//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxMessageBox.cpp
// implementation: Win32 API
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxMessageBox.h>
#include <mx/mxWindow.h>
#include <windows.h>
#include <commdlg.h>
#include <string.h>



int
mxMessageBox (mxWindow *parent, const char *msg, const char *title, int style)
{
	HWND hwndParent = 0;
	if (parent)
		hwndParent = (HWND) parent->getHandle ();

	UINT uType = 0;

	if (style & MX_MB_OK)
		uType |= MB_OK;
	else if (style & MX_MB_YESNO)
		uType |= MB_YESNO;
	else if (style & MX_MB_YESNOCANCEL)
		uType |= MB_YESNOCANCEL;

	if (style & MX_MB_INFORMATION)
		uType |= MB_ICONINFORMATION;
	else if (style & MX_MB_ERROR)
		uType |= MB_ICONHAND;
	else if (style & MX_MB_WARNING)
		uType |= MB_ICONEXCLAMATION;
	else if (style & MX_MB_QUESTION)
		uType |= MB_ICONQUESTION;

	int ret = MessageBox (hwndParent, msg, title, uType);

	switch (ret)
	{
	case IDOK:
	case IDYES:
		return 0;

	case IDNO:
		return 1;

	case IDCANCEL:
		return 2;
	}

	return 0;
}
