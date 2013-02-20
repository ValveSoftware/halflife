//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "vgui_listbox.h"



using namespace vgui;


CListBox::CListBox() : Panel(0, 0, 0, 0),
	m_ItemsPanel(0,0,0,0),
	m_ScrollBar(0, 0, 0, 0, true),
	m_Slider(0, 0, 10, 40, true)
{
	m_Signal.m_pListBox = this;

	m_ItemsPanel.setParent(this);
	m_ItemsPanel.setBgColor(0,0,0,255);
 
	m_Slider.setRangeWindow(50);
	m_Slider.setRangeWindowEnabled(true);

	m_ScrollBar.setParent(this);
	m_ScrollBar.addIntChangeSignal(&m_Signal);
	m_ScrollBar.setSlider(&m_Slider);
	m_ScrollBar.setButtonPressedScrollValue(1);

	m_Items.m_pNext = m_Items.m_pPrev = &m_Items;
	m_ItemOffset = 0;
	m_iScrollMax = -1;
}

CListBox::~CListBox()
{
	Term();
}

void CListBox::Init()
{
	Term();
}

void CListBox::Term()
{
	m_ItemOffset = 0;

	// Free the LBItems.
	LBItem *pNext;
	for(LBItem *pItem=m_Items.m_pNext; pItem != &m_Items; pItem=pNext)
	{
		pItem->m_pPanel->setParent(NULL);	// detach the panel from us
		pNext = pItem->m_pNext;
		delete pItem;
	}
	m_Items.m_pPrev = m_Items.m_pNext = &m_Items;
}

void CListBox::AddItem(Panel* panel)
{
	// Add the item.
	LBItem *pItem = new LBItem;
	if(!pItem)
		return;

	pItem->m_pPanel = panel;
	pItem->m_pPanel->setParent(&m_ItemsPanel);

	pItem->m_pPrev = m_Items.m_pPrev;
	pItem->m_pNext = &m_Items;
	pItem->m_pNext->m_pPrev = pItem->m_pPrev->m_pNext = pItem;	

	m_ScrollBar.setRange(0, GetScrollMax());
	m_Slider.setRangeWindow(50);
	m_Slider.setRangeWindowEnabled(true);

	InternalLayout();
}

int CListBox::GetNumItems()
{
	int count=0;
	for(LBItem *pItem=m_Items.m_pNext; pItem != &m_Items; pItem=pItem->m_pNext)
		++count;

	return count;
}

int CListBox::GetItemWidth()
{
	int wide, tall;
	m_ItemsPanel.getSize(wide, tall);
	return wide;
}

int CListBox::GetScrollPos()
{
	return m_ItemOffset;
}

void CListBox::SetScrollPos(int pos)
{
	int maxItems = GetScrollMax();
	if(maxItems < 0)
		return;

	m_ItemOffset = (pos < 0) ? 0 : ((pos > maxItems) ? maxItems : pos);
	InternalLayout();
}

void CListBox::setPos(int x, int y)
{
	Panel::setPos(x, y);
	InternalLayout();
}

void CListBox::setSize(int wide,int tall)
{
	Panel::setSize(wide,tall);
	InternalLayout();
}

void CListBox::setPixelScroll(int value)
{
	m_ItemOffset = m_ScrollBar.getValue();
	InternalLayout();
}

void CListBox::InternalLayout()
{
	int x, y, wide, tall;
	getBounds(x, y, wide, tall);

	// Reposition the main panel and the scrollbar.
	m_ItemsPanel.setBounds(0, 0, wide-15, tall);
	m_ScrollBar.setBounds(wide-15, 0, 15, tall);

	bool bNeedScrollbar = false;

	// Reposition the items.
	int curItem = 0;
	int curY = 0;
	int maxItem = GetScrollMax();
	for(LBItem *pItem=m_Items.m_pNext; pItem != &m_Items; pItem=pItem->m_pNext)
	{
		if(curItem < m_ItemOffset)
		{
			pItem->m_pPanel->setVisible(false);
			bNeedScrollbar = true;
		}
		else if (curItem >= maxItem)
		{
			// item is past the end of the items we care about
			pItem->m_pPanel->setVisible(false);
		}
		else
		{
			pItem->m_pPanel->setVisible(true);

			int itemWidth, itemHeight;
			pItem->m_pPanel->getSize(itemWidth, itemHeight);

			// Don't change the item's height but change its width to fit the listbox.
			pItem->m_pPanel->setBounds(0, curY, wide, itemHeight);

			curY += itemHeight;

			if (curY > tall)
			{
				bNeedScrollbar = true;
			}
		}

		++curItem;
	}

	m_ScrollBar.setVisible(bNeedScrollbar);

	repaint();
}

void CListBox::paintBackground()
{
}

void CListBox::SetScrollRange(int maxScroll)
{
	m_iScrollMax = maxScroll;
	m_ScrollBar.setRange(0, maxScroll);
	InternalLayout();
}

int	CListBox::GetScrollMax()
{
	if (m_iScrollMax < 0)
	{
		return GetNumItems() - 1;
	}

	return m_iScrollMax;
}


