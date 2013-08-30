//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: VGUI scoreboard
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================


#include<VGUI_LineBorder.h>

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_ScorePanel.h"
#include "vgui_helpers.h"
#include "vgui_loadtga.h"
//#include "ITrackerUser.h"
//extern ITrackerUser *g_pTrackerUser;

hud_player_info_t	 g_PlayerInfoList[MAX_PLAYERS+1];	   // player info from the engine
extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS+1];   // additional player info sent directly to the client dll
team_info_t			 g_TeamInfo[MAX_TEAMS+1];
int					 g_IsSpectator[MAX_PLAYERS+1];

int HUD_IsGame( const char *game );
int EV_TFC_IsAllyTeam( int iTeam1, int iTeam2 ){ return 1; }

int iNumberOfTeamColors = 33;

// Scoreboard dimensions
#define SBOARD_TITLE_SIZE_Y			YRES(22)

#define X_BORDER					XRES(4)

// Column sizes
class SBColumnInfo
{
public:
	char				*m_pTitle;		// If null, ignore, if starts with #, it's localized, otherwise use the string directly.
	int					m_Width;		// Based on 640 width. Scaled to fit other resolutions.
	Label::Alignment	m_Alignment;	
};

// grid size is marked out for 640x480 screen

SBColumnInfo g_ColumnInfo[NUM_COLUMNS] =
{
	{NULL,			24,			Label::a_east},		// tracker column
	{NULL,			140,		Label::a_east},		// name
	{"#QUEUE",		56,			Label::a_east},		// class
	{"#WINS",		40,			Label::a_east},
	{"#POINTS",		46,			Label::a_east},
	{"#LATENCY",	46,			Label::a_east},
	{"#VOICE",		40,			Label::a_east},
	{NULL,			2,			Label::a_east},		// blank column to take up the slack
};

extern float g_iaDiscColors[33][3];

#define TEAM_NO				0
#define TEAM_YES			1
#define TEAM_UNASSIGNED		2
#define TEAM_SPECTATORS		3


//-----------------------------------------------------------------------------
// ScorePanel::HitTestPanel.
//-----------------------------------------------------------------------------

void ScorePanel::HitTestPanel::internalMousePressed(MouseCode code)
{
	for(int i=0;i<_inputSignalDar.getCount();i++)
	{
		_inputSignalDar[i]->mousePressed(code,this);
	}
}



//-----------------------------------------------------------------------------
// Purpose: Create the ScoreBoard panel
//-----------------------------------------------------------------------------
ScorePanel::ScorePanel(int x,int y,int wide,int tall) : Panel(x,y,wide,tall)
{
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle("Scoreboard Title Text");
	SchemeHandle_t hSmallScheme = pSchemes->getSchemeHandle("Scoreboard Small Text");
	Font *tfont = pSchemes->getFont(hTitleScheme);
	Font *smallfont = pSchemes->getFont(hSmallScheme);

	setBgColor(0, 0, 0, 96);
	m_pCurrentHighlightLabel = NULL;
	m_iHighlightRow = -1;

	m_pTrackerIcon = vgui_LoadTGANoInvertAlpha("gfx/vgui/640_scoreboardtracker.tga");

	// Initialize the top title.
	m_TitleLabel.setFont(tfont);
	m_TitleLabel.setText("");
	m_TitleLabel.setBgColor( 0, 0, 0, 255 );
	m_TitleLabel.setFgColor( Scheme::sc_primary1 );
	m_TitleLabel.setContentAlignment( vgui::Label::a_west );

	LineBorder *border = new LineBorder(Color(60, 60, 60, 128));
	setBorder(border);
	setPaintBorderEnabled(true);

	int xpos = g_ColumnInfo[0].m_Width + 3;
	if (ScreenWidth >= 640)
	{
		// only expand column size for res greater than 640
		xpos = XRES(xpos);
	}
	m_TitleLabel.setBounds(xpos, 4, wide, SBOARD_TITLE_SIZE_Y);
	m_TitleLabel.setContentFitted(false);
	m_TitleLabel.setParent(this);

	// Setup the header (labels like "name", "class", etc..).
	m_HeaderGrid.SetDimensions(NUM_COLUMNS, 1);
	m_HeaderGrid.SetSpacing(0, 0);
	
	for(int i=0; i < NUM_COLUMNS; i++)
	{
		if (g_ColumnInfo[i].m_pTitle && g_ColumnInfo[i].m_pTitle[0] == '#')
			m_HeaderLabels[i].setText(CHudTextMessage::BufferedLocaliseTextString(g_ColumnInfo[i].m_pTitle));
		else if(g_ColumnInfo[i].m_pTitle)
			m_HeaderLabels[i].setText(g_ColumnInfo[i].m_pTitle);

		int xwide = g_ColumnInfo[i].m_Width;
		if (ScreenWidth >= 640)
		{
			xwide = XRES(xwide);
		}
		else if (ScreenWidth == 400)
		{
			// hack to make 400x300 resolution scoreboard fit
			if (i == 1)
			{
				// reduces size of player name cell
				xwide -= 28;
			}
			else if (i == 0)
			{
				// tracker icon cell
				xwide -= 8;
			}
		}
		
		m_HeaderGrid.SetColumnWidth(i, xwide);
		m_HeaderGrid.SetEntry(i, 0, &m_HeaderLabels[i]);

		m_HeaderLabels[i].setBgColor(0,0,0,255);
		m_HeaderLabels[i].setFgColor(Scheme::sc_primary1);
		m_HeaderLabels[i].setFont(smallfont);
		m_HeaderLabels[i].setContentAlignment(g_ColumnInfo[i].m_Alignment);

		int yres = 12;
		if (ScreenHeight >= 480)
		{
			yres = YRES(yres);
		}
		m_HeaderLabels[i].setSize(50, yres);
	}

	// Set the width of the last column to be the remaining space.
	int ex, ey, ew, eh;
	m_HeaderGrid.GetEntryBox(NUM_COLUMNS - 2, 0, ex, ey, ew, eh);
	m_HeaderGrid.SetColumnWidth(NUM_COLUMNS - 1, (wide - X_BORDER) - (ex + ew));

	m_HeaderGrid.AutoSetRowHeights();
	m_HeaderGrid.setBounds(X_BORDER, SBOARD_TITLE_SIZE_Y, wide - X_BORDER*2, m_HeaderGrid.GetRowHeight(0));
	m_HeaderGrid.setParent(this);
	m_HeaderGrid.setBgColor(0,0,0,255);


	// Now setup the listbox with the actual player data in it.
	int headerX, headerY, headerWidth, headerHeight;
	m_HeaderGrid.getBounds(headerX, headerY, headerWidth, headerHeight);
	m_PlayerList.setBounds(headerX, headerY+headerHeight, headerWidth, tall - headerY - headerHeight - 6);
	m_PlayerList.setBgColor(0,0,0,255);
	m_PlayerList.setParent(this);

	for(int row=0; row < NUM_ROWS; row++)
	{
		CGrid *pGridRow = &m_PlayerGrids[row];

		pGridRow->SetDimensions(NUM_COLUMNS, 1);
		
		for(int col=0; col < NUM_COLUMNS; col++)
		{
			m_PlayerEntries[col][row].setContentFitted(false);
			m_PlayerEntries[col][row].setRow(row);
			m_PlayerEntries[col][row].addInputSignal(this);
			pGridRow->SetEntry(col, 0, &m_PlayerEntries[col][row]);
		}

		pGridRow->setBgColor(0,0,0,255);
		pGridRow->SetSpacing(0, 0);
		pGridRow->CopyColumnWidths(&m_HeaderGrid);
		pGridRow->AutoSetRowHeights();
		pGridRow->setSize(PanelWidth(pGridRow), pGridRow->CalcDrawHeight());
		pGridRow->RepositionContents();

		m_PlayerList.AddItem(pGridRow);
	}


	// Add the hit test panel. It is invisible and traps mouse clicks so we can go into squelch mode.
	m_HitTestPanel.setBgColor(0,0,0,255);
	m_HitTestPanel.setParent(this);
	m_HitTestPanel.setBounds(0, 0, wide, tall);
	m_HitTestPanel.addInputSignal(this);

/*	m_pCloseButton = new CommandButton( "x", wide-XRES(12 + 4), YRES(2), XRES( 12 ) , YRES( 12 ) );
	m_pCloseButton->setParent( this );
	m_pCloseButton->addActionSignal( new CMenuHandler_StringCommandWatch( "-showscores", true ) );
	m_pCloseButton->setBgColor(0,0,0,255);
	m_pCloseButton->setFgColor( 255, 255, 255, 0 );
	m_pCloseButton->setFont(tfont);
	m_pCloseButton->setBoundKey( (char)255 );
	m_pCloseButton->setContentAlignment(Label::a_center);*/


	Initialize();
}


//-----------------------------------------------------------------------------
// Purpose: Called each time a new level is started.
//-----------------------------------------------------------------------------
void ScorePanel::Initialize( void )
{
	// Clear out scoreboard data
	m_iLastKilledBy = 0;
	m_fLastKillTime = 0;
	m_iPlayerNum = 0;
	m_iNumTeams = 0;
	memset( g_PlayerExtraInfo, 0, sizeof g_PlayerExtraInfo );
	memset( g_TeamInfo, 0, sizeof g_TeamInfo );
}

bool HACK_GetPlayerUniqueID( int iPlayer, char playerID[16] )
{
	return !!gEngfuncs.GetPlayerUniqueID( iPlayer, playerID ); // TODO remove after testing
}
		
extern int g_iArenaMode;

//-----------------------------------------------------------------------------
// Purpose: Recalculate the internal scoreboard data
//-----------------------------------------------------------------------------
void ScorePanel::Update()
{
	int i;

	// Set the title
	if (gViewPort->m_szServerName)
	{
		char sz[MAX_SERVERNAME_LENGTH + 16];
		sprintf(sz, "%s", gViewPort->m_szServerName );
		m_TitleLabel.setText(sz);
	}

	m_iRows = 0;
	gViewPort->GetAllPlayersInfo();

	
	//Hide or show the QUEUE and WINS labels.
	if ( g_iArenaMode == TRUE )
	{
		 m_HeaderLabels[2].setVisible ( true );
		 m_HeaderLabels[3].setVisible ( true );
	}
	else
	{
	     m_HeaderLabels[2].setVisible ( false );
		 m_HeaderLabels[3].setVisible ( false );
	}

	// Clear out sorts
	for (i = 0; i < NUM_ROWS; i++)
	{
		m_iSortedRows[i] = 0;
		m_iIsATeam[i] = TEAM_NO;
	}
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		m_bHasBeenSorted[i] = false;
	}

	SortTeams();

	// If it's not teamplay, sort all the players. Otherwise, sort the teams.
//	if ( !gHUD.m_Teamplay )
		SortPlayers( 0, NULL );
//	else
		

	// set scrollbar range
	m_PlayerList.SetScrollRange(m_iRows);

	FillGrid();
}

//-----------------------------------------------------------------------------
// Purpose: Sort all the teams
//-----------------------------------------------------------------------------
void ScorePanel::SortTeams()
{
	// clear out team scores
	int i;
	for ( i = 1; i <= m_iNumTeams; i++ )
	{
		if ( !g_TeamInfo[i].scores_overriden )
			g_TeamInfo[i].frags = g_TeamInfo[i].deaths = 0;
		g_TeamInfo[i].ping = g_TeamInfo[i].packetloss = 0;
	}

	// recalc the team scores, then draw them
	for ( i = 1; i < MAX_PLAYERS; i++ )
	{
		if ( g_PlayerInfoList[i].name == NULL )
			continue; // empty player slot, skip

		if ( g_PlayerExtraInfo[i].teamname[0] == 0 )
			continue; // skip over players who are not in a team

		// find what team this player is in
		int j;
		for ( j = 1; j <= m_iNumTeams; j++ )
		{
			if ( !stricmp( g_PlayerExtraInfo[i].teamname, g_TeamInfo[j].name ) )
				break;
		}
		if ( j > m_iNumTeams )  // player is not in a team, skip to the next guy
			continue;

		if ( !g_TeamInfo[j].scores_overriden )
		{
			g_TeamInfo[j].frags += g_PlayerExtraInfo[i].frags;
			g_TeamInfo[j].deaths += g_PlayerExtraInfo[i].deaths;
		}

		g_TeamInfo[j].ping += g_PlayerInfoList[i].ping;
		g_TeamInfo[j].packetloss += g_PlayerInfoList[i].packetloss;

		if ( g_PlayerInfoList[i].thisplayer )
			g_TeamInfo[j].ownteam = TRUE;
		else
			g_TeamInfo[j].ownteam = FALSE;

		// Set the team's number (used for team colors)
		g_TeamInfo[j].teamnumber = g_PlayerExtraInfo[i].teamnumber;
	}

	// find team ping/packetloss averages
	for ( i = 1; i <= m_iNumTeams; i++ )
	{
		g_TeamInfo[i].already_drawn = FALSE;

		if ( g_TeamInfo[i].players > 0 )
		{
			g_TeamInfo[i].ping /= g_TeamInfo[i].players;  // use the average ping of all the players in the team as the teams ping
			g_TeamInfo[i].packetloss /= g_TeamInfo[i].players;  // use the average ping of all the players in the team as the teams ping
		}
	}

	// Draw the teams
	while ( 1 )
	{
		int highest_frags = -99999; int lowest_deaths = 99999;
		int best_team = 0;

		for ( i = 1; i <= m_iNumTeams; i++ )
		{
			if ( g_TeamInfo[i].players < 1 )
				continue;

			if ( !g_TeamInfo[i].already_drawn && g_TeamInfo[i].frags >= highest_frags )
			{
				if ( g_TeamInfo[i].frags > highest_frags || g_TeamInfo[i].deaths < lowest_deaths )
				{
					best_team = i;
					lowest_deaths = g_TeamInfo[i].deaths;
					highest_frags = g_TeamInfo[i].frags;
				}
			}
		}

		// draw the best team on the scoreboard
		if ( !best_team )
			break;

		// Put this team in the sorted list
		m_iSortedRows[ m_iRows ] = best_team;
		m_iIsATeam[ m_iRows ] = TEAM_YES;
		g_TeamInfo[best_team].already_drawn = TRUE;  // set the already_drawn to be TRUE, so this team won't get sorted again
		m_iRows++;

		// Now sort all the players on this team
		SortPlayers( 0, g_TeamInfo[best_team].name );
	}

	// Add all the players who aren't in a team yet into spectators
	SortPlayers( TEAM_SPECTATORS, NULL );
}

//-----------------------------------------------------------------------------
// Purpose: Sort a list of players
//-----------------------------------------------------------------------------
void ScorePanel::SortPlayers( int iTeam, char *team )
{
	bool bCreatedTeam = false;

	// draw the players, in order,  and restricted to team if set
	while ( 1 )
	{
		// Find the top ranking player
		int highest_frags = -99999;	int lowest_deaths = 99999;
		int best_player;
		best_player = 0;

		for ( int i = 1; i < MAX_PLAYERS; i++ )
		{
			if ( m_bHasBeenSorted[i] == false && g_PlayerInfoList[i].name && g_PlayerExtraInfo[i].frags >= highest_frags )
			{
				cl_entity_t *ent = gEngfuncs.GetEntityByIndex( i );

				if ( ent && !(team && stricmp(g_PlayerExtraInfo[i].teamname, team)) )  
				{
					extra_player_info_t *pl_info = &g_PlayerExtraInfo[i];
					if ( pl_info->frags > highest_frags || pl_info->deaths < lowest_deaths )
					{
						best_player = i;
						lowest_deaths = pl_info->deaths;
						highest_frags = pl_info->frags;
					}
				}
			}
		}

		if ( !best_player )
			break;

		// If we haven't created the Team yet, do it first
		if (!bCreatedTeam && iTeam)
		{
			m_iIsATeam[ m_iRows ] = iTeam;
			m_iRows++;

			bCreatedTeam = true;
		}

		// Put this player in the sorted list
		m_iSortedRows[ m_iRows ] = best_player;
		m_bHasBeenSorted[ best_player ] = true;
		m_iRows++;
	}

	if (team)
	{
		m_iIsATeam[m_iRows++] = TEAM_SPECTATORS;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Recalculate the existing teams in the match
//-----------------------------------------------------------------------------
void ScorePanel::RebuildTeams()
{
	// clear out player counts from teams
	int i;
	for ( i = 1; i <= m_iNumTeams; i++ )
	{
		g_TeamInfo[i].players = 0;
	}

	// rebuild the team list
	gViewPort->GetAllPlayersInfo();
	m_iNumTeams = 0;
	for ( i = 1; i < MAX_PLAYERS; i++ )
	{
		if ( g_PlayerInfoList[i].name == NULL )
			continue;

		if ( g_PlayerExtraInfo[i].teamname[0] == 0 )
			continue; // skip over players who are not in a team

		// is this player in an existing team?
		int j;
		for ( j = 1; j <= m_iNumTeams; j++ )
		{
			if ( g_TeamInfo[j].name[0] == '\0' )
				break;

			if ( !stricmp( g_PlayerExtraInfo[i].teamname, g_TeamInfo[j].name ) )
				break;
		}

		if ( j > m_iNumTeams )
		{ // they aren't in a listed team, so make a new one
			// search through for an empty team slot
			for ( int j = 1; j <= m_iNumTeams; j++ )
			{
				if ( g_TeamInfo[j].name[0] == '\0' )
					break;
			}
			m_iNumTeams = max( j, m_iNumTeams );

			strncpy( g_TeamInfo[j].name, g_PlayerExtraInfo[i].teamname, MAX_TEAM_NAME );
			g_TeamInfo[j].players = 0;
		}

		g_TeamInfo[j].players++;
	}

	// clear out any empty teams
	for ( i = 1; i <= m_iNumTeams; i++ )
	{
		if ( g_TeamInfo[i].players < 1 )
			memset( &g_TeamInfo[i], 0, sizeof(team_info_t) );
	}

	// Update the scoreboard
	Update();
}


void ScorePanel::FillGrid()
{
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hScheme = pSchemes->getSchemeHandle("Scoreboard Text");
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle("Scoreboard Title Text");
	SchemeHandle_t hSmallScheme = pSchemes->getSchemeHandle("Scoreboard Small Text");

	Font *sfont = pSchemes->getFont(hScheme);
	Font *tfont = pSchemes->getFont(hTitleScheme);
	Font *smallfont = pSchemes->getFont(hSmallScheme);
	int i = 0;

	// update highlight position
	int x, y;
	getApp()->getCursorPos(x, y);
	cursorMoved(x, y, this);

	// remove highlight row if we're not in squelch mode
	if (!GetClientVoiceMgr()->IsInSquelchMode())
	{
		m_iHighlightRow = -1;
	}

	bool bNextRowIsGap = false;

	int row;
	for(row=0; row < NUM_ROWS; row++)
	{
		CGrid *pGridRow = &m_PlayerGrids[row];
		pGridRow->SetRowUnderline(0, false, 0, 0, 0, 0, 0);

		if(row >= m_iRows)
		{
			for(int col=0; col < NUM_COLUMNS; col++)
				m_PlayerEntries[col][row].setVisible(false);
		
			continue;
		}

		bool bRowIsGap = false;
		if (bNextRowIsGap)
		{
			bNextRowIsGap = false;
			bRowIsGap = true;
		}

		for(int col=0; col < NUM_COLUMNS; col++)
		{
			CLabelHeader *pLabel = &m_PlayerEntries[col][row];

			pLabel->setVisible(true);
			pLabel->setText2("");
			pLabel->setImage(NULL);
			pLabel->setFont(sfont);
			pLabel->setTextOffset(0, 0);
			
			int rowheight = 13;
			if (ScreenHeight > 480)
			{
				rowheight = YRES(rowheight);
			}
			else
			{
				// more tweaking, make sure icons fit at low res
				rowheight = 15;
			}
			pLabel->setSize(pLabel->getWide(), rowheight);
			pLabel->setBgColor(0, 0, 0, 255);
			
			char sz[128];
			hud_player_info_t *pl_info = NULL;
			team_info_t *team_info = NULL;

			if (m_iIsATeam[row] == TEAM_SPECTATORS)
			{
				pLabel->setText(" ");
				continue;
			}
			else if ( m_iIsATeam[row] == TEAM_YES )
			{
				// Get the team's data
				team_info = &g_TeamInfo[ m_iSortedRows[row] ];

				// team color text for team names
				pLabel->setFgColor(	iTeamColors[team_info->teamnumber % 5][0],
									iTeamColors[team_info->teamnumber % 5][1],
									iTeamColors[team_info->teamnumber % 5][2],
									0 );

				// different height for team header rows
				rowheight = 20;
				if (ScreenHeight >= 480)
				{
					rowheight = YRES(rowheight);
				}
				pLabel->setSize(pLabel->getWide(), rowheight);
				pLabel->setFont(tfont);

				pGridRow->SetRowUnderline(	0,
											true,
											YRES(3),
											iTeamColors[team_info->teamnumber % 5][0],
											iTeamColors[team_info->teamnumber % 5][1],
											iTeamColors[team_info->teamnumber % 5][2],
											0 );
			}
			else if ( m_iIsATeam[row] == TEAM_SPECTATORS )
			{
				// grey text for spectators
				pLabel->setFgColor(100, 100, 100, 0);

				// different height for team header rows
				rowheight = 20;
				if (ScreenHeight >= 480)
				{
					rowheight = YRES(rowheight);
				}
				pLabel->setSize(pLabel->getWide(), rowheight);
				pLabel->setFont(tfont);

				pGridRow->SetRowUnderline(0, true, YRES(3), 100, 100, 100, 0);
			}
			else
			{
				// team color text for player names
				pLabel->setFgColor(	g_iaDiscColors[ g_PlayerExtraInfo[ m_iSortedRows[row] ].teamnumber % iNumberOfTeamColors ][0],
									g_iaDiscColors[ g_PlayerExtraInfo[ m_iSortedRows[row] ].teamnumber % iNumberOfTeamColors ][1],
									g_iaDiscColors[ g_PlayerExtraInfo[ m_iSortedRows[row] ].teamnumber % iNumberOfTeamColors ][2],
									0 );

				// Get the player's data
				pl_info = &g_PlayerInfoList[ m_iSortedRows[row] ];

				// Set background color
				if ( pl_info->thisplayer ) // if it is their name, draw it a different color
				{
					// Highlight this player
					pLabel->setFgColor(Scheme::sc_white);
					pLabel->setBgColor(	g_iaDiscColors[ g_PlayerExtraInfo[ m_iSortedRows[row] ].teamnumber % iNumberOfTeamColors ][0],
										g_iaDiscColors[ g_PlayerExtraInfo[ m_iSortedRows[row] ].teamnumber % iNumberOfTeamColors ][1],
										g_iaDiscColors[ g_PlayerExtraInfo[ m_iSortedRows[row] ].teamnumber % iNumberOfTeamColors ][2],
										196 );
				}
				else if ( m_iSortedRows[row] == m_iLastKilledBy && m_fLastKillTime && m_fLastKillTime > gHUD.m_flTime )
				{
					// Killer's name
					pLabel->setBgColor( 255,0,0, 255 - ((float)15 * (float)(m_fLastKillTime - gHUD.m_flTime)) );
				}
			}				

			// Align 
			if (col == COLUMN_NAME )
			{
				pLabel->setContentAlignment( vgui::Label::a_west );
			}
			else if (col == COLUMN_TRACKER)
			{
				pLabel->setContentAlignment( vgui::Label::a_center );
			}
			else
			{
				pLabel->setContentAlignment( vgui::Label::a_east );
			}

			// Fill out with the correct data
			strcpy(sz, "");
			if ( m_iIsATeam[row] )
			{
				char sz2[128];

				switch (col)
				{
				case COLUMN_NAME:
					if ( m_iIsATeam[row] == TEAM_UNASSIGNED )
					{
						sprintf( sz2, CHudTextMessage::BufferedLocaliseTextString( "#Queue" ) );
					}
					else if ( m_iIsATeam[row] == TEAM_SPECTATORS )
					{
						sprintf( sz2, CHudTextMessage::BufferedLocaliseTextString( "#Spectators" ) );
					}
					else
					{
						sprintf( sz2, CHudTextMessage::BufferedLocaliseTextString( team_info->name ) );
					}

					// Uppercase it
					for ( i = 0; i < (int)strlen(sz2); i++)
					{
						if ( *(sz2 + i) )
							sz[i] = toupper( *(sz2 + i) );
					}
					sz[i] = '\0';

					// Append the number of players
					if ( m_iIsATeam[row] == TEAM_YES )
					{
						char *pszTemp = NULL;

						if ( team_info->players == 1 )
						{
							pszTemp = "#Player";
						}
						else
						{
							pszTemp = "#Player_plural";
						}

						sprintf(sz, "%s (%d %s)", sz, team_info->players, CHudTextMessage::BufferedLocaliseTextString( pszTemp ) );
					}
			
					break;
				case COLUMN_VOICE:
					break;
				case COLUMN_CLASS:
					break;
				case COLUMN_KILLS: //Wins
					if ( g_iArenaMode == FALSE )
						 strcpy ( sz, " " );
					else
					{
						if ( m_iIsATeam[row] == TEAM_YES )
							sprintf(sz, "%d",  team_info->deaths );
					}
					break;
				case COLUMN_DEATHS: //Points
					if ( m_iIsATeam[row] == TEAM_YES )
						sprintf(sz, "%d",  team_info->frags );
					break;
				case COLUMN_LATENCY:
					if ( m_iIsATeam[row] == TEAM_YES )
						sprintf(sz, "%d", team_info->ping );
					break;
				default:
					break;
				}
			}
			else
			{
				bool bShowClass = false;

				switch (col)
				{
				case COLUMN_NAME:
					/*
					if (g_pTrackerUser)
					{
						int playerSlot = m_iSortedRows[row];
						int trackerID = gEngfuncs.GetTrackerIDForPlayer(playerSlot);
						const char *trackerName = g_pTrackerUser->GetUserName(trackerID);
						if (trackerName && *trackerName)
						{
							sprintf(sz, "   (%s)", trackerName);
							pLabel->setText2(sz);
						}
					}
					*/
					sprintf(sz, "%s  ", pl_info->name);
					break;
				case COLUMN_VOICE:
					sz[0] = 0;
					GetClientVoiceMgr()->UpdateSpeakerImage(pLabel, m_iSortedRows[row]);
					break;
				case COLUMN_CLASS:
					
						if ( g_PlayerExtraInfo[ m_iSortedRows[row] ].teamnumber == 0 && g_iArenaMode == TRUE )
						{
							sprintf(sz, "%d",  g_PlayerExtraInfo[ m_iSortedRows[row] ].playerclass );
						}
						else
						{
							strcpy(sz, "");
						}
					
					break;

				case COLUMN_TRACKER:
						/*
					if (g_pTrackerUser)
					{
						int playerSlot = m_iSortedRows[row];
						int trackerID = gEngfuncs.GetTrackerIDForPlayer(playerSlot);

						if (g_pTrackerUser->IsFriend(trackerID) && trackerID != g_pTrackerUser->GetTrackerID())
						{
							pLabel->setImage(m_pTrackerIcon);
							pLabel->setFgColorAsImageColor(false);
							m_pTrackerIcon->setColor(Color(255, 255, 255, 0));
						}
					}*/
					break;

				case COLUMN_KILLS: //Wins
			        if ( g_iArenaMode == TRUE )
					     sprintf(sz, "%d",  g_PlayerExtraInfo[ m_iSortedRows[row] ].deaths );
					break;
				case COLUMN_DEATHS: //Points
					sprintf(sz, "%d",  g_PlayerExtraInfo[ m_iSortedRows[row] ].frags );
					break;
				case COLUMN_LATENCY:
					sprintf(sz, "%d", g_PlayerInfoList[ m_iSortedRows[row] ].ping );
					break;
				default:
					break;
				}
			}

			pLabel->setText(sz);
		}
	}

	for(row=0; row < NUM_ROWS; row++)
	{
		CGrid *pGridRow = &m_PlayerGrids[row];

		pGridRow->AutoSetRowHeights();
		pGridRow->setSize(PanelWidth(pGridRow), pGridRow->CalcDrawHeight());
		pGridRow->RepositionContents();
	}

	// hack, for the thing to resize
	m_PlayerList.getSize(x, y);
	m_PlayerList.setSize(x, y);
}


//-----------------------------------------------------------------------------
// Purpose: Setup highlights for player names in scoreboard
//-----------------------------------------------------------------------------
void ScorePanel::DeathMsg( int killer, int victim )
{
	// if we were the one killed,  or the world killed us, set the scoreboard to indicate suicide
	if ( victim == m_iPlayerNum || killer == 0 )
	{
		m_iLastKilledBy = killer ? killer : m_iPlayerNum;
		m_fLastKillTime = gHUD.m_flTime + 10;	// display who we were killed by for 10 seconds

		if ( killer == m_iPlayerNum )
			m_iLastKilledBy = m_iPlayerNum;
	}
}


void ScorePanel::Open( void )
{
	RebuildTeams();
	setVisible(true);
	m_HitTestPanel.setVisible(true);
}


void ScorePanel::mousePressed(MouseCode code, Panel* panel)
{
	if(gHUD.m_iIntermission)
		return;

	if (!GetClientVoiceMgr()->IsInSquelchMode())
	{
		GetClientVoiceMgr()->StartSquelchMode();
		m_HitTestPanel.setVisible(false);
	}
	else if (m_iHighlightRow >= 0)
	{
		// mouse has been pressed, toggle mute state
		int iPlayer = m_iSortedRows[m_iHighlightRow];
		if (iPlayer > 0)
		{
			// print text message
			hud_player_info_t *pl_info = &g_PlayerInfoList[iPlayer];

			if (pl_info && pl_info->name && pl_info->name[0])
			{
				char string[256];
				if (GetClientVoiceMgr()->IsPlayerBlocked(iPlayer))
				{
					char string1[1024];

					// remove mute
					GetClientVoiceMgr()->SetPlayerBlockedState(iPlayer, false);

					sprintf( string1, CHudTextMessage::BufferedLocaliseTextString( "#Unmuted" ), pl_info->name );
					sprintf( string, "%c** %s\n", HUD_PRINTTALK, string1 );

					gHUD.m_TextMessage.MsgFunc_TextMsg(NULL, strlen(string)+1, string );
				}
				else
				{
					char string1[1024];
					char string2[1024];

					// mute the player
					GetClientVoiceMgr()->SetPlayerBlockedState(iPlayer, true);

					sprintf( string1, CHudTextMessage::BufferedLocaliseTextString( "#Muted" ), pl_info->name );
					sprintf( string2, CHudTextMessage::BufferedLocaliseTextString( "#No_longer_hear_that_player" ) );
					sprintf( string, "%c** %s %s\n", HUD_PRINTTALK, string1, string2 );

					gHUD.m_TextMessage.MsgFunc_TextMsg(NULL, strlen(string)+1, string );
				}
			}
		}
	}
}

void ScorePanel::cursorMoved(int x, int y, Panel *panel)
{
	if (GetClientVoiceMgr()->IsInSquelchMode())
	{
		// look for which cell the mouse is currently over
		for (int i = 0; i < NUM_ROWS; i++)
		{
			int row, col;
			if (m_PlayerGrids[i].getCellAtPoint(x, y, row, col))
			{
				MouseOverCell(i, col);
				return;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handles mouse movement over a cell
// Input  : row - 
//			col - 
//-----------------------------------------------------------------------------
void ScorePanel::MouseOverCell(int row, int col)
{
	CLabelHeader *label = &m_PlayerEntries[col][row];

	// clear the previously highlighted label
	if (m_pCurrentHighlightLabel != label)
	{
		m_pCurrentHighlightLabel = NULL;
		m_iHighlightRow = -1;
	}
	if (!label)
		return;

	// don't act on teams
	if (m_iIsATeam[row] != TEAM_NO)
		return;

	// don't act on disconnected players or ourselves
	hud_player_info_t *pl_info = &g_PlayerInfoList[ m_iSortedRows[row] ];
	if (!pl_info->name || !pl_info->name[0])
		return;

	if (pl_info->thisplayer && !gEngfuncs.IsSpectateOnly() )
		return;

	// setup the new highlight
	m_pCurrentHighlightLabel = label;
	m_iHighlightRow = row;
}

//-----------------------------------------------------------------------------
// Purpose: Label paint functions - take into account current highligh status
//-----------------------------------------------------------------------------
void CLabelHeader::paintBackground()
{
	Color oldBg;
	getBgColor(oldBg);

	if (gViewPort->GetScoreBoard()->m_iHighlightRow == _row)
	{
		setBgColor(134, 91, 19, 0);
	}

	Panel::paintBackground();

	setBgColor(oldBg);
}
		

//-----------------------------------------------------------------------------
// Purpose: Label paint functions - take into account current highligh status
//-----------------------------------------------------------------------------
void CLabelHeader::paint()
{
	Color oldFg;
	getFgColor(oldFg);

	if (gViewPort->GetScoreBoard()->m_iHighlightRow == _row)
	{
		setFgColor(255, 255, 255, 0);
	}

	// draw text
	int x, y, iwide, itall;
	getTextSize(iwide, itall);
	calcAlignment(iwide, itall, x, y);
	_dualImage->setPos(x, y);

	int x1, y1;
	_dualImage->GetImage(1)->getPos(x1, y1);
	_dualImage->GetImage(1)->setPos(_gap, y1);

	_dualImage->doPaint(this);

	// get size of the panel and the image
	if (_image)
	{
		Color imgColor;
		getFgColor( imgColor );
		if( _useFgColorAsImageColor )
		{
			_image->setColor( imgColor );
		}

		_image->getSize(iwide, itall);
		calcAlignment(iwide, itall, x, y);
		_image->setPos(x, y);
		_image->doPaint(this);
	}

	setFgColor(oldFg[0], oldFg[1], oldFg[2], oldFg[3]);
}


void CLabelHeader::calcAlignment(int iwide, int itall, int &x, int &y)
{
	// calculate alignment ourselves, since vgui is so broken
	int wide, tall;
	getSize(wide, tall);

	x = 0, y = 0;

	// align left/right
	switch (_contentAlignment)
	{
		// left
		case Label::a_northwest:
		case Label::a_west:
		case Label::a_southwest:
		{
			x = 0;
			break;
		}
		
		// center
		case Label::a_north:
		case Label::a_center:
		case Label::a_south:
		{
			x = (wide - iwide) / 2;
			break;
		}
		
		// right
		case Label::a_northeast:
		case Label::a_east:
		case Label::a_southeast:
		{
			x = wide - iwide;
			break;
		}
	}

	// top/down
	switch (_contentAlignment)
	{
		// top
		case Label::a_northwest:
		case Label::a_north:
		case Label::a_northeast:
		{
			y = 0;
			break;
		}
		
		// center
		case Label::a_west:
		case Label::a_center:
		case Label::a_east:
		{
			y = (tall - itall) / 2;
			break;
		}
		
		// south
		case Label::a_southwest:
		case Label::a_south:
		case Label::a_southeast:
		{
			y = tall - itall;
			break;
		}
	}

// don't clip to Y
//	if (y < 0)
//	{
//		y = 0;
//	}
	if (x < 0)
	{
		x = 0;
	}

	x += _offset[0];
	y += _offset[1];
}
