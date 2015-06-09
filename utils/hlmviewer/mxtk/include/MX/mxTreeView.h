//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxTreeView.h
// implementation: all
// last modified:  Apr 12 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXTREEVIEW
#define INCLUDED_MXTREEVIEW



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif



class mxWindow;



typedef void *mxTreeViewItem;



class mxTreeView_i;
class mxTreeView : public mxWidget
{
	mxTreeView_i *d_this;

public:
	// CREATORS
	mxTreeView (mxWindow *parent, int x, int y, int w, int h, int id = 0);
	virtual ~mxTreeView ();

	// MANIPULATORS
	mxTreeViewItem *add (mxTreeViewItem *parent, const char *item);
	void remove (mxTreeViewItem *item);
	void removeAll ();
	void setLabel (mxTreeViewItem *item, const char *label);
	void setUserData (mxTreeViewItem *item, void *userData);
	void setOpen (mxTreeViewItem *item, bool b);
	void setSelected (mxTreeViewItem *item, bool b);

	// ACCESSORS
	mxTreeViewItem *getFirstChild (mxTreeViewItem *item) const;
	mxTreeViewItem *getNextChild (mxTreeViewItem *item) const;
	mxTreeViewItem *getSelectedItem () const;
	const char *getLabel (mxTreeViewItem *item) const;
	void *getUserData (mxTreeViewItem *item) const;
	bool isOpen (mxTreeViewItem *item) const;
	bool isSelected (mxTreeViewItem *item) const;
	mxTreeViewItem *getParent (mxTreeViewItem *item) const;

private:
	// NOT IMPLEMENTED
	mxTreeView (const mxTreeView&);
	mxTreeView& operator= (const mxTreeView&);
};



#endif // INCLUDED_MXTREEVIEW
