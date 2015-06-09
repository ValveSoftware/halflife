//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxTreeView.cpp
// implementation: Win32 API
// last modified:  May 03 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxTreeView.h>
#include <mx/mxWindow.h>
#include <windows.h>
#include <commctrl.h>



class mxTreeView_i
{
public:
	HWND d_hwnd;
};



mxTreeView::mxTreeView (mxWindow *parent, int x, int y, int w, int h, int id)
: mxWidget (parent, x, y, w, h)
{
	if (!parent)
		return;

	d_this = new mxTreeView_i;

	DWORD dwStyle = TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	HWND hwndParent = (HWND) ((mxWidget *) parent)->getHandle ();

	d_this->d_hwnd = CreateWindowEx (WS_EX_CLIENTEDGE, WC_TREEVIEW, "", dwStyle,
				x, y, w, h, hwndParent,
				(HMENU) id, (HINSTANCE) GetModuleHandle (NULL), NULL);
	
	SendMessage (d_this->d_hwnd, WM_SETFONT, (WPARAM) (HFONT) GetStockObject (ANSI_VAR_FONT), MAKELPARAM (TRUE, 0));
	SetWindowLong (d_this->d_hwnd, GWL_USERDATA, (LONG) this);

	setHandle ((void *) d_this->d_hwnd);
	setType (MX_TREEVIEW);
	setParent (parent);
	setId (id);

	parent->addWidget (this);
}



mxTreeView::~mxTreeView ()
{
	remove (0);
	delete d_this;
}



mxTreeViewItem*
mxTreeView::add (mxTreeViewItem *parent, const char *item)
{
	if (!d_this)
		return 0;

	TV_ITEM tvItem;
	tvItem.mask = TVIF_TEXT;
	tvItem.pszText = (LPSTR) item;
	tvItem.cchTextMax = 256;

	HTREEITEM hParent;
	if (!parent)
		hParent = TVI_ROOT;
	else
		hParent = (HTREEITEM) parent;

	TV_INSERTSTRUCT tvInsert;
	tvInsert.hParent = hParent;
	tvInsert.hInsertAfter = TVI_LAST;
	tvInsert.item = tvItem;

	return (mxTreeViewItem *) TreeView_InsertItem (d_this->d_hwnd, &tvInsert);
}


void
mxTreeView::remove (mxTreeViewItem *item)
{
	if (!d_this)
		return;

	if (!item)
		TreeView_DeleteAllItems (d_this->d_hwnd);
	else
		TreeView_DeleteItem (d_this->d_hwnd, (HTREEITEM) item);
}



void
mxTreeView::setLabel (mxTreeViewItem *item, const char *label)
{
	if (!d_this)
		return;

	if (item)
	{
		TV_ITEM tvItem;
		tvItem.mask = TVIF_HANDLE | TVIF_TEXT;
		tvItem.hItem = (HTREEITEM) item;
		tvItem.pszText = (LPSTR) label;
		tvItem.cchTextMax = 256;

		TreeView_SetItem (d_this->d_hwnd, &tvItem);
	}
}



void
mxTreeView::setUserData (mxTreeViewItem *item, void *userData)
{
	if (!d_this)
		return;

	if (item)
	{
		TV_ITEM tvItem;
		tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
		tvItem.hItem = (HTREEITEM) item;
		tvItem.lParam = (LPARAM) userData;

		TreeView_SetItem (d_this->d_hwnd, &tvItem);
	}
}



void
mxTreeView::setOpen (mxTreeViewItem *item, bool b)
{
	if (!d_this)
		return;

	if (item)
		TreeView_Expand (d_this->d_hwnd, (HTREEITEM) item, b ? TVE_EXPAND:TVE_COLLAPSE);
}



void
mxTreeView::setSelected (mxTreeViewItem *item, bool b)
{
	if (!d_this)
		return;

	if (item)
		TreeView_SelectItem (d_this->d_hwnd, (HTREEITEM) item);
}



mxTreeViewItem*
mxTreeView::getFirstChild (mxTreeViewItem *item) const
{
	if (!d_this)
		return 0;

	return (mxTreeViewItem *) TreeView_GetChild (d_this->d_hwnd, item ? (HTREEITEM) item:TVI_ROOT);
}



mxTreeViewItem*
mxTreeView::getNextChild (mxTreeViewItem *item) const
{
	if (!d_this)
		return 0;

	if (item)
		return (mxTreeViewItem *) TreeView_GetNextSibling (d_this->d_hwnd, (HTREEITEM) item);
	else
		return 0;
}



mxTreeViewItem*
mxTreeView::getSelectedItem () const
{
	if (!d_this)
		return 0;

	return (mxTreeViewItem *) TreeView_GetSelection (d_this->d_hwnd);
}



const char*
mxTreeView::getLabel (mxTreeViewItem *item) const
{
	static char label[256];
	strcpy (label, "");

	if (!d_this)
		return label;

	if (item)
	{
		TV_ITEM tvItem;
		tvItem.mask = TVIF_HANDLE | TVIF_TEXT;
		tvItem.hItem = (HTREEITEM) item;
		tvItem.pszText = (LPSTR) label;
		tvItem.cchTextMax = 256;

		TreeView_GetItem (d_this->d_hwnd, &tvItem);

		return tvItem.pszText;
	}

	return label;
}



void*
mxTreeView::getUserData (mxTreeViewItem *item) const
{
	if (!d_this)
		return 0;

	if (item)
	{
		TV_ITEM tvItem;
		tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
		tvItem.hItem = (HTREEITEM) item;

		TreeView_GetItem (d_this->d_hwnd, &tvItem);

		return (void *) tvItem.lParam;
	}

	return 0;
}



bool
mxTreeView::isOpen (mxTreeViewItem *item) const
{
	if (!d_this)
		return false;

	if (item)
	{
		TV_ITEM tvItem;
		tvItem.mask = TVIF_HANDLE | TVIF_STATE;
		tvItem.hItem = (HTREEITEM) item;
		tvItem.stateMask = TVIS_STATEIMAGEMASK;

		TreeView_GetItem (d_this->d_hwnd, &tvItem);

		return ((tvItem.state & TVIS_EXPANDED) == TVIS_EXPANDED);
	}

	return false;
}



bool
mxTreeView::isSelected (mxTreeViewItem *item) const
{
	if (!d_this)
		return false;

	if (item)
	{
		TV_ITEM tvItem;
		tvItem.mask = TVIF_HANDLE | TVIF_STATE;
		tvItem.hItem = (HTREEITEM) item;
		tvItem.stateMask = TVIS_STATEIMAGEMASK;

		TreeView_GetItem (d_this->d_hwnd, &tvItem);

		return ((tvItem.state & TVIS_SELECTED) == TVIS_SELECTED);
	}

	return false;
}



mxTreeViewItem *
mxTreeView::getParent (mxTreeViewItem *item) const
{
	if (!d_this)
		return 0;

	if (item)
		return (mxTreeViewItem *) TreeView_GetParent (d_this->d_hwnd, (HTREEITEM) item);

	return 0;
}
