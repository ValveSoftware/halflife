//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "hud.h"
#include "cl_util.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "vgui_TeamFortressViewport.h"

// these are included for the math functions
#include "com_model.h"
#include "studio_util.h"

#pragma warning(disable: 4244)

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CHudOverview::Init()
{
	gHUD.AddHudElem(this);

	m_iFlags |= HUD_ACTIVE;

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: Loads new icons
//-----------------------------------------------------------------------------
int CHudOverview::VidInit()
{
	m_hsprPlayer = gEngfuncs.pfnSPR_Load("sprites/ring.spr");
	m_hsprViewcone = gEngfuncs.pfnSPR_Load("sprites/camera.spr");

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flTime - 
//			intermission - 
//-----------------------------------------------------------------------------
int CHudOverview::Draw(float flTime)
{
	// only draw in overview mode
	if (!gEngfuncs.Overview_GetOverviewState())
		return 1;

	// make sure we have player info
	gViewPort->GetAllPlayersInfo();

	// calculate player size on the overview
	int x1, y1, x2, y2;
	float v0[3]={0,0,0}, v1[3]={64,64,0};
	gEngfuncs.Overview_WorldToScreen(v0, &x1, &y1);
	gEngfuncs.Overview_WorldToScreen(v1, &x2, &y2);
	float scale = abs(x2 - x1);

	// loop through all the players and draw them on the map
	for (int i = 1; i < MAX_PLAYERS; i++)
	{
		cl_entity_t *pl = gEngfuncs.GetEntityByIndex(i);

		if (pl && pl->player && pl->curstate.health > 0 && pl->curstate.solid != SOLID_NOT)
		{
			int x, y, z = 0;
			float v[3]={pl->origin[0], pl->origin[1], 0};
			gEngfuncs.Overview_WorldToScreen(v, &x, &y);

			// hack in some team colors
			float r, g, bc;
			if (g_PlayerExtraInfo[i].teamnumber == 1)
			{
				r = 0.0f; g = 0.0f; bc = 1.0f;
			}
			else if (g_PlayerExtraInfo[i].teamnumber == 2)
			{
				r = 1.0f; g = 0.0f; bc = 0.0f;
			}
			else
			{
				// just use the default orange color if the team isn't set
				r = 1.0f; g = 0.7f; bc = 0.0f;
			}

			// set the current texture
			gEngfuncs.pTriAPI->SpriteTexture((struct model_s *)gEngfuncs.GetSpritePointer(m_hsprPlayer), 0);

			// additive render mode
			gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);

			// no culling
			gEngfuncs.pTriAPI->CullFace(TRI_NONE);

			// draw a square
			gEngfuncs.pTriAPI->Begin(TRI_QUADS);

			// set the color to be that of the team
			gEngfuncs.pTriAPI->Color4f(r, g, bc, 1.0f);

			// calculate rotational matrix
			vec3_t a, b, angles;
			float rmatrix[3][4];	// transformation matrix
			VectorCopy(pl->angles, angles);
			angles[0] = 0.0f;
			angles[1] += 90.f;
			angles[1] = -angles[1];
			angles[2] = 0.0f;
			AngleMatrix(angles, rmatrix);
			a[2] = 0;

			a[0] = -scale; a[1] = -scale;
			VectorTransform(a, rmatrix , b );
			gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
			gEngfuncs.pTriAPI->Vertex3f(x + b[0], y + b[1], z);

			a[0]=-scale; a[1] = scale;
			VectorTransform(a, rmatrix , b );
			gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
			gEngfuncs.pTriAPI->Vertex3f (x + b[0], y + b[1], z);
			
			a[0]=scale; a[1] = scale;
			VectorTransform(a, rmatrix , b );
			gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
			gEngfuncs.pTriAPI->Vertex3f (x + b[0], y + b[1], z);

			a[0]=scale; a[1] = -scale;
			VectorTransform(a, rmatrix , b );
			gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
			gEngfuncs.pTriAPI->Vertex3f (x + b[0], y + b[1], z);

			// finish up
			gEngfuncs.pTriAPI->End();
			gEngfuncs.pTriAPI->RenderMode( kRenderNormal );

			// draw the players name and health underneath
			char string[256];
			sprintf(string, "%s (%i%%)", g_PlayerInfoList[i].name, pl->curstate.health);
			DrawConsoleString(x, y + (1.1 * scale), string);
		}
	}

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: called every time a server is connected to
//-----------------------------------------------------------------------------
void CHudOverview::InitHUDData()
{
//  this block would force the spectator view to be on
//	gEngfuncs.Overview_SetDrawOverview( 1 );
//	gEngfuncs.Overview_SetDrawInset( 0 );
}

