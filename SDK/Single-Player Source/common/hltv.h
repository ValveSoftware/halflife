//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// hltv.h
// all shared consts between server, clients and proxy

#ifndef HLTV_H
#define HLTV_H

#define TYPE_CLIENT				0	// client is a normal HL client (default)
#define TYPE_PROXY				1	// client is another proxy
#define TYPE_COMMENTATOR		3	// client is a commentator
#define TYPE_DEMO				4	// client is a demo file
// sub commands of svc_hltv:
#define HLTV_ACTIVE				0	// tells client that he's an spectator and will get director commands
#define HLTV_STATUS				1	// send status infos about proxy 
#define HLTV_LISTEN				2	// tell client to listen to a multicast stream

// sub commands of svc_director:
#define DRC_CMD_NONE				0	// NULL director command
#define DRC_CMD_START				1	// start director mode
#define DRC_CMD_EVENT				2	// informs about director command
#define DRC_CMD_MODE				3	// switches camera modes
#define DRC_CMD_CAMERA				4	// sets camera registers
#define DRC_CMD_TIMESCALE			5	// sets time scale
#define DRC_CMD_MESSAGE				6	// send HUD centerprint
#define DRC_CMD_SOUND				7	// plays a particular sound
#define DRC_CMD_STATUS				8	// status info about broadcast
#define DRC_CMD_BANNER				9	// banner file name for HLTV gui
#define	DRC_CMD_FADE				10	// send screen fade command
#define DRC_CMD_SHAKE				11	// send screen shake command
#define DRC_CMD_STUFFTEXT			12	// like the normal svc_stufftext but as director command

#define DRC_CMD_LAST				12



// HLTV_EVENT event flags
#define DRC_FLAG_PRIO_MASK		0x0F	// priorities between 0 and 15 (15 most important)
#define DRC_FLAG_SIDE			(1<<4)	// 
#define DRC_FLAG_DRAMATIC		(1<<5)	// is a dramatic scene
#define DRC_FLAG_SLOWMOTION		(1<<6)  // would look good in SloMo
#define DRC_FLAG_FACEPLAYER		(1<<7)  // player is doning something (reload/defuse bomb etc)
#define DRC_FLAG_INTRO			(1<<8)	// is a introduction scene
#define DRC_FLAG_FINAL			(1<<9)	// is a final scene
#define DRC_FLAG_NO_RANDOM		(1<<10)	// don't randomize event data


#define MAX_DIRECTOR_CMD_PARAMETERS		4
#define MAX_DIRECTOR_CMD_STRING			128


#endif // HLTV_H
