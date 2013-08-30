//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VOICE_LISTBOX_H
#define VOICE_LISTBOX_H
#ifdef _WIN32
#pragma once
#endif


#include "VGUI_Panel.h"
#include "VGUI_IntChangeSignal.h"

#include "vgui_slider2.h"
#include "vgui_scrollbar2.h"


namespace vgui
{

// Listbox class used by voice code. Based off of vgui's list panel but with some modifications:
// - This listbox clips its child items to its rectangle.
// - You can access things like the scrollbar and find out the item width.
// - The scrollbar scrolls one element at a time and the range is correct.

// Note: this listbox does not provide notification when items are 
class CListBox : public Panel
{
public:
	
					CListBox();
					~CListBox();

	void			Init();
	void			Term();

	// Add an item to the listbox. This automatically sets the item's parent to the listbox 
	// and resizes the item's width to fit within the listbox.
	void			AddItem(Panel *pPanel);

	// Get the number of items currently in the listbox.
	int				GetNumItems();

	// Get the width that listbox items will be set to (this changes if you resize the listbox).
	int				GetItemWidth();

	// Get/set the scrollbar position (position says which element is at the top of the listbox).
	int				GetScrollPos();
	void			SetScrollPos(int pos);

	// sets the last item the listbox should scroll to
	// scroll to GetNumItems() if not set
	void			SetScrollRange(int maxScroll);

	// returns the maximum value the scrollbar can scroll to
	int				GetScrollMax();

// vgui overrides.
public:
	
	virtual void	setPos(int x, int y);
	virtual void	setSize(int wide,int tall);
	virtual void	setPixelScroll(int value);
	virtual void	paintBackground();


protected:

	class LBItem
	{
	public:
		Panel	*m_pPanel;
		LBItem	*m_pPrev, *m_pNext;
	};

	class ListBoxSignal : public IntChangeSignal
	{
	public:
		void intChanged(int value,Panel* panel)
		{
			m_pListBox->setPixelScroll(-value);
		}

		vgui::CListBox	*m_pListBox;
	};


protected:
	
	void			InternalLayout();


protected: 

	// All the items..
	LBItem			m_Items;

	Panel			m_ItemsPanel;

	int				m_ItemOffset;	// where we're scrolled to
	Slider2			m_Slider;
	ScrollBar2		m_ScrollBar;
	ListBoxSignal	m_Signal;

	int				m_iScrollMax;
};

}


#endif // VOICE_LISTBOX_H
