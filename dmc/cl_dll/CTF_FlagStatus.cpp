#ifdef THREEWAVE

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_materials.h"
#include "ref_params.h"
#include <string.h>
#include "vgui_viewport.h"
#include "vgui_ScorePanel.h"

#define RED_FLAG_STOLE 1
#define BLUE_FLAG_STOLE 2
#define RED_FLAG_LOST 3
#define BLUE_FLAG_LOST 4
#define RED_FLAG_ATBASE 5
#define BLUE_FLAG_ATBASE 6

#define ITEM_RUNE1_FLAG                 1
#define ITEM_RUNE2_FLAG                 2
#define ITEM_RUNE3_FLAG                 3
#define ITEM_RUNE4_FLAG                 4

DECLARE_MESSAGE(m_FlagStat, FlagStat)
DECLARE_MESSAGE(m_FlagStat, RuneStat)
DECLARE_MESSAGE(m_FlagStat, FlagCarrier)

int CHudFlagStatus::Init(void)
{    
	HOOK_MESSAGE( FlagStat );
	HOOK_MESSAGE( RuneStat );
	HOOK_MESSAGE( FlagCarrier );

    m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	Reset();
    
    return 1;
}; 

int CHudFlagStatus::VidInit(void)
{
	m_iBlueAtBaseIndex = gHUD.GetSpriteIndex( "blue_atbase" );
	m_iBlueLostIndex = gHUD.GetSpriteIndex( "blue_lost" );
	m_iBlueStolenIndex = gHUD.GetSpriteIndex( "blue_stolen" );

	m_iRedAtBaseIndex = gHUD.GetSpriteIndex( "red_atbase" );
	m_iRedLostIndex = gHUD.GetSpriteIndex( "red_lost" );
	m_iRedStolenIndex = gHUD.GetSpriteIndex( "red_stolen" );

	m_iRune1Index = gHUD.GetSpriteIndex( "rune1" );
	m_iRune2Index = gHUD.GetSpriteIndex( "rune2" );
	m_iRune3Index = gHUD.GetSpriteIndex( "rune3" );
	m_iRune4Index = gHUD.GetSpriteIndex( "rune4" );

	m_hBlueAtBase = gHUD.GetSprite( m_iBlueAtBaseIndex );
	m_hBlueLost = gHUD.GetSprite( m_iBlueLostIndex );
	m_hBlueStolen = gHUD.GetSprite( m_iBlueStolenIndex );

	m_hRedAtBase = gHUD.GetSprite( m_iRedAtBaseIndex );
	m_hRedLost = gHUD.GetSprite( m_iRedLostIndex );
	m_hRedStolen = gHUD.GetSprite( m_iRedStolenIndex );

	m_hRune1 = gHUD.GetSprite( m_iRune1Index );
	m_hRune2 = gHUD.GetSprite( m_iRune2Index );
	m_hRune3 = gHUD.GetSprite( m_iRune3Index );
	m_hRune4 = gHUD.GetSprite( m_iRune4Index );

	// Load sprites here
	m_iBlueFlagIndex = gHUD.GetSpriteIndex( "b_flag_c" );
	m_iRedFlagIndex = gHUD.GetSpriteIndex( "r_flag_c" );

	m_hBlueFlag = gHUD.GetSprite( m_iBlueFlagIndex );
	m_hRedFlag = gHUD.GetSprite( m_iRedFlagIndex );

    return 1;
}

void CHudFlagStatus :: Reset( void )
{
     return;
}


int CHudFlagStatus ::Draw(float flTime )
{

   if ( !iDrawStatus )
	   return 1;

   int x, y;
   int r,g,b;

   r = g = b = 255;

   x = 20;
   y = ( ScreenHeight - gHUD.m_iFontHeight ) - ( gHUD.m_iFontHeight / 2 ) - 40;
   

   switch ( iBlueFlagStatus )
   {
	   case BLUE_FLAG_STOLE:
		   SPR_Set( m_hBlueStolen, r, g, b );
		   SPR_DrawHoles( 1, x, y, NULL );
		   break;
       case BLUE_FLAG_LOST:
		   SPR_Set( m_hBlueLost, r, g, b );
		   SPR_DrawHoles( 1, x, y, NULL );
		   break;
	   case BLUE_FLAG_ATBASE:
		   SPR_Set( m_hBlueAtBase, r, g, b );
		   SPR_DrawHoles( 1, x, y, NULL );
		   break;
   }

   x = 50;
   
   if ( iBlueTeamScore < 10)
   {
	   x += 3;
   	   gHUD.DrawHudNumber( x, y + 4, DHN_DRAWZERO, iBlueTeamScore, 255, 255, 255 );
   }
   else if ( iBlueTeamScore >= 10 && iBlueTeamScore < 100 )
        gHUD.DrawHudNumber( x, y + 4, DHN_2DIGITS | DHN_DRAWZERO, iBlueTeamScore, 255, 255, 255 );
      
   x = 20;
   y = ( ScreenHeight - gHUD.m_iFontHeight ) - ( gHUD.m_iFontHeight / 2 ) - 75;

   switch ( iRedFlagStatus )
   {
	   case RED_FLAG_STOLE:
		   SPR_Set( m_hRedStolen, r, g, b );
		   SPR_DrawHoles( 1, x, y, NULL );
		   break;
       case RED_FLAG_LOST:
		   SPR_Set( m_hRedLost, r, g, b );
		   SPR_DrawHoles( 1, x, y, NULL );
		   break;
	   case RED_FLAG_ATBASE:
		   SPR_Set( m_hRedAtBase, r, g, b );
		   SPR_DrawHoles( 1, x, y, NULL );
		   break;
   }
 
   x = 50;
   if ( iRedTeamScore < 10)
   {
	   x += 3;
	   gHUD.DrawHudNumber( x, y + 4, DHN_DRAWZERO, iRedTeamScore, 255, 255, 255 );
   }
   else if ( iBlueTeamScore >= 10 && iBlueTeamScore < 100 )
       gHUD.DrawHudNumber( x, y + 4, DHN_2DIGITS | DHN_DRAWZERO, iRedTeamScore, 255, 255, 255 );

   x = 20;
   y = ( ScreenHeight - gHUD.m_iFontHeight ) - ( gHUD.m_iFontHeight / 2 ) - 110;

   switch ( m_iRuneStat )
   {
		case ITEM_RUNE1_FLAG:
		   SPR_Set( m_hRune1, r, g, b );
		   SPR_Draw( 1, x, y, NULL );
		   break;

		case ITEM_RUNE2_FLAG:
		   SPR_Set( m_hRune2, r, g, b );
		   SPR_Draw( 1, x, y, NULL );
		   break;

		case ITEM_RUNE3_FLAG:
		   SPR_Set( m_hRune3, r, g, b );
		   SPR_Draw( 1, x, y, NULL );
		   break;

		case ITEM_RUNE4_FLAG:
		   SPR_Set( m_hRune4, r, g, b );
		   SPR_Draw( 1, x, y, NULL );
		   break;
   }
	   
   return 1;
}
 
int CHudFlagStatus::MsgFunc_FlagStat(const char *pszName, int iSize, void *pbuf)
{
    BEGIN_READ( pbuf, iSize ); 

	iDrawStatus = READ_BYTE();
	iRedFlagStatus = READ_BYTE();
	iBlueFlagStatus = READ_BYTE();

	iRedTeamScore = READ_BYTE();
	iBlueTeamScore = READ_BYTE();
	
	return 1;
}

int CHudFlagStatus::MsgFunc_RuneStat(const char *pszName, int iSize, void *pbuf)
{
    BEGIN_READ( pbuf, iSize ); 

	m_iRuneStat = READ_BYTE();
	
	return 1;
}

int CHudFlagStatus::MsgFunc_FlagCarrier(const char *pszName, int iSize, void *pbuf)
{
    BEGIN_READ( pbuf, iSize ); 

	int index = READ_BYTE();

	bool bRedFlag = false;
	bool bBlueFlag = false;

	g_PlayerExtraInfo[ index ].iHasFlag = READ_BYTE();

	for ( int i = 1; i < MAX_PLAYERS + 1; i++ )
	{
		if ( g_PlayerExtraInfo[ i ].iHasFlag )
		{
			if ( g_PlayerExtraInfo[ i ].teamnumber == 1 )
				bRedFlag = true;
			else if ( g_PlayerExtraInfo[ i ].teamnumber == 2 )
				bBlueFlag = true;
		}
	}
	
	if ( !bRedFlag )
		gViewPort->m_pScoreBoard->m_pImages[ 5 ]->setVisible( false );
	if ( !bBlueFlag )
		gViewPort->m_pScoreBoard->m_pImages[ 4 ]->setVisible( false );
			
	return 1;
}


#endif