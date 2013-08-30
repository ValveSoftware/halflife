//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef SCOREPANEL_H
#define SCOREPANEL_H

#include<VGUI_Panel.h>
#include<VGUI_TablePanel.h>
#include<VGUI_HeaderPanel.h>
#include<VGUI_TextGrid.h>
#include<VGUI_Label.h>
#include<VGUI_TextImage.h>
#include "../game_shared/vgui_listbox.h"

#include <ctype.h>

#define MAX_SCORES					10
#define MAX_SCOREBOARD_TEAMS		5

// Scoreboard cells
#define COLUMN_TRACKER	0
#define COLUMN_NAME		1
#define COLUMN_CLASS	2
#define COLUMN_KILLS	3
#define COLUMN_DEATHS	4
#define COLUMN_LATENCY	5
#define COLUMN_VOICE	6
#define COLUMN_BLANK	7
#define NUM_COLUMNS		8
#define NUM_ROWS		(MAX_PLAYERS + (MAX_SCOREBOARD_TEAMS * 2))

using namespace vgui;

class CTextImage2 : public Image
{
public:
	CTextImage2()
	{
		_image[0] = new TextImage("");
		_image[1] = new TextImage("");
	}

	~CTextImage2()
	{
		delete _image[0];
		delete _image[1];
	}

	TextImage *GetImage(int image)
	{
		return _image[image];
	}

	void getSize(int &wide, int &tall)
	{
		int w1, w2, t1, t2;
		_image[0]->getTextSize(w1, t1);
		_image[1]->getTextSize(w2, t2);

		wide = w1 + w2;
		tall = max(t1, t2);
		setSize(wide, tall);
	}

	void doPaint(Panel *panel)
	{
		_image[0]->doPaint(panel);
		_image[1]->doPaint(panel);
	}

	void setPos(int x, int y)
	{
		_image[0]->setPos(x, y);
		
		int swide, stall;
		_image[0]->getSize(swide, stall);

		int wide, tall;
		_image[1]->getSize(wide, tall);
		_image[1]->setPos(x + wide, y + (stall * 0.9) - tall);
	}

	void setColor(Color color)
	{
		_image[0]->setColor(color);
	}

	void setColor2(Color color)
	{
		_image[1]->setColor(color);
	}

private:
	TextImage *_image[2];

};

//-----------------------------------------------------------------------------
// Purpose: Custom label for cells in the Scoreboard's Table Header
//-----------------------------------------------------------------------------
class CLabelHeader : public Label
{
public:
	CLabelHeader() : Label("")
	{
		_dualImage = new CTextImage2();
		_dualImage->setColor2(Color(255, 170, 0, 0));
		_row = -2;
		_useFgColorAsImageColor = true;
		_offset[0] = 0;
		_offset[1] = 0;
	}

	~CLabelHeader()
	{
		delete _dualImage;
	}

	void setRow(int row)
	{
		_row = row;
	}

	void setFgColorAsImageColor(bool state)
	{
		_useFgColorAsImageColor = state;
	}

	virtual void setText(int textBufferLen, const char* text)
	{
		_dualImage->GetImage(0)->setText(text);

		// calculate the text size
		Font *font = _dualImage->GetImage(0)->getFont();
		_gap = 0;
		for (const char *ch = text; *ch != 0; ch++)
		{
			int a, b, c;
			font->getCharABCwide(*ch, a, b, c);
			_gap += (a + b + c);
		}

		_gap += XRES(5);
	}

	virtual void setText(const char* text)
	{
		// strip any non-alnum characters from the end
		char buf[512];
		strcpy(buf, text);

		int len = strlen(buf);
		while (len && isspace(buf[--len]))
		{
			buf[len] = 0;
		}

		CLabelHeader::setText(0, buf);
	}

	void setText2(const char *text)
	{
		_dualImage->GetImage(1)->setText(text);
	}

	void getTextSize(int &wide, int &tall)
	{
		_dualImage->getSize(wide, tall);
	}

	void setFgColor(int r,int g,int b,int a)
	{
		Label::setFgColor(r,g,b,a);
		Color color(r,g,b,a);
		_dualImage->setColor(color);
		_dualImage->setColor2(color);
		repaint();
	}

	void setFgColor(Scheme::SchemeColor sc)
	{
		int r, g, b, a;
		Label::setFgColor(sc);
		Label::getFgColor( r, g, b, a );

		// Call the r,g,b,a version so it sets the color in the dualImage..
		setFgColor( r, g, b, a );
	}

	void setFont(Font *font)
	{
		_dualImage->GetImage(0)->setFont(font);
	}

	void setFont2(Font *font)
	{
		_dualImage->GetImage(1)->setFont(font);
	}

	// this adjust the absolute position of the text after alignment is calculated
	void setTextOffset(int x, int y)
	{
		_offset[0] = x;
		_offset[1] = y;
	}

	void paint();
	void paintBackground();
	void calcAlignment(int iwide, int itall, int &x, int &y);

private:
	CTextImage2 *_dualImage;
	int _row;
	int _gap;
	int _offset[2];
	bool _useFgColorAsImageColor;
};

class ScoreTablePanel;

#include "../game_shared/vgui_grid.h"
#include "../game_shared/vgui_defaultinputsignal.h"

//-----------------------------------------------------------------------------
// Purpose: Scoreboard back panel
//-----------------------------------------------------------------------------
class ScorePanel : public Panel, public vgui::CDefaultInputSignal
{
private:
	// Default panel implementation doesn't forward mouse messages when there is no cursor and we need them.
	class HitTestPanel : public Panel
	{
	public:
		virtual void	internalMousePressed(MouseCode code);
	};


private:

	Label			m_TitleLabel;
	
	// Here is how these controls are arranged hierarchically.
	// m_HeaderGrid
	//     m_HeaderLabels

	// m_PlayerGridScroll
	//     m_PlayerGrid
	//         m_PlayerEntries 

	CGrid			m_HeaderGrid;
	CLabelHeader	m_HeaderLabels[NUM_COLUMNS];			// Labels above the 
	CLabelHeader	*m_pCurrentHighlightLabel;
	int				m_iHighlightRow;
	
	vgui::CListBox	m_PlayerList;
	CGrid			m_PlayerGrids[NUM_ROWS];				// The grid with player and team info. 
	CLabelHeader	m_PlayerEntries[NUM_COLUMNS][NUM_ROWS];	// Labels for the grid entries.

	ScorePanel::HitTestPanel	m_HitTestPanel;
	CommandButton				*m_pCloseButton;
	CLabelHeader*	GetPlayerEntry(int x, int y)	{return &m_PlayerEntries[x][y];}

public:
	
	int				m_iNumTeams;
	int				m_iPlayerNum;
	int				m_iShowscoresHeld;

	int				m_iRows;
	int				m_iSortedRows[NUM_ROWS];
	int				m_iIsATeam[NUM_ROWS];
	bool			m_bHasBeenSorted[MAX_PLAYERS];
	int				m_iLastKilledBy;
	int				m_fLastKillTime;


public:

	ScorePanel(int x,int y,int wide,int tall);

	void Update( void );

	void SortTeams( void );
	void SortPlayers( int iTeam, char *team );
	void RebuildTeams( void );

	void FillGrid();

	void DeathMsg( int killer, int victim );

	void Initialize( void );

	void Open( void );

	void MouseOverCell(int row, int col);

// InputSignal overrides.
public:

	virtual void mousePressed(MouseCode code, Panel* panel);
	virtual void cursorMoved(int x, int y, Panel *panel);

	friend class CLabelHeader;
};

#endif
