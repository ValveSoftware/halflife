//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include <stdarg.h>
#include <stdio.h>
#include "vgui_checkbutton2.h"
#include "vgui_loadtga.h"


#define EXTRA_X	5


using namespace vgui;



CCheckButton2::CCheckButton2() :
	m_Label(""),
	m_pChecked(NULL),
	m_pUnchecked(NULL),
	m_pHandler(NULL),
	m_CheckboxPanel(NULL)
{
	m_bOwnImages = false;
	m_bChecked = false;
	m_pChecked = m_pUnchecked = NULL;
	m_bCheckboxLeft = true;

	m_Label.setParent(this);
	m_Label.setFgColor(255,255,255,0);
	m_Label.setBgColor(0,0,0,255);	// background is not drawn and foreground is white
	m_Label.addInputSignal(this);

	m_CheckboxPanel.setParent(this);
	m_CheckboxPanel.addInputSignal(this);

	setPaintBackgroundEnabled(false);
}


CCheckButton2::~CCheckButton2()
{
	DeleteImages();
}


void CCheckButton2::SetImages(char const *pChecked, char const *pUnchecked)
{
	DeleteImages();
	
	m_pChecked = vgui_LoadTGA(pChecked);
	m_pUnchecked = vgui_LoadTGA(pUnchecked);
	m_bOwnImages = true;

	SetupControls();
}


void CCheckButton2::SetImages(Image *pChecked, Image *pUnchecked)
{
	DeleteImages();

	m_pChecked = pChecked;
	m_pUnchecked = pUnchecked;
	m_bOwnImages = false;

	SetupControls();
}


void CCheckButton2::DeleteImages()
{
	if(m_bOwnImages)
	{
		delete m_pChecked;
		delete m_pUnchecked;
	}

	m_pChecked = NULL;
	m_pUnchecked = NULL;
	m_bOwnImages = false;

	SetupControls();
}


void CCheckButton2::SetCheckboxLeft(bool bLeftAlign)
{
	m_bCheckboxLeft = bLeftAlign;
	SetupControls();
}


bool CCheckButton2::GetCheckboxLeft()
{
	return m_bCheckboxLeft;
}


void CCheckButton2::SetText(char const *pText, ...)
{
	char str[512];
	
	va_list marker;
	va_start(marker, pText);
	_vsnprintf(str, sizeof(str), pText, marker);
	va_end(marker);

	m_Label.setText(str);
	SetupControls();
}


void CCheckButton2::SetTextColor(int r, int g, int b, int a)
{
	m_Label.setFgColor(r, g, b, a);
	repaint();
}


void CCheckButton2::SetHandler(ICheckButton2Handler *pHandler)
{
	m_pHandler = pHandler;
}


bool CCheckButton2::IsChecked()
{
	return m_bChecked;
}


void CCheckButton2::SetChecked(bool bChecked)
{
	m_bChecked = bChecked;
	SetupControls();
}


void CCheckButton2::internalMousePressed(MouseCode code)
{
	m_bChecked = !m_bChecked;

	if(m_pHandler)
		m_pHandler->StateChanged(this);

	SetupControls();
}


void CCheckButton2::SetupControls()
{
	// Initialize the checkbutton bitmap.
	Image *pBitmap = m_bChecked ? m_pChecked : m_pUnchecked;

	Panel *controls[2] = {&m_CheckboxPanel, &m_Label};
	int controlSizes[2][2];
	
	controlSizes[0][0] = controlSizes[0][1] = 0;
	if(pBitmap)
		pBitmap->getSize(controlSizes[0][0], controlSizes[0][1]);
	
	m_CheckboxPanel.setImage(pBitmap);
	m_CheckboxPanel.setSize(controlSizes[0][0], controlSizes[0][1]);

	
	// Get the label's size.
	m_Label.getSize(controlSizes[1][0], controlSizes[1][1]);
	m_Label.setContentAlignment(Label::a_west);


	// Position the controls.
	int iLeftControl = !m_bCheckboxLeft;
	int iBiggestY = controlSizes[0][1] > controlSizes[1][0] ? 0 : 1;
	controls[iLeftControl]->setPos(0, (controlSizes[iBiggestY][1] - controlSizes[iLeftControl][1]) / 2);
	controls[!iLeftControl]->setPos(controlSizes[iLeftControl][0] + EXTRA_X, (controlSizes[iBiggestY][1] - controlSizes[!iLeftControl][1]) / 2);

	
	// Fit this control to the sizes of the subcontrols.
	setSize(controlSizes[0][0] + controlSizes[1][0] + EXTRA_X, (controlSizes[0][1] > controlSizes[1][1]) ? controlSizes[0][1] : controlSizes[1][1]);
	repaint();
}


void CCheckButton2::mousePressed(MouseCode code, Panel *panel)
{
	internalMousePressed(code);
}





