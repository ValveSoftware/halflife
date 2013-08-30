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


#define MAX_BONUS 10

#define RED_FLAG_STOLEN 1
#define BLUE_FLAG_STOLEN 2
#define RED_FLAG_CAPTURED 3
#define BLUE_FLAG_CAPTURED 4
#define RED_FLAG_RETURNED_PLAYER 5
#define BLUE_FLAG_RETURNED_PLAYER 6
#define RED_FLAG_RETURNED 7
#define BLUE_FLAG_RETURNED 8
#define RED_FLAG_LOST_HUD 9
#define BLUE_FLAG_LOST_HUD 10



char *sBonusStrings[] = 
{
	"",
	"\\w stole the \\rRED\\w Flag!",
	"\\w stole the \\bBLUE\\w Flag!",
	"\\w captured the \\rRED\\w Flag",
	"\\w captured the \\bBLUE\\w Flag",
	"\\w returned the \\rRED\\w Flag",
	"\\w returned the \\bBLUE\\w Flag", 
	"\\wThe \\rRED\\w Flag has Returned",
	"\\wThe \\bBLUE\\w Flag has Returned",
	"\\w lost the \\rRED\\w Flag!",
	"\\w lost the \\bBLUE\\w Flag!",
};

DECLARE_MESSAGE(m_Bonus, Bonus)

struct bonus_info_t
{
	int iSlot;
	int iType;
	bool bActive;
	float flBonusTime;
	char sPlayerName[64];
};

bonus_info_t  g_PlayerBonus[MAX_BONUS+1]; 

int CHudBonus::Init(void)
{    
	HOOK_MESSAGE( Bonus );

    m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	Reset();
    
    return 1;
}; 

int CHudBonus::VidInit(void)
{
    return 1;
}

void CHudBonus :: Reset( void )
{
     m_iFlags  |= HUD_ACTIVE;

	 for ( int reset = 0; reset < MAX_BONUS; reset++)
	 {
		 g_PlayerBonus[ reset ].flBonusTime = 0.0;
		 g_PlayerBonus[ reset ].iSlot = 0;
		 g_PlayerBonus[ reset ].iType = 0;
		 g_PlayerBonus[ reset ].bActive = false;
		 m_bUsedSlot[ reset ] = false;
		 strcpy ( g_PlayerBonus[ reset ].sPlayerName, "" );
	 }
}

int CHudBonus ::Draw(float flTime )
{
	for (int index = 1; index < MAX_BONUS + 1; index++)
	{
		//Just activated
		if ( g_PlayerBonus[ index ].bActive && !g_PlayerBonus[ index ].flBonusTime )
		{
			g_PlayerBonus[ index ].flBonusTime = flTime + 5.0;

			for ( int i = 1; i < MAX_BONUS + 1; i++ )
			{
				if ( m_bUsedSlot[ i ] == false ) //found one thats not used
				{
					m_bUsedSlot[ i ] = true; //use it!
					g_PlayerBonus[ index ].iSlot = i;
					break;
				}
			}
		}

		if ( g_PlayerBonus[ index ].flBonusTime > flTime )
		{
			int YPos;
			int iMod = gHUD.ReturnStringPixelLength( "\\w\\r\\w" );
			
			YPos = ( ( ScreenHeight - gHUD.m_iFontHeight ) - ( gHUD.m_iFontHeight / 2 ) + 3 ) - ( 30 * g_PlayerBonus[ index ].iSlot );
			
			int XPos = 75;
			
			char szText[256];

			strcpy ( szText, g_PlayerBonus[ index ].sPlayerName );
			strcat ( szText, sBonusStrings[ g_PlayerBonus[ index ].iType ] );
		
			if ( gHUD.m_FlagStat.iBlueTeamScore >= 10 )
				gHUD.DrawHudStringCTF( XPos + 20, YPos, 640, szText, 255, 255, 255 );
			else
				gHUD.DrawHudStringCTF( XPos , YPos, 320, szText, 255, 255, 255 );

		}

		if ( g_PlayerBonus[ index ].flBonusTime < flTime )
		{
			g_PlayerBonus[ index ].bActive = false;
			m_bUsedSlot[ g_PlayerBonus[ index ].iSlot ] = false;
			g_PlayerBonus[ index ].iSlot = 0;
			strcpy ( g_PlayerBonus[ index ].sPlayerName, "" );
		}
	}

	return 1;
}
 
int CHudBonus::MsgFunc_Bonus(const char *pszName, int iSize, void *pbuf)
{
    BEGIN_READ( pbuf, iSize ); 

	for ( int index = 1; index < MAX_BONUS + 1; index++)
	{
		//Find wich one is not used
		if ( g_PlayerBonus[ index ].bActive == false )
			break; //Not using this one?, then we shall use this.
	}
    
	g_PlayerBonus[ index ].bActive = true;
	g_PlayerBonus[ index ].flBonusTime = 0.0;
	g_PlayerBonus[ index ].iType = READ_BYTE();
	strcpy ( g_PlayerBonus[ index ].sPlayerName, READ_STRING() );

	switch ( g_PlayerBonus[ index ].iType )
	{
		case RED_FLAG_STOLEN:
		case BLUE_FLAG_STOLEN:
			PlaySound( "ctf/flagtk.wav", 1 );
			break;
		case RED_FLAG_CAPTURED:
		case BLUE_FLAG_CAPTURED:
			PlaySound( "ctf/flagcap.wav", 1 );
			break;
		case RED_FLAG_RETURNED_PLAYER:
		case BLUE_FLAG_RETURNED_PLAYER:
		case RED_FLAG_RETURNED:
		case BLUE_FLAG_RETURNED:
			PlaySound( "ctf/flagret.wav", 1 );
			break;
	}
	
	return 1;
}

#endif