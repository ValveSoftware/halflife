//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_CHECKBUTTON2_H
#define VGUI_CHECKBUTTON2_H
#ifdef _WIN32
#pragma once
#endif


#include "vgui_label.h"
#include "vgui_imagepanel.h"
#include "vgui_defaultinputsignal.h"


namespace vgui
{


class CCheckButton2;


class ICheckButton2Handler
{
public:
	virtual void	StateChanged(CCheckButton2 *pButton) = 0;
};


// VGUI checkbox class.
// - Provides access to the checkbox images.
// - Provides an easy callback mechanism for state changes.
// - Default background is invisible, and default text color is white.
class CCheckButton2 : public Panel, public CDefaultInputSignal
{
public:

				CCheckButton2();
				~CCheckButton2();
	
	// Initialize the button with these.
	void		SetImages(char const *pChecked, char const *pUnchecked);
	void		SetImages(Image *pChecked, Image *pUnchecked);		// If you use this, the button will never delete the images.
	void		DeleteImages();

	// The checkbox can be to the left or the right of the text (default is left).
	void		SetCheckboxLeft(bool bLeftAlign);
	bool		GetCheckboxLeft();
	
	// Set the label text.
	void		SetText(char const *pText, ...);
	void		SetTextColor(int r, int g, int b, int a);

	// You can register for change notification here.
	void		SetHandler(ICheckButton2Handler *pHandler);
	
	// Get/set the check state.
	bool		IsChecked();
	void		SetChecked(bool bChecked);



// Panel overrides.
public:

	virtual void	internalMousePressed(MouseCode code);	


protected:

	void			SetupControls();


// InputSignal overrides.
protected:
	virtual void mousePressed(MouseCode code,Panel* panel);


public:
	ICheckButton2Handler	*m_pHandler;

	bool		m_bCheckboxLeft;
	Label		m_Label;
	ImagePanel	m_CheckboxPanel;
	
	Image		*m_pChecked;
	Image		*m_pUnchecked;
	bool		m_bOwnImages;

	bool		m_bChecked;
};


}


#endif // VGUI_CHECKBUTTON2_H
