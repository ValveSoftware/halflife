//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxPopupMenu.cpp
// implementation: Win32 API
// last modified:  Mar 18 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxPopupMenu.h>
#include <windows.h>
#include <mx/mxMenu.h>
#include <vector>
using namespace std;



class mxPopupMenu_i
{
public:
	vector<mxMenu *> d_menuList;
};



mxPopupMenu::mxPopupMenu ()
: mxWidget (0, 0, 0, 0, 0)
{
	d_this = new mxPopupMenu_i;
	void *handle = (void *) CreatePopupMenu ();
	setHandle (handle);
	setType (MX_POPUPMENU);
}



mxPopupMenu::~mxPopupMenu ()
{
	int nSize = d_this->d_menuList.size();
	for (int i = 0; i < nSize; i++)
	{
		delete d_this->d_menuList[i];
	}
	d_this->d_menuList.clear();

	delete d_this;

	::DestroyMenu((HMENU) getHandle());
}



int
mxPopupMenu::popup (mxWidget *widget, int x, int y)
{
	POINT pt;
	pt.x = x;
	pt.y = y;

	ClientToScreen ((HWND) widget->getHandle (), &pt);
	return (int) TrackPopupMenu ((HMENU) getHandle (), TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, (HWND) widget->getHandle (), NULL);
}



void
mxPopupMenu::add (const char *item, int id)
{
	AppendMenu ((HMENU) getHandle (), MF_STRING, (UINT) id, item);
}



void
mxPopupMenu::addMenu (const char *item, mxMenu *menu)
{
	AppendMenu ((HMENU) getHandle (), MF_POPUP, (UINT) menu->getHandle (), item);
	d_this->d_menuList.push_back(menu);
}



void
mxPopupMenu::addSeparator ()
{
	AppendMenu ((HMENU) getHandle (), MF_SEPARATOR, 0, 0);
}



void
mxPopupMenu::setEnabled (int id, bool b)
{
	EnableMenuItem ((HMENU) getHandle (), (UINT) id, MF_BYCOMMAND | (b ? MF_ENABLED:MF_GRAYED));
}



void
mxPopupMenu::setChecked (int id, bool b)
{
	CheckMenuItem ((HMENU) getHandle (), (UINT) id, MF_BYCOMMAND | (b ? MF_CHECKED:MF_UNCHECKED));
}



bool
mxPopupMenu::isEnabled (int id) const
{
	MENUITEMINFO mii;

	memset (&mii, 0, sizeof (mii));
	mii.cbSize = sizeof (mii);
	mii.fMask = MIIM_STATE;
	GetMenuItemInfo ((HMENU) getHandle (), (UINT) id, false, &mii);
	if (mii.fState & MFS_GRAYED)
		return true;

	return false;
}



bool
mxPopupMenu::isChecked (int id) const
{
	MENUITEMINFO mii;

	memset (&mii, 0, sizeof (mii));
	mii.cbSize = sizeof (mii);
	mii.fMask = MIIM_STATE;
	GetMenuItemInfo ((HMENU) getHandle (), (UINT) id, false, &mii);
	if (mii.fState & MFS_CHECKED)
		return true;

	return false;
}
