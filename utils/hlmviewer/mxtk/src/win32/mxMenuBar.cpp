//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxMenuBar.cpp
// implementation: Win32 API
// last modified:  Apr 28 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxMenuBar.h>
#include <mx/mxWindow.h>
#include <mx/mxMenu.h>
#include <windows.h>
#include <string.h>
#include <vector>
using namespace std;



class mxMenuBar_i
{
public:
	vector<mxMenu *> d_menuList;
};



mxMenuBar::mxMenuBar (mxWindow *parent)
: mxWidget (0, 0, 0, 0, 0)
{
	d_this = new mxMenuBar_i;
	void *handle = (void *) CreateMenu ();
	setHandle (handle);
	setType (MX_MENUBAR);
	setParent (parent);

	if (parent)
	{
		mxWidget *w = (mxWidget *) parent;
		SetMenu ((HWND) w->getHandle (), (HMENU) handle);
		parent->addWidget (this);
	}
}



mxMenuBar::~mxMenuBar ()
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



void
mxMenuBar::addMenu (const char *item, mxMenu *menu)
{
	AppendMenu ((HMENU) getHandle (), MF_POPUP, (UINT) ((mxWidget *) menu)->getHandle (), item);
	d_this->d_menuList.push_back(menu);
}



void
mxMenuBar::setEnabled (int id, bool b)
{
	EnableMenuItem ((HMENU) getHandle (), (UINT) id, MF_BYCOMMAND | (b ? MF_ENABLED:MF_GRAYED));
}



void
mxMenuBar::setChecked (int id, bool b)
{
	CheckMenuItem ((HMENU) getHandle (), (UINT) id, MF_BYCOMMAND | (b ? MF_CHECKED:MF_UNCHECKED));
}



void
mxMenuBar::modify (int id, int newId, const char *newItem)
{
	ModifyMenu ((HMENU) getHandle (), (UINT) id, MF_BYCOMMAND | MF_STRING, (UINT) newId, (LPCTSTR) newItem);
}



bool
mxMenuBar::isEnabled (int id) const
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
mxMenuBar::isChecked (int id) const
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



int
mxMenuBar::getHeight () const
{
	return 0;
}
