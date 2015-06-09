//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxTab.cpp
// implementation: Win32 API
// last modified:  Apr 18 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxTab.h>
#include <mx/mxWindow.h>
#include <windows.h>
#include <commctrl.h>

#include <stdio.h>

class mxTab_i
{
public:
	int dummy;
};



void mxTab_resizeChild (HWND hwnd)
{
	TC_ITEM ti;

	int index = TabCtrl_GetCurSel (hwnd);
	if (index >= 0)
	{
		ti.mask = TCIF_PARAM;
		TabCtrl_GetItem (hwnd, index, &ti);
		mxWidget *widget = (mxWidget *) ti.lParam;
		if (widget)
		{
			RECT rc, rc2;

			GetWindowRect (hwnd, &rc);
			ScreenToClient (GetParent (hwnd), (LPPOINT) &rc.left);
			ScreenToClient (GetParent (hwnd), (LPPOINT) &rc.right);

			TabCtrl_GetItemRect (hwnd, index, &rc2);

			int ex = GetSystemMetrics (SM_CXEDGE);
			int ey = GetSystemMetrics (SM_CYEDGE);
			rc.top += (rc2.bottom - rc2.top) + 3 * ey;
			rc.left += 2 * ex;
			rc.right -= 2 * ex;
			rc.bottom -= 2 * ey;
			HDWP hdwp = BeginDeferWindowPos (2);
			DeferWindowPos (hdwp, hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
			DeferWindowPos (hdwp, (HWND) widget->getHandle (), HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
			EndDeferWindowPos (hdwp);
		}
	}
}



mxTab::mxTab (mxWindow *parent, int x, int y, int w, int h, int id)
: mxWidget (parent, x, y, w, h)
{
	if (!parent)
		return;

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS;
	HWND hwndParent = (HWND) ((mxWidget *) parent)->getHandle ();

	void *handle = (void *) CreateWindowEx (0, WC_TABCONTROL, "", dwStyle,
				x, y, w, h, hwndParent,
				(HMENU) id, (HINSTANCE) GetModuleHandle (NULL), NULL);

	SendMessage ((HWND) handle, WM_SETFONT, (WPARAM) (HFONT) GetStockObject (ANSI_VAR_FONT), MAKELPARAM (TRUE, 0));
	SetWindowLong ((HWND) handle, GWL_USERDATA, (LONG) this);

	setHandle (handle);
	setType (MX_TAB);
	setParent (parent);
	setId (id);

	parent->addWidget (this);
}



mxTab::~mxTab ()
{
	//TabCtrl_DeleteAllItems ((HWND) getHandle ());
}



void
mxTab::add (mxWidget *widget, const char *text)
{
	TC_ITEM ti;

	ti.mask = TCIF_TEXT | TCIF_PARAM;
	ti.pszText = (LPSTR) text;
	ti.lParam = (LPARAM) widget;

	TabCtrl_InsertItem ((HWND) getHandle (), TabCtrl_GetItemCount ((HWND) getHandle ()), &ti);
	mxTab_resizeChild ((HWND) getHandle ());
}



void
mxTab::remove (int index)
{
	TabCtrl_DeleteItem ((HWND) getHandle (), index);
}



void
mxTab::select (int index)
{
	TabCtrl_SetCurSel ((HWND) getHandle (), index);
}



int
mxTab::getSelectedIndex () const
{
	return (int) TabCtrl_GetCurSel ((HWND) getHandle ());
}
