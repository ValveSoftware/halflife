//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxPopupMenu.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXPOPUPMENU
#define INCLUDED_MXPOPUPMENU



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif


class mxMenu;
class mxPopupMenu_i;
class mxPopupMenu : public mxWidget
{
	mxPopupMenu_i *d_this;

public:
	// CREATORS
	mxPopupMenu ();
	virtual ~mxPopupMenu ();

	// MANIPULATORS
	int popup (mxWidget *widget, int x, int y);
	void add (const char *item, int id);
	void addMenu (const char *item, mxMenu *menu);
	void addSeparator ();
	void setEnabled (int id, bool b);
	void setChecked (int id, bool b);

	// ACCESSORS
	bool isEnabled (int id) const;
	bool isChecked (int id) const;

private:
	// NOT IMPLEMENTED
	mxPopupMenu (const mxPopupMenu&);
	mxPopupMenu& operator= (const mxPopupMenu&);
};



#endif // INCLUDED_MXPOPUPMENU
