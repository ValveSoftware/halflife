//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxMenu.cpp
// implementation: Win32 API
// last modified:  Mar 18 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxMenu.h>
#include <windows.h>
#include <string.h>
//#include <ostream.h>
#include <vector>
using namespace std;



class mxMenu_i
{
public:
	vector<mxMenu *> d_menuList;
};



mxMenu::mxMenu ()
: mxWidget (0, 0, 0, 0, 0)
{
	d_this = new mxMenu_i;
	void *handle = (void *) CreateMenu ();

	setHandle (handle);
	setType (MX_MENU);
}



mxMenu::~mxMenu ()
{
	int nSize = d_this->d_menuList.size();
	for (int i = 0; i < nSize; i++)
	{
		delete d_this->d_menuList[i];
	}
	d_this->d_menuList.clear();

	delete d_this;
}



void
mxMenu::add (const char *item, int id)
{
	AppendMenu ((HMENU) getHandle (), MF_STRING, (UINT) id, item);
}



void
mxMenu::addMenu (const char *item, mxMenu *menu)
{
	AppendMenu ((HMENU) getHandle (), MF_POPUP, (UINT) menu->getHandle (), item);
	d_this->d_menuList.push_back(menu);
}



void
mxMenu::addSeparator ()
{
	AppendMenu ((HMENU) getHandle (), MF_SEPARATOR, 0, 0);
}



void
mxMenu::setEnabled (int id, bool b)
{
	EnableMenuItem ((HMENU) getHandle (), (UINT) id, MF_BYCOMMAND | (b ? MF_ENABLED:MF_GRAYED));
}



void
mxMenu::setChecked (int id, bool b)
{
	CheckMenuItem ((HMENU) getHandle (), (UINT) id, MF_BYCOMMAND | (b ? MF_CHECKED:MF_UNCHECKED));
}



bool
mxMenu::isEnabled (int id) const
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
mxMenu::isChecked (int id) const
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
