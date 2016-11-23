/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#ifndef __TF_DEFS_H
#define __TF_DEFS_H

//===========================================================================
// OLD OPTIONS.QC
//===========================================================================
#define DEFAULT_AUTOZOOM		 FALSE
#define WEINER_SNIPER                           // autoaiming for sniper rifle
#define FLAME_MAXWORLDNUM        20             // maximum number of flames in the world. DO NOT PUT BELOW 20.

//#define MAX_WORLD_PIPEBOMBS      15             // This is divided between teams - this is the most you should have on a net server
#define MAX_PLAYER_PIPEBOMBS	 8				// maximum number of pipebombs any 1 player can have active
#define MAX_PLAYER_AMMOBOXES  3				// maximum number of ammoboxes any 1 player can have active

//#define MAX_WORLD_FLARES         9              // This is the total number of flares allowed in the world at one time
//#define MAX_WORLD_AMMOBOXES      20             // This is divided between teams - this is the most you should have on a net server
#define GR_TYPE_MIRV_NO          4              // Number of Mirvs a Mirv Grenade breaks into
#define GR_TYPE_NAPALM_NO        8              // Number of flames napalm grenade breaks into (unused if net server)
#define MEDIKIT_IS_BIOWEAPON					// Medikit acts as a bioweapon against enemies

#define TEAM_HELP_RATE   60     // used only if teamplay bit 64 (help team with lower score) is set.  
								// 60 is a mild setting, and won't make too much difference
								// increasing it _decreases_ the amount of help the losing team gets
								// Minimum setting is 1, which would really help the losing team

#define DISPLAY_CLASS_HELP			TRUE            // Change this to #OFF if you don't want the class help to 
													// appear whenever a player connects
#define NEVER_TEAMFRAGS				FALSE			// teamfrags options always off 
#define ALWAYS_TEAMFRAGS			FALSE			// teamfrags options always on 
#define CHECK_SPEEDS				TRUE            // makes sure players aren't moving too fast
#define SNIPER_RIFLE_RELOAD_TIME    1.5				// seconds

#define MAPBRIEFING_MAXTEXTLENGTH	512
#define PLAYER_PUSH_VELOCITY			 50			// Players push teammates if they're moving under this speed

// Debug Options
//#define MAP_DEBUG                     // Debug for Map code. I suggest running in a hi-res
										// mode and/or piping the output from the server to a file.
#ifdef MAP_DEBUG
	#define MDEBUG(x) x
#else
	#define MDEBUG(x)
#endif
//#define VERBOSE                       // Verbose Debugging on/off

//===========================================================================
// OLD QUAKE Defs
//===========================================================================
// items
#define IT_AXE					4096 
#define IT_SHOTGUN				1 
#define IT_SUPER_SHOTGUN		2 
#define IT_NAILGUN				4 
#define IT_SUPER_NAILGUN		8 
#define IT_GRENADE_LAUNCHER		16 
#define IT_ROCKET_LAUNCHER		32 
#define IT_LIGHTNING			64 
#define IT_EXTRA_WEAPON			128 

#define IT_SHELLS				256 
#define IT_NAILS				512 
#define IT_ROCKETS				1024 
#define IT_CELLS				2048 

#define IT_ARMOR1				8192 
#define IT_ARMOR2				16384 
#define IT_ARMOR3				32768 
#define IT_SUPERHEALTH			65536 

#define IT_KEY1					131072 
#define IT_KEY2					262144 

#define IT_INVISIBILITY			524288 
#define IT_INVULNERABILITY		1048576 
#define IT_SUIT					2097152
#define IT_QUAD					4194304 
#define IT_HOOK					8388608

#define IT_KEY3					16777216	// Stomp invisibility
#define IT_KEY4					33554432	// Stomp invulnerability

//===========================================================================
// TEAMFORTRESS Defs
//===========================================================================
// TeamFortress State Flags
#define TFSTATE_GRENPRIMED		1 	// Whether the player has a primed grenade
#define TFSTATE_RELOADING		2 	// Whether the player is reloading
#define TFSTATE_ALTKILL			4  	// #TRUE if killed with a weapon not in self.weapon: NOT USED ANYMORE
#define TFSTATE_RANDOMPC		8   // Whether Playerclass is random, new one each respawn
#define TFSTATE_INFECTED		16 	// set when player is infected by the bioweapon
#define TFSTATE_INVINCIBLE		32 	// Player has permanent Invincibility (Usually by GoalItem)
#define TFSTATE_INVISIBLE		64 	// Player has permanent Invisibility (Usually by GoalItem)
#define TFSTATE_QUAD			128 // Player has permanent Quad Damage (Usually by GoalItem)
#define TFSTATE_RADSUIT			256 // Player has permanent Radsuit (Usually by GoalItem)
#define TFSTATE_BURNING			512 // Is on fire
#define TFSTATE_GRENTHROWING	1024  // is throwing a grenade
#define TFSTATE_AIMING			2048  // is using the laser sight
#define TFSTATE_ZOOMOFF			4096  // doesn't want the FOV changed when zooming
#define TFSTATE_RESPAWN_READY	8192  // is waiting for respawn, and has pressed fire
#define TFSTATE_HALLUCINATING  16384  // set when player is hallucinating
#define TFSTATE_TRANQUILISED   32768  // set when player is tranquilised
#define TFSTATE_CANT_MOVE	   65536  // set when player is setting a detpack
#define TFSTATE_RESET_FLAMETIME 131072 // set when the player has to have his flames increased in health

// Defines used by TF_T_Damage (see combat.qc)
#define TF_TD_IGNOREARMOUR	1  // Bypasses the armour of the target
#define TF_TD_NOTTEAM		2  // Doesn't damage a team member (indicates direct fire weapon)
#define TF_TD_NOTSELF		4  // Doesn't damage self

#define TF_TD_OTHER			0  // Ignore armorclass
#define TF_TD_SHOT			1  // Bullet damage
#define TF_TD_NAIL			2  // Nail damage
#define TF_TD_EXPLOSION		4  // Explosion damage
#define TF_TD_ELECTRICITY	8  // Electric damage
#define TF_TD_FIRE			16  // Fire damage
#define TF_TD_NOSOUND		256 // Special damage. Makes no sound/painframe, etc

/*==================================================*/
/* Toggleable Game Settings							*/
/*==================================================*/
#define TF_RESPAWNDELAY1	5 	// seconds of waiting before player can respawn
#define TF_RESPAWNDELAY2	10 	// seconds of waiting before player can respawn
#define TF_RESPAWNDELAY3	20 	// seconds of waiting before player can respawn

#define TEAMPLAY_NORMAL			 1			
#define TEAMPLAY_HALFDIRECT		 2
#define TEAMPLAY_NODIRECT		 4
#define TEAMPLAY_HALFEXPLOSIVE	 8
#define TEAMPLAY_NOEXPLOSIVE	 16
#define TEAMPLAY_LESSPLAYERSHELP 32
#define TEAMPLAY_LESSSCOREHELP	 64
#define TEAMPLAY_HALFDIRECTARMOR 128
#define TEAMPLAY_NODIRECTARMOR 	 256
#define TEAMPLAY_HALFEXPARMOR	 512
#define TEAMPLAY_NOEXPARMOR		 1024
#define TEAMPLAY_HALFDIRMIRROR	 2048
#define TEAMPLAY_FULLDIRMIRROR	 4096
#define TEAMPLAY_HALFEXPMIRROR	 8192
#define TEAMPLAY_FULLEXPMIRROR	 16384

#define TEAMPLAY_TEAMDAMAGE		(TEAMPLAY_NODIRECT | TEAMPLAY_HALFDIRECT | TEAMPLAY_HALFEXPLOSIVE | TEAMPLAY_NOEXPLOSIVE)
// FortressMap stuff
#define TEAM1_CIVILIANS 1	
#define TEAM2_CIVILIANS 2
#define TEAM3_CIVILIANS 4	
#define TEAM4_CIVILIANS 8	

// Defines for the playerclass
#define PC_UNDEFINED	0 

#define PC_SCOUT		1 
#define PC_SNIPER		2 
#define PC_SOLDIER		3 
#define PC_DEMOMAN		4 
#define PC_MEDIC		5 
#define PC_HVYWEAP		6 
#define PC_PYRO			7
#define PC_SPY			8
#define PC_ENGINEER		9

// Insert new class definitions here

// PC_RANDOM _MUST_ be the third last class
#define PC_RANDOM		10 		// Random playerclass
#define PC_CIVILIAN		11		// Civilians are a special class. They cannot
								// be chosen by players, only enforced by maps
#define PC_LASTCLASS	12 		// Use this as the high-boundary for any loops
								// through the playerclass.

// These are just for the scanner
#define SCAN_SENTRY		13
#define SCAN_GOALITEM	14

// Values returned by CheckArea
enum
{
	CAREA_CLEAR,
	CAREA_BLOCKED,
	CAREA_NOBUILD
};

/*==================================================*/
/* Impulse Defines		                        	*/
/*==================================================*/
// Alias check to see whether they already have the aliases
#define TF_ALIAS_CHECK		13 

// CTF Support Impulses
#define HOOK_IMP1		22
#define FLAG_INFO		23
#define HOOK_IMP2		39

// Axe
#define AXE_IMP			40

// Camera Impulse
#define TF_CAM_TARGET			50
#define TF_CAM_ZOOM				51
#define TF_CAM_ANGLE			52
#define TF_CAM_VEC				53
#define TF_CAM_PROJECTILE		54
#define TF_CAM_PROJECTILE_Z		55
#define TF_CAM_REVANGLE			56
#define TF_CAM_OFFSET			57
#define TF_CAM_DROP				58	
#define TF_CAM_FADETOBLACK		59
#define TF_CAM_FADEFROMBLACK	60
#define TF_CAM_FADETOWHITE		61
#define TF_CAM_FADEFROMWHITE	62

// Last Weapon impulse
#define TF_LAST_WEAPON			69

// Status Bar Resolution Settings.  Same as CTF to maintain ease of use.
#define TF_STATUSBAR_RES_START	71
#define TF_STATUSBAR_RES_END	81

// Clan Messages
#define TF_MESSAGE_1			82
#define TF_MESSAGE_2			83
#define TF_MESSAGE_3			84
#define TF_MESSAGE_4			85
#define TF_MESSAGE_5			86

#define TF_CHANGE_CLASS			99	// Bring up the Class Change menu

// Added to PC_??? to get impulse to use if this clashes with your 
// own impulses, just change this value, not the PC_??
#define TF_CHANGEPC			100 
// The next few impulses are all the class selections
//PC_SCOUT		101 
//PC_SNIPER		102 
//PC_SOLDIER	103 
//PC_DEMOMAN	104 
//PC_MEDIC		105 
//PC_HVYWEAP	106 
//PC_PYRO		107 
//PC_RANDOM		108
//PC_CIVILIAN	109  // Cannot be used
//PC_SPY		110
//PC_ENGINEER	111

// Help impulses
#define TF_DISPLAYLOCATION  118
#define TF_STATUS_QUERY		119

#define TF_HELP_MAP			131

// Information impulses
#define TF_INVENTORY		135
#define TF_SHOWTF			136 
#define TF_SHOWLEGALCLASSES	137

// Team Impulses
#define TF_TEAM_1			140   // Join Team 1
#define TF_TEAM_2			141   // Join Team 2
#define TF_TEAM_3			142   // Join Team 3
#define TF_TEAM_4			143   // Join Team 4
#define TF_TEAM_CLASSES		144   // Impulse to display team classes
#define TF_TEAM_SCORES		145   // Impulse to display team scores
#define TF_TEAM_LIST		146   // Impulse to display the players in each team.

// Grenade Impulses
#define TF_GRENADE_1		150   // Prime grenade type 1
#define TF_GRENADE_2		151   // Prime grenade type 2
#define TF_GRENADE_T		152   // Throw primed grenade

// Impulses for new items
//#define TF_SCAN				159		// Scanner Pre-Impulse
#define TF_AUTO_SCAN		159		// Scanner On/Off
#define TF_SCAN_ENEMY		160		// Impulses to toggle scanning of enemies
#define TF_SCAN_FRIENDLY	161		// Impulses to toggle scanning of friendlies 
//#define TF_SCAN_10			162		// Scan using 10 enery (1 cell)
#define TF_SCAN_SOUND		162		// Scanner sounds on/off
#define TF_SCAN_30			163		// Scan using 30 energy (2 cells)
#define TF_SCAN_100			164		// Scan using 100 energy (5 cells)
#define TF_DETPACK_5		165		// Detpack set to 5 seconds
#define TF_DETPACK_20		166		// Detpack set to 20 seconds
#define TF_DETPACK_50		167		// Detpack set to 50 seconds
#define TF_DETPACK			168		// Detpack Pre-Impulse
#define TF_DETPACK_STOP		169		// Impulse to stop setting detpack
#define TF_PB_DETONATE		170		// Detonate Pipebombs

// Special skill
#define TF_SPECIAL_SKILL	171

// Ammo Drop impulse
#define TF_DROP_AMMO        172

// Reload impulse
#define TF_RELOAD			173

// auto-zoom toggle
#define TF_AUTOZOOM			174

// drop/pass commands
#define TF_DROPKEY			175

// Select Medikit		
#define TF_MEDIKIT			176

// Spy Impulses
#define TF_SPY_SPY			177		// On net, go invisible, on LAN, change skin/color
#define TF_SPY_DIE			178		// Feign Death

// Engineer Impulses
#define TF_ENGINEER_BUILD	179
#define TF_ENGINEER_SANDBAG	180

// Medic
#define TF_MEDIC_HELPME		181

// Status bar
#define TF_STATUSBAR_ON		182
#define TF_STATUSBAR_OFF	183

// Discard impulse
#define TF_DISCARD 	  		184

// ID Player impulse
#define TF_ID	 	  		185

// Clan Battle impulses
#define TF_SHOWIDS			186

// More Engineer Impulses
#define TF_ENGINEER_DETDISP 187
#define TF_ENGINEER_DETSENT 188

// Admin Commands
#define TF_ADMIN_DEAL_CYCLE		189
#define TF_ADMIN_KICK			190
#define TF_ADMIN_BAN			191
#define TF_ADMIN_COUNTPLAYERS	192
#define TF_ADMIN_CEASEFIRE		193

// Drop Goal Items
#define TF_DROPGOALITEMS 		194

// More Admin Commands
#define TF_ADMIN_NEXT			195

// More Engineer Impulses
#define TF_ENGINEER_DETEXIT 	196
#define TF_ENGINEER_DETENTRANCE	197

// Yet MORE Admin Commands
#define TF_ADMIN_LISTIPS		198

// Silent Spy Feign
#define TF_SPY_SILENTDIE		199


/*==================================================*/
/*	Colors											*/
/*==================================================*/
#define TEAM1_COLOR		150
#define TEAM2_COLOR		250
#define TEAM3_COLOR		45
#define TEAM4_COLOR		100

/*==================================================*/
/* Defines for the ENGINEER's Building ability		*/
/*==================================================*/
// Ammo costs
#define AMMO_COST_SHELLS		2		// Metal needed to make 1 shell
#define AMMO_COST_NAILS			1
#define AMMO_COST_ROCKETS		2
#define AMMO_COST_CELLS			2

// Building types
#define BUILD_DISPENSER				1
#define BUILD_SENTRYGUN				2
#define BUILD_MORTAR				3
#define BUILD_TELEPORTER_ENTRANCE	4
#define BUILD_TELEPORTER_EXIT		5

// Building metal costs
#define BUILD_COST_DISPENSER	100		// Metal needed to built 
#define BUILD_COST_SENTRYGUN	130		
#define BUILD_COST_MORTAR		150		
#define BUILD_COST_TELEPORTER	125		

#define BUILD_COST_SANDBAG		20		// Built with a separate alias

// Building times
#define BUILD_TIME_DISPENSER	2		// seconds to build
#define BUILD_TIME_SENTRYGUN	5		
#define BUILD_TIME_MORTAR		5		
#define BUILD_TIME_TELEPORTER	4		

// Building health levels
#define BUILD_HEALTH_DISPENSER	150		// Health of the building
#define BUILD_HEALTH_SENTRYGUN	150		
#define BUILD_HEALTH_MORTAR		200		
#define BUILD_HEALTH_TELEPORTER 80

// Dispenser's maximum carrying capability
#define BUILD_DISPENSER_MAX_SHELLS  400
#define BUILD_DISPENSER_MAX_NAILS   600
#define BUILD_DISPENSER_MAX_ROCKETS 300
#define BUILD_DISPENSER_MAX_CELLS   400
#define BUILD_DISPENSER_MAX_ARMOR   500

// Build state sent down to client
#define BS_BUILDING			(1<<0)
#define BS_HAS_DISPENSER	(1<<1)
#define BS_HAS_SENTRYGUN	(1<<2)
#define BS_CANB_DISPENSER	(1<<3)
#define BS_CANB_SENTRYGUN	(1<<4)
/*==================================================*/
/* Ammo quantities for dropping & dispenser use		*/
/*==================================================*/
#define DROP_SHELLS   20
#define DROP_NAILS    20
#define DROP_ROCKETS  10
#define DROP_CELLS    10
#define DROP_ARMOR	  40

/*==================================================*/
/* Team Defines				            			*/
/*==================================================*/
#define TM_MAX_NO	4 			// Max number of teams. Simply changing this value isn't enough.
								// A new global to hold new team colors is needed, and more flags
								// in the spawnpoint spawnflags may need to be used.
								// Basically, don't change this unless you know what you're doing :)

/*==================================================*/
/* New Weapon Defines		                        */
/*==================================================*/
#define WEAP_HOOK				1
#define WEAP_BIOWEAPON			2
#define WEAP_MEDIKIT			4
#define WEAP_SPANNER			8
#define WEAP_AXE				16
#define WEAP_SNIPER_RIFLE		32
#define WEAP_AUTO_RIFLE			64
#define WEAP_SHOTGUN			128
#define WEAP_SUPER_SHOTGUN		256
#define WEAP_NAILGUN			512
#define WEAP_SUPER_NAILGUN		1024
#define WEAP_GRENADE_LAUNCHER	2048
#define WEAP_FLAMETHROWER		4096
#define WEAP_ROCKET_LAUNCHER	8192
#define WEAP_INCENDIARY			16384
#define WEAP_ASSAULT_CANNON		32768
#define WEAP_LIGHTNING			65536
#define WEAP_DETPACK			131072
#define WEAP_TRANQ				262144
#define WEAP_LASER				524288
// still room for 12 more weapons
// but we can remove some by giving the weapons
// a weapon mode (like the rifle)

// HL-compatible weapon numbers
#define WEAPON_HOOK				1
#define WEAPON_BIOWEAPON		(WEAPON_HOOK+1)
#define WEAPON_MEDIKIT			(WEAPON_HOOK+2)
#define WEAPON_SPANNER			(WEAPON_HOOK+3)
#define WEAPON_AXE				(WEAPON_HOOK+4)
#define WEAPON_SNIPER_RIFLE		(WEAPON_HOOK+5)
#define WEAPON_AUTO_RIFLE		(WEAPON_HOOK+6)
#define WEAPON_TF_SHOTGUN		(WEAPON_HOOK+7)
#define WEAPON_SUPER_SHOTGUN	(WEAPON_HOOK+8)
#define WEAPON_NAILGUN			(WEAPON_HOOK+9)
#define WEAPON_SUPER_NAILGUN	(WEAPON_HOOK+10)
#define WEAPON_GRENADE_LAUNCHER	(WEAPON_HOOK+11)
#define WEAPON_FLAMETHROWER		(WEAPON_HOOK+12)
#define WEAPON_ROCKET_LAUNCHER	(WEAPON_HOOK+13)
#define WEAPON_INCENDIARY		(WEAPON_HOOK+14)
#define WEAPON_ASSAULT_CANNON	(WEAPON_HOOK+16)
#define WEAPON_LIGHTNING		(WEAPON_HOOK+17)
#define WEAPON_DETPACK			(WEAPON_HOOK+18)
#define WEAPON_TRANQ			(WEAPON_HOOK+19)
#define WEAPON_LASER			(WEAPON_HOOK+20)
#define WEAPON_PIPEBOMB_LAUNCHER (WEAPON_HOOK+21)
#define WEAPON_KNIFE			(WEAPON_HOOK+22)
#define WEAPON_BENCHMARK		(WEAPON_HOOK+23)

/*==================================================*/
/* New Weapon Related Defines		                */
/*==================================================*/
// shots per reload 
#define RE_SHOTGUN			8
#define RE_SUPER_SHOTGUN	16 // 8 shots
#define RE_GRENADE_LAUNCHER	6 
#define RE_ROCKET_LAUNCHER	4 

// reload times
#define RE_SHOTGUN_TIME				2 
#define RE_SUPER_SHOTGUN_TIME		3 
#define RE_GRENADE_LAUNCHER_TIME	4 
#define RE_ROCKET_LAUNCHER_TIME		5 

// Maximum velocity you can move and fire the Sniper Rifle
#define WEAP_SNIPER_RIFLE_MAX_MOVE	50 

// Medikit
#define WEAP_MEDIKIT_HEAL	200  // Amount medikit heals per hit
#define WEAP_MEDIKIT_OVERHEAL 50 // Amount of superhealth over max_health the medikit will dispense

// Spanner
#define WEAP_SPANNER_REPAIR 10

// Detpack
#define WEAP_DETPACK_DISARMTIME		3   	// Time it takes to disarm a Detpack
#define WEAP_DETPACK_SETTIME		3   	// Time it takes to set a Detpack
#define WEAP_DETPACK_SIZE			700	 	// Explosion Size
#define WEAP_DETPACK_GOAL_SIZE		1500 	// Explosion Size for goal triggering
#define WEAP_DETPACK_BITS_NO		12  	// Bits that detpack explodes into

// Tranquiliser Gun
#define TRANQ_TIME			15

// Grenades
#define GR_PRIMETIME		3
#define GR_CALTROP_PRIME	0.5
#define GR_TYPE_NONE		0 
#define GR_TYPE_NORMAL		1 
#define GR_TYPE_CONCUSSION	2 
#define GR_TYPE_NAIL		3 
#define GR_TYPE_MIRV		4 
#define GR_TYPE_NAPALM		5 
//#define GR_TYPE_FLARE		6 
#define GR_TYPE_GAS			7
#define GR_TYPE_EMP			8
#define GR_TYPE_CALTROP		9
//#define GR_TYPE_FLASH		10

// Defines for WeaponMode
#define GL_NORMAL	0 
#define GL_PIPEBOMB	1

// Defines for OLD Concussion Grenade
#define GR_OLD_CONCUSS_TIME		5 
#define GR_OLD_CONCUSS_DEC		20 

// Defines for Concussion Grenade
#define GR_CONCUSS_TIME		0.25 
#define GR_CONCUSS_DEC		10
#define MEDIUM_PING			150
#define HIGH_PING			200

// Defines for the Gas Grenade
#define GR_HALLU_TIME		0.3
#define GR_OLD_HALLU_TIME	0.5
#define GR_HALLU_DEC		2.5

// Defines for the BioInfection
#define BIO_JUMP_RADIUS 128		// The distance the bioinfection can jump between players

/*==================================================*/
/* New Items			                        	*/
/*==================================================*/
#define NIT_SCANNER				1 

#define NIT_SILVER_DOOR_OPENED 	#IT_KEY1	// 131072 
#define NIT_GOLD_DOOR_OPENED 	#IT_KEY2	// 262144

/*==================================================*/
/* New Item Flags		                        	*/
/*==================================================*/
#define NIT_SCANNER_ENEMY		1 	// Detect enemies
#define NIT_SCANNER_FRIENDLY	2 	// Detect friendlies (team members)
#define NIT_SCANNER_SOUND		4 	// Motion detection. Only report moving entities.

/*==================================================*/
/* New Item Related Defines		                    */
/*==================================================*/
#define NIT_SCANNER_POWER			25	// The amount of power spent on a scan with the scanner
										// is multiplied by this to get the scanrange.
#define NIT_SCANNER_MAXCELL			50 	// The maximum number of cells than can be used in one scan
#define NIT_SCANNER_MIN_MOVEMENT	50 	// The minimum velocity an entity must have to be detected
										// by scanners that only detect movement

/*==================================================*/
/* Variables used for New Weapons and Reloading     */
/*==================================================*/
// Armor Classes : Bitfields. Use the "armorclass" of armor for the Armor Type.
#define AT_SAVESHOT			1   // Kevlar  	 : Reduces bullet damage by 15%
#define AT_SAVENAIL			2   // Wood :) 	 : Reduces nail damage by 15%
#define AT_SAVEEXPLOSION	4  	// Blast   	 : Reduces explosion damage by 15%
#define AT_SAVEELECTRICITY	8 	// Shock	 : Reduces electricity damage by 15%
#define AT_SAVEFIRE			16 	// Asbestos	 : Reduces fire damage by 15%

/*==========================================================================*/
/* TEAMFORTRESS CLASS DETAILS												*/
/*==========================================================================*/
// Class Details for SCOUT
#define PC_SCOUT_SKIN				4 		// Skin for this class when Classkin is on.
#define PC_SCOUT_MAXHEALTH			75 		// Maximum Health Level
#define PC_SCOUT_MAXSPEED			400		// Maximum movement speed
#define PC_SCOUT_MAXSTRAFESPEED		400		// Maximum strafing movement speed
#define PC_SCOUT_MAXARMOR			50 		// Maximum Armor Level, of any armor class
#define PC_SCOUT_INITARMOR			25 		// Armor level when respawned
#define PC_SCOUT_MAXARMORTYPE		0.3		// Maximum level of Armor absorption
#define PC_SCOUT_INITARMORTYPE		0.3		// Absorption Level of armor when respawned
#define PC_SCOUT_ARMORCLASSES		3 		// #AT_SAVESHOT | #AT_SAVENAIL   		<-Armor Classes allowed for this class
#define PC_SCOUT_INITARMORCLASS		0 		// Armorclass worn when respawned
#define PC_SCOUT_WEAPONS			WEAP_AXE | WEAP_SHOTGUN | WEAP_NAILGUN
#define PC_SCOUT_MAXAMMO_SHOT		50 		// Maximum amount of shot ammo this class can carry
#define PC_SCOUT_MAXAMMO_NAIL		200		// Maximum amount of nail ammo this class can carry
#define PC_SCOUT_MAXAMMO_CELL		100		// Maximum amount of cell ammo this class can carry
#define PC_SCOUT_MAXAMMO_ROCKET		25 		// Maximum amount of rocket ammo this class can carry
#define PC_SCOUT_INITAMMO_SHOT		25 		// Amount of shot ammo this class has when respawned
#define PC_SCOUT_INITAMMO_NAIL		100		// Amount of nail ammo this class has when respawned
#define PC_SCOUT_INITAMMO_CELL		50 		// Amount of cell ammo this class has when respawned
#define PC_SCOUT_INITAMMO_ROCKET	0 		// Amount of rocket ammo this class has when respawned
#define PC_SCOUT_GRENADE_TYPE_1		GR_TYPE_CALTROP			 //    <- 1st Type of Grenade this class has
#define PC_SCOUT_GRENADE_TYPE_2		GR_TYPE_CONCUSSION      //    <- 2nd Type of Grenade this class has
#define PC_SCOUT_GRENADE_INIT_1		2 		// Number of grenades of Type 1 this class has when respawned
#define PC_SCOUT_GRENADE_INIT_2		3 		// Number of grenades of Type 2 this class has when respawned
#define PC_SCOUT_TF_ITEMS			NIT_SCANNER  // <- TeamFortress Items this class has

#define PC_SCOUT_MOTION_MIN_I		0.5 	// < Short range
#define PC_SCOUT_MOTION_MIN_MOVE	50 		// Minimum vlen of player velocity to be picked up by motion detector
#define PC_SCOUT_SCAN_TIME			2		// # of seconds between each scan pulse
#define PC_SCOUT_SCAN_RANGE			100		// Default scanner range
#define PC_SCOUT_SCAN_COST			2		// Default scanner cell useage per scan

// Class Details for SNIPER
#define PC_SNIPER_SKIN				5 
#define PC_SNIPER_MAXHEALTH			90 
#define PC_SNIPER_MAXSPEED			300 		
#define PC_SNIPER_MAXSTRAFESPEED	300 
#define PC_SNIPER_MAXARMOR			50 
#define PC_SNIPER_INITARMOR			0 
#define PC_SNIPER_MAXARMORTYPE		0.3 
#define PC_SNIPER_INITARMORTYPE		0.3 
#define PC_SNIPER_ARMORCLASSES		3 		// #AT_SAVESHOT | #AT_SAVENAIL
#define PC_SNIPER_INITARMORCLASS	0 
#define PC_SNIPER_WEAPONS			WEAP_SNIPER_RIFLE | WEAP_AUTO_RIFLE | WEAP_AXE | WEAP_NAILGUN
#define PC_SNIPER_MAXAMMO_SHOT		75 
#define PC_SNIPER_MAXAMMO_NAIL		100 
#define PC_SNIPER_MAXAMMO_CELL		50 
#define PC_SNIPER_MAXAMMO_ROCKET	25 
#define PC_SNIPER_INITAMMO_SHOT		60 
#define PC_SNIPER_INITAMMO_NAIL		50 
#define PC_SNIPER_INITAMMO_CELL		0 
#define PC_SNIPER_INITAMMO_ROCKET	0 
#define PC_SNIPER_GRENADE_TYPE_1	GR_TYPE_NORMAL
#define PC_SNIPER_GRENADE_TYPE_2	GR_TYPE_NONE
#define PC_SNIPER_GRENADE_INIT_1	2 	 
#define PC_SNIPER_GRENADE_INIT_2	0
#define PC_SNIPER_TF_ITEMS			0 

// Class Details for SOLDIER
#define PC_SOLDIER_SKIN				6 			
#define PC_SOLDIER_MAXHEALTH		100	 
#define PC_SOLDIER_MAXSPEED			240 
#define PC_SOLDIER_MAXSTRAFESPEED	240 
#define PC_SOLDIER_MAXARMOR			200 
#define PC_SOLDIER_INITARMOR		100 
#define PC_SOLDIER_MAXARMORTYPE		0.8 
#define PC_SOLDIER_INITARMORTYPE	0.8 
#define PC_SOLDIER_ARMORCLASSES		31 		// ALL
#define PC_SOLDIER_INITARMORCLASS	0 
#define PC_SOLDIER_WEAPONS		 	WEAP_AXE | WEAP_SHOTGUN | WEAP_SUPER_SHOTGUN | WEAP_ROCKET_LAUNCHER
#define PC_SOLDIER_MAXAMMO_SHOT		100 
#define PC_SOLDIER_MAXAMMO_NAIL		100 
#define PC_SOLDIER_MAXAMMO_CELL		50 
#define PC_SOLDIER_MAXAMMO_ROCKET	50 
#define PC_SOLDIER_INITAMMO_SHOT	50 
#define PC_SOLDIER_INITAMMO_NAIL	0 
#define PC_SOLDIER_INITAMMO_CELL	0 
#define PC_SOLDIER_INITAMMO_ROCKET	10 
#define PC_SOLDIER_GRENADE_TYPE_1	GR_TYPE_NORMAL
#define PC_SOLDIER_GRENADE_TYPE_2	GR_TYPE_NAIL
#define PC_SOLDIER_GRENADE_INIT_1	4 	 
#define PC_SOLDIER_GRENADE_INIT_2	1 	 
#define PC_SOLDIER_TF_ITEMS			0 

#define MAX_NAIL_GRENS				2	// Can only have 2 Nail grens active
#define MAX_NAPALM_GRENS			2	// Can only have 2 Napalm grens active
#define MAX_GAS_GRENS				2	// Can only have 2 Gas grenades active
#define MAX_MIRV_GRENS				2	// Can only have 2 Mirv's
#define MAX_CONCUSSION_GRENS		3
#define MAX_CALTROP_CANS			3

// Class Details for DEMOLITION MAN
#define PC_DEMOMAN_SKIN				1 
#define PC_DEMOMAN_MAXHEALTH		90 
#define PC_DEMOMAN_MAXSPEED			280 		
#define PC_DEMOMAN_MAXSTRAFESPEED	280 
#define PC_DEMOMAN_MAXARMOR			120 
#define PC_DEMOMAN_INITARMOR		50 
#define PC_DEMOMAN_MAXARMORTYPE		0.6 
#define PC_DEMOMAN_INITARMORTYPE	0.6 
#define PC_DEMOMAN_ARMORCLASSES		31 		// ALL
#define PC_DEMOMAN_INITARMORCLASS	0 		
#define PC_DEMOMAN_WEAPONS			WEAP_AXE | WEAP_SHOTGUN | WEAP_GRENADE_LAUNCHER | WEAP_DETPACK
#define PC_DEMOMAN_MAXAMMO_SHOT		75 
#define PC_DEMOMAN_MAXAMMO_NAIL		50 
#define PC_DEMOMAN_MAXAMMO_CELL		50 
#define PC_DEMOMAN_MAXAMMO_ROCKET	50 
#define PC_DEMOMAN_MAXAMMO_DETPACK	1 
#define PC_DEMOMAN_INITAMMO_SHOT	30 
#define PC_DEMOMAN_INITAMMO_NAIL	0 
#define PC_DEMOMAN_INITAMMO_CELL	0 
#define PC_DEMOMAN_INITAMMO_ROCKET	20 
#define PC_DEMOMAN_INITAMMO_DETPACK	1 
#define PC_DEMOMAN_GRENADE_TYPE_1	GR_TYPE_NORMAL
#define PC_DEMOMAN_GRENADE_TYPE_2	GR_TYPE_MIRV
#define PC_DEMOMAN_GRENADE_INIT_1	4 	 
#define PC_DEMOMAN_GRENADE_INIT_2	4 	 
#define PC_DEMOMAN_TF_ITEMS			0 

// Class Details for COMBAT MEDIC
#define PC_MEDIC_SKIN				3 
#define PC_MEDIC_MAXHEALTH			90 
#define PC_MEDIC_MAXSPEED			320 
#define PC_MEDIC_MAXSTRAFESPEED		320 
#define PC_MEDIC_MAXARMOR			100
#define PC_MEDIC_INITARMOR			50 
#define PC_MEDIC_MAXARMORTYPE		0.6 
#define PC_MEDIC_INITARMORTYPE		0.3 
#define PC_MEDIC_ARMORCLASSES		11 		// ALL except EXPLOSION
#define PC_MEDIC_INITARMORCLASS		0 
#define PC_MEDIC_WEAPONS			WEAP_BIOWEAPON | WEAP_MEDIKIT | WEAP_SHOTGUN | WEAP_SUPER_SHOTGUN | WEAP_SUPER_NAILGUN
#define PC_MEDIC_MAXAMMO_SHOT		75 
#define PC_MEDIC_MAXAMMO_NAIL		150 
#define PC_MEDIC_MAXAMMO_CELL		50 
#define PC_MEDIC_MAXAMMO_ROCKET		25 
#define PC_MEDIC_MAXAMMO_MEDIKIT	100 
#define PC_MEDIC_INITAMMO_SHOT		50 
#define PC_MEDIC_INITAMMO_NAIL		50 
#define PC_MEDIC_INITAMMO_CELL		0 
#define PC_MEDIC_INITAMMO_ROCKET	0 
#define PC_MEDIC_INITAMMO_MEDIKIT	50 
#define PC_MEDIC_GRENADE_TYPE_1		GR_TYPE_NORMAL
#define PC_MEDIC_GRENADE_TYPE_2		GR_TYPE_CONCUSSION
#define PC_MEDIC_GRENADE_INIT_1		3 	 
#define PC_MEDIC_GRENADE_INIT_2		2 	 
#define PC_MEDIC_TF_ITEMS			0 
#define PC_MEDIC_REGEN_TIME			3   // Number of seconds between each regen.
#define PC_MEDIC_REGEN_AMOUNT		2 	// Amount of health regenerated each regen.

// Class Details for HVYWEAP
#define PC_HVYWEAP_SKIN				2 
#define PC_HVYWEAP_MAXHEALTH		100 
#define PC_HVYWEAP_MAXSPEED			230		
#define PC_HVYWEAP_MAXSTRAFESPEED	230
#define PC_HVYWEAP_MAXARMOR			300 
#define PC_HVYWEAP_INITARMOR		150 
#define PC_HVYWEAP_MAXARMORTYPE		0.8 
#define PC_HVYWEAP_INITARMORTYPE	0.8 
#define PC_HVYWEAP_ARMORCLASSES		31 			// ALL
#define PC_HVYWEAP_INITARMORCLASS	0 		
#define PC_HVYWEAP_WEAPONS			WEAP_ASSAULT_CANNON | WEAP_AXE | WEAP_SHOTGUN | WEAP_SUPER_SHOTGUN
#define PC_HVYWEAP_MAXAMMO_SHOT		200 
#define PC_HVYWEAP_MAXAMMO_NAIL		200 
#define PC_HVYWEAP_MAXAMMO_CELL		50 
#define PC_HVYWEAP_MAXAMMO_ROCKET	25 
#define PC_HVYWEAP_INITAMMO_SHOT	200 
#define PC_HVYWEAP_INITAMMO_NAIL	0 
#define PC_HVYWEAP_INITAMMO_CELL	30 
#define PC_HVYWEAP_INITAMMO_ROCKET	0 
#define PC_HVYWEAP_GRENADE_TYPE_1	GR_TYPE_NORMAL
#define PC_HVYWEAP_GRENADE_TYPE_2	GR_TYPE_MIRV
#define PC_HVYWEAP_GRENADE_INIT_1	4 	 
#define PC_HVYWEAP_GRENADE_INIT_2	1 	 
#define PC_HVYWEAP_TF_ITEMS			0 
#define PC_HVYWEAP_CELL_USAGE		7	// Amount of cells spent to power up assault cannon



// Class Details for PYRO
#define PC_PYRO_SKIN			21 
#define PC_PYRO_MAXHEALTH		100 
#define PC_PYRO_MAXSPEED		300 
#define PC_PYRO_MAXSTRAFESPEED	300
#define PC_PYRO_MAXARMOR		150 
#define PC_PYRO_INITARMOR		50 
#define PC_PYRO_MAXARMORTYPE	0.6 
#define PC_PYRO_INITARMORTYPE	0.6 
#define PC_PYRO_ARMORCLASSES	27 		// ALL except EXPLOSION
#define PC_PYRO_INITARMORCLASS	16  	// #AT_SAVEFIRE
#define PC_PYRO_WEAPONS			WEAP_INCENDIARY | WEAP_FLAMETHROWER | WEAP_AXE | WEAP_SHOTGUN
#define PC_PYRO_MAXAMMO_SHOT	40 
#define PC_PYRO_MAXAMMO_NAIL	50 
#define PC_PYRO_MAXAMMO_CELL	200 
#define PC_PYRO_MAXAMMO_ROCKET	20 
#define PC_PYRO_INITAMMO_SHOT	20 
#define PC_PYRO_INITAMMO_NAIL	0 
#define PC_PYRO_INITAMMO_CELL	120 
#define PC_PYRO_INITAMMO_ROCKET	5 
#define PC_PYRO_GRENADE_TYPE_1	GR_TYPE_NORMAL
#define PC_PYRO_GRENADE_TYPE_2	GR_TYPE_NAPALM
#define PC_PYRO_GRENADE_INIT_1	1 	 
#define PC_PYRO_GRENADE_INIT_2	4 	 
#define PC_PYRO_TF_ITEMS		0
#define PC_PYRO_ROCKET_USAGE	3	// Number of rockets per incendiary cannon shot

// Class Details for SPY
#define PC_SPY_SKIN				22 
#define PC_SPY_MAXHEALTH		90 
#define PC_SPY_MAXSPEED			300 
#define PC_SPY_MAXSTRAFESPEED	300 
#define PC_SPY_MAXARMOR			100 
#define PC_SPY_INITARMOR		25 
#define PC_SPY_MAXARMORTYPE		0.6		// Was 0.3 
#define PC_SPY_INITARMORTYPE	0.6		// Was 0.3
#define PC_SPY_ARMORCLASSES		27 		// ALL except EXPLOSION
#define PC_SPY_INITARMORCLASS	0  
#define PC_SPY_WEAPONS			WEAP_AXE | WEAP_TRANQ | WEAP_SUPER_SHOTGUN | WEAP_NAILGUN
#define PC_SPY_MAXAMMO_SHOT		40 
#define PC_SPY_MAXAMMO_NAIL		100 
#define PC_SPY_MAXAMMO_CELL		30 
#define PC_SPY_MAXAMMO_ROCKET	15 
#define PC_SPY_INITAMMO_SHOT	40 
#define PC_SPY_INITAMMO_NAIL	50 
#define PC_SPY_INITAMMO_CELL	10 
#define PC_SPY_INITAMMO_ROCKET	0 
#define PC_SPY_GRENADE_TYPE_1	GR_TYPE_NORMAL
#define PC_SPY_GRENADE_TYPE_2	GR_TYPE_GAS
#define PC_SPY_GRENADE_INIT_1	2 	 
#define PC_SPY_GRENADE_INIT_2	2 	 
#define PC_SPY_TF_ITEMS			0 
#define PC_SPY_CELL_REGEN_TIME		5	
#define PC_SPY_CELL_REGEN_AMOUNT	1
#define PC_SPY_CELL_USAGE			3	// Amount of cells spent while invisible
#define PC_SPY_GO_UNDERCOVER_TIME	4	// Time it takes to go undercover

// Class Details for ENGINEER
#define PC_ENGINEER_SKIN			22 		// Not used anymore
#define PC_ENGINEER_MAXHEALTH		80 
#define PC_ENGINEER_MAXSPEED		300 
#define PC_ENGINEER_MAXSTRAFESPEED	300
#define PC_ENGINEER_MAXARMOR		50
#define PC_ENGINEER_INITARMOR		25 
#define PC_ENGINEER_MAXARMORTYPE	0.6 
#define PC_ENGINEER_INITARMORTYPE	0.3 
#define PC_ENGINEER_ARMORCLASSES	31 		// ALL
#define PC_ENGINEER_INITARMORCLASS	0  
#define PC_ENGINEER_WEAPONS			WEAP_SPANNER | WEAP_LASER | WEAP_SUPER_SHOTGUN
#define PC_ENGINEER_MAXAMMO_SHOT	50
#define PC_ENGINEER_MAXAMMO_NAIL	50 
#define PC_ENGINEER_MAXAMMO_CELL	200		// synonymous with metal 
#define PC_ENGINEER_MAXAMMO_ROCKET	30 
#define PC_ENGINEER_INITAMMO_SHOT	20 
#define PC_ENGINEER_INITAMMO_NAIL	25 
#define PC_ENGINEER_INITAMMO_CELL	100 	// synonymous with metal 
#define PC_ENGINEER_INITAMMO_ROCKET	0 
#define PC_ENGINEER_GRENADE_TYPE_1	GR_TYPE_NORMAL
#define PC_ENGINEER_GRENADE_TYPE_2	GR_TYPE_EMP
#define PC_ENGINEER_GRENADE_INIT_1	2 	 
#define PC_ENGINEER_GRENADE_INIT_2	2 	 
#define PC_ENGINEER_TF_ITEMS		0 

// Class Details for CIVILIAN
#define PC_CIVILIAN_SKIN			22 
#define PC_CIVILIAN_MAXHEALTH		50
#define PC_CIVILIAN_MAXSPEED		240
#define PC_CIVILIAN_MAXSTRAFESPEED	240
#define PC_CIVILIAN_MAXARMOR		0
#define PC_CIVILIAN_INITARMOR		0 
#define PC_CIVILIAN_MAXARMORTYPE	0
#define PC_CIVILIAN_INITARMORTYPE	0 
#define PC_CIVILIAN_ARMORCLASSES	0 		
#define PC_CIVILIAN_INITARMORCLASS	0
#define PC_CIVILIAN_WEAPONS			WEAP_AXE
#define PC_CIVILIAN_MAXAMMO_SHOT	0
#define PC_CIVILIAN_MAXAMMO_NAIL	0 
#define PC_CIVILIAN_MAXAMMO_CELL	0 
#define PC_CIVILIAN_MAXAMMO_ROCKET	0 
#define PC_CIVILIAN_INITAMMO_SHOT	0 
#define PC_CIVILIAN_INITAMMO_NAIL	0 
#define PC_CIVILIAN_INITAMMO_CELL	0 
#define PC_CIVILIAN_INITAMMO_ROCKET	0 
#define PC_CIVILIAN_GRENADE_TYPE_1	0
#define PC_CIVILIAN_GRENADE_TYPE_2	0
#define PC_CIVILIAN_GRENADE_INIT_1	0 	 
#define PC_CIVILIAN_GRENADE_INIT_2	0 	 
#define PC_CIVILIAN_TF_ITEMS		0 


/*==========================================================================*/
/* TEAMFORTRESS GOALS														*/
/*==========================================================================*/
// For all these defines, see the tfortmap.txt that came with the zip
// for complete descriptions.
// Defines for Goal Activation types : goal_activation (in goals)
#define TFGA_TOUCH			1  // Activated when touched
#define TFGA_TOUCH_DETPACK	2  // Activated when touched by a detpack explosion
#define TFGA_REVERSE_AP		4  // Activated when AP details are _not_ met
#define TFGA_SPANNER		8  // Activated when hit by an engineer's spanner
#define TFGA_DROPTOGROUND	2048 // Drop to Ground when spawning

// Defines for Goal Effects types : goal_effect
#define TFGE_AP				  1  // AP is affected. Default.
#define TFGE_AP_TEAM		  2  // All of the AP's team.
#define TFGE_NOT_AP_TEAM	  4  // All except AP's team.
#define TFGE_NOT_AP			  8  // All except AP.
#define TFGE_WALL			  16 // If set, walls stop the Radius effects
#define TFGE_SAME_ENVIRONMENT 32 // If set, players in a different environment to the Goal are not affected
#define TFGE_TIMER_CHECK_AP	  64 // If set, Timer Goals check their critera for all players fitting their effects

// Defines for Goal Result types : goal_result
#define TFGR_SINGLE				1  // Goal can only be activated once
#define TFGR_ADD_BONUSES		2 	// Any Goals activated by this one give their bonuses
#define TFGR_ENDGAME			4 	// Goal fires Intermission, displays scores, and ends level
#define TFGR_NO_ITEM_RESULTS	8	// GoalItems given by this Goal don't do results
#define TFGR_REMOVE_DISGUISE	16 // Prevent/Remove undercover from any Spy
#define TFGR_FORCE_RESPAWN		32 // Forces the player to teleport to a respawn point
#define TFGR_DESTROY_BUILDINGS	64 // Destroys this player's buildings, if anys

// Defines for Goal Group Result types : goal_group
// None!
// But I'm leaving this variable in there, since it's fairly likely
// that some will show up sometime.

// Defines for Goal Item types, : goal_activation (in items)
#define TFGI_GLOW			1   // Players carrying this GoalItem will glow
#define TFGI_SLOW			2   // Players carrying this GoalItem will move at half-speed
#define TFGI_DROP			4   // Players dying with this item will drop it
#define TFGI_RETURN_DROP	8   // Return if a player with it dies
#define TFGI_RETURN_GOAL	16  // Return if a player with it has it removed by a goal's activation
#define TFGI_RETURN_REMOVE	32  // Return if it is removed by TFGI_REMOVE
#define TFGI_REVERSE_AP		64  // Only pickup if the player _doesn't_ match AP Details
#define TFGI_REMOVE			128 // Remove if left untouched for 2 minutes after being dropped
#define TFGI_KEEP			256 // Players keep this item even when they die
#define TFGI_ITEMGLOWS		512	// Item glows when on the ground
#define TFGI_DONTREMOVERES	1024 // Don't remove results when the item is removed
#define TFGI_DROPTOGROUND	2048 // Drop To Ground when spawning
#define TFGI_CANBEDROPPED	4096 // Can be voluntarily dropped by players
#define TFGI_SOLID			8192 // Is solid... blocks bullets, etc

// Defines for methods of GoalItem returning
#define GI_RET_DROP_DEAD 	0		// Dropped by a dead player
#define GI_RET_DROP_LIVING 	1		// Dropped by a living player
#define GI_RET_GOAL			2		// Returned by a Goal
#define GI_RET_TIME			3		// Returned due to timeout

// Defines for TeamSpawnpoints : goal_activation (in teamspawns)
#define TFSP_MULTIPLEITEMS	1  // Give out the GoalItem multiple times
#define TFSP_MULTIPLEMSGS	2  // Display the message multiple times

// Defines for TeamSpawnpoints : goal_effects (in teamspawns)
#define TFSP_REMOVESELF		1  // Remove itself after being spawned on

// Defines for Goal States
#define TFGS_ACTIVE		1 
#define TFGS_INACTIVE	2 
#define TFGS_REMOVED	3 
#define TFGS_DELAYED	4

// Defines for GoalItem Removing from Player Methods
#define GI_DROP_PLAYERDEATH	  0		// Dropped by a dying player
#define GI_DROP_REMOVEGOAL	  1		// Removed by a Goal
#define GI_DROP_PLAYERDROP	  2		// Dropped by a player

// Legal Playerclass Handling
#define TF_ILL_SCOUT 		1
#define TF_ILL_SNIPER		2
#define TF_ILL_SOLDIER		4
#define TF_ILL_DEMOMAN		8
#define TF_ILL_MEDIC		16
#define TF_ILL_HVYWEP		32
#define TF_ILL_PYRO			64
#define TF_ILL_RANDOMPC		128
#define TF_ILL_SPY			256
#define TF_ILL_ENGINEER		512

// Addition classes
#define CLASS_TFGOAL					128
#define CLASS_TFGOAL_TIMER			129
#define CLASS_TFGOAL_ITEM			130
#define CLASS_TFSPAWN				   131

/*==========================================================================*/
/* Flamethrower																*/
/*==========================================================================*/
#define FLAME_PLYRMAXTIME	4.5 // lifetime in seconds of a flame on a player
#define FLAME_MAXBURNTIME	8  	// lifetime in seconds of a flame on the world (big ones)
#define NAPALM_MAXBURNTIME	20 	// lifetime in seconds of flame from a napalm grenade
#define FLAME_MAXPLYRFLAMES	4 	// maximum number of flames on a player
#define FLAME_NUMLIGHTS		1 	// maximum number of light flame 
#define FLAME_BURNRATIO		0.3 // the chance of a flame not 'sticking'
#define GR_TYPE_FLAMES_NO	15 	// number of flames spawned when a grenade explode
#define FLAME_DAMAGE_TIME	1	// Interval between damage burns from flames
#define FLAME_EFFECT_TIME	0.2	// frequency at which we display flame effects.
#define FLAME_THINK_TIME	0.1	// Seconds between times the flame checks burn

/*==================================================*/
/* CTF Support defines 								*/
/*==================================================*/
#define CTF_FLAG1 		1
#define CTF_FLAG2 		2
#define CTF_DROPOFF1 	3
#define CTF_DROPOFF2 	4
#define CTF_SCORE1   	5
#define CTF_SCORE2   	6

//.float	hook_out;

/*==================================================*/
/* Camera defines	 								*/
/*==================================================*/
/*
float live_camera;
.float camdist;
.vector camangle;
.entity camera_list;
*/

/*==================================================*/
/* QuakeWorld defines 								*/
/*==================================================*/
/*
float already_chosen_map;

// grappling hook variables
.entity	hook;	
.float	on_hook;
.float  fire_held_down;// flag - TRUE if player is still holding down the
                       // fire button after throwing a hook.
*/
/*==================================================*/
/* Server Settings								    */
/*==================================================*/
// Admin modes
#define ADMIN_MODE_NONE	0
#define ADMIN_MODE_DEAL	1

/*==================================================*/
/* Death Message defines							*/
/*==================================================*/
#define DMSG_SHOTGUN			1
#define DMSG_SSHOTGUN			2
#define DMSG_NAILGUN			3
#define DMSG_SNAILGUN			4
#define DMSG_GRENADEL			5
#define DMSG_ROCKETL			6
#define DMSG_LIGHTNING			7
#define DMSG_GREN_HAND			8
#define DMSG_GREN_NAIL			9
#define DMSG_GREN_MIRV			10
#define DMSG_GREN_PIPE			11
#define DMSG_DETPACK			12
#define DMSG_BIOWEAPON			13
#define DMSG_BIOWEAPON_ATT		14
#define DMSG_FLAME				15
#define DMSG_DETPACK_DIS		16
#define DMSG_AXE				17
#define DMSG_SNIPERRIFLE		18
#define DMSG_AUTORIFLE			19
#define DMSG_ASSAULTCANNON		20
#define DMSG_HOOK				21
#define DMSG_BACKSTAB			22
#define DMSG_MEDIKIT			23
#define DMSG_GREN_GAS			24
#define DMSG_TRANQ				25
#define DMSG_LASERBOLT			26
#define DMSG_SENTRYGUN_BULLET 	27
#define DMSG_SNIPERLEGSHOT		28
#define DMSG_SNIPERHEADSHOT		29
#define DMSG_GREN_EMP			30
#define DMSG_GREN_EMP_AMMO		31
#define DMSG_SPANNER			32
#define DMSG_INCENDIARY			33
#define DMSG_SENTRYGUN_ROCKET	34
#define DMSG_GREN_FLASH			35
#define DMSG_TRIGGER			36
#define DMSG_MIRROR				37
#define DMSG_SENTRYDEATH		38
#define DMSG_DISPENSERDEATH		39
#define DMSG_GREN_AIRPIPE		40
#define DMSG_CALTROP			41

/*==================================================*/
// TOGGLEFLAGS
/*==================================================*/
// Some of the toggleflags aren't used anymore, but the bits are still
// there to provide compatability with old maps
#define TFLAG_CLASS_PERSIST			(1 << 0)  		// Persistent Classes Bit
#define TFLAG_CHEATCHECK			(1 << 1) 		// Cheatchecking Bit
#define TFLAG_RESPAWNDELAY			(1 << 2) 		// RespawnDelay bit
//#define TFLAG_UN					(1 << 3)		// NOT USED ANYMORE
#define TFLAG_OLD_GRENS				(1 << 3)		// Use old concussion grenade and flash grenade
#define TFLAG_UN2					(1 << 4)		// NOT USED ANYMORE
#define TFLAG_UN3					(1 << 5)		// NOT USED ANYMORE
#define TFLAG_UN4					(1 << 6)		// NOT USED ANYMORE: Was Autoteam. CVAR tfc_autoteam used now.
#define TFLAG_TEAMFRAGS				(1 << 7)		// Individual Frags, or Frags = TeamScore
#define TFLAG_FIRSTENTRY			(1 << 8)		// Used to determine the first time toggleflags is set
													// In a map. Cannot be toggled by players.
#define TFLAG_SPYINVIS				(1 << 9)		// Spy invisible only
#define TFLAG_GRAPPLE				(1 << 10)	// Grapple on/off
//#define TFLAG_FULLTEAMSCORE		(1 << 11)  	// Each Team's score is TeamScore + Frags
#define TFLAG_FLAGEMULATION			(1 << 12)  	// Flag emulation on for old TF maps
#define TFLAG_USE_STANDARD			(1 << 13)  	// Use the TF War standard for Flag emulation

#define TFLAG_FRAGSCORING			(1 << 14)	// Use frag scoring only

/*======================*/
//      Menu stuff      //
/*======================*/

#define MENU_DEFAULT				1
#define MENU_TEAM 					2
#define MENU_CLASS 					3
#define MENU_MAPBRIEFING			4
#define MENU_INTRO 					5
#define MENU_CLASSHELP				6
#define MENU_CLASSHELP2 			7
#define MENU_REPEATHELP 			8



#define MENU_SPY					12
#define MENU_SPY_SKIN				13
#define MENU_SPY_COLOR				14
#define MENU_ENGINEER				15
#define MENU_ENGINEER_FIX_DISPENSER	16
#define MENU_ENGINEER_FIX_SENTRYGUN	17
#define MENU_ENGINEER_FIX_MORTAR	18
#define MENU_DISPENSER				19
#define MENU_CLASS_CHANGE			20
#define MENU_TEAM_CHANGE			21

#define MENU_REFRESH_RATE 			25

//============================
// Timer Types
#define TF_TIMER_ANY				0
#define TF_TIMER_CONCUSSION			1
#define TF_TIMER_INFECTION			2
#define TF_TIMER_HALLUCINATION		3
#define TF_TIMER_TRANQUILISATION	4
#define TF_TIMER_ROTHEALTH			5
#define TF_TIMER_REGENERATION		6
#define TF_TIMER_GRENPRIME			7
#define TF_TIMER_CELLREGENERATION	8
#define TF_TIMER_DETPACKSET			9
#define TF_TIMER_DETPACKDISARM		10
#define TF_TIMER_BUILD				11
#define TF_TIMER_CHECKBUILDDISTANCE 12
#define TF_TIMER_DISGUISE			13

// Non Player timers
#define TF_TIMER_RETURNITEM			100
#define TF_TIMER_DELAYEDGOAL		101

//============================
// Teamscore printing
#define TS_PRINT_SHORT				1
#define TS_PRINT_LONG				2
#define TS_PRINT_LONG_TO_ALL		3

#ifndef TF_DEFS_ONLY
/*==================================================*/
/* GLOBAL VARIABLES									*/
/*==================================================*/
// FortressMap stuff
extern float number_of_teams;	// number of teams supported by the map
extern int   illegalclasses[5];	// Illegal playerclasses for all teams
extern int   civilianteams;		// Bitfield holding Civilian teams
extern Vector  rgbcolors[5];		 // RGB colors for each of the 4 teams
extern int   teamcolors[5];		// Colours for each of the 4 teams
extern int   teamscores[5];		// Goal Score of each team
extern int	 g_iOrderedTeams[5]; // Teams ordered into order of winners->losers
extern int	 teamfrags[5];		// Total Frags for each team
extern int   teamlives[5];		// Number of lives each team's players have
extern int   teammaxplayers[5];	// Max number of players allowed in each team
extern float teamadvantage[5];	// only used if the teamplay equalisation bits are set
								// stores the damage ratio players take/give
extern int   teamallies[5];		// Keeps track of which teams are allied
extern string_t	team_names[5];

extern BOOL  CTF_Map;
extern BOOL  birthday;
extern BOOL  christmas;

extern float num_world_flames;

// Clan Battle stuff
extern float clan_scores_dumped;
extern float cb_prematch_time;
extern float fOldPrematch;
extern float fOldCeaseFire;
extern float cb_ceasefire_time;
extern float last_id;
extern float spy_off;
extern float old_grens;		
extern float flagem_checked;
extern float flNextEqualisationCalc;
extern BOOL  cease_fire;
extern BOOL  initial_cease_fire;
extern BOOL  last_cease_fire;
// Autokick stuff
extern float autokick_kills;

extern float deathmsg;		// Global, which is set before every T_Damage, to indicate
							// the death message that should be used.

extern char *sTeamSpawnNames[];
extern char *sClassNames[];
extern char *sClassModelFiles[];
extern char *sClassModels[];
extern char *sClassCfgs[];
extern char *sGrenadeNames[];
extern string_t	team_menu_string;	

extern int toggleflags;					// toggleable flags

extern CBaseEntity* g_pLastSpawns[5];
extern BOOL g_bFirstClient;

extern float g_fNextPrematchAlert;

typedef struct
{
	int			ip;
	edict_t	*pEdict;
} ip_storage_t;

extern ip_storage_t g_IpStorage[32];

class CGhost;
/*==========================================================================*/
BOOL ClassIsRestricted(float tno, int pc);
char* GetTeamName(int tno);
int TeamFortress_GetNoPlayers();
void DestroyBuilding(CBaseEntity *eng, char *bld);
void teamsprint( int tno, CBaseEntity *ignore, int msg_dest, const char *st, const char *param1 = NULL, const char *param2 = NULL, const char *param3 = NULL );
float anglemod( float v );

// Team Funcs
BOOL TeamFortress_TeamIsCivilian(float tno);
void TeamFortress_TeamShowScores(BOOL bLong, CBasePlayer *pPlayer);
BOOL TeamFortress_TeamPutPlayerInTeam();
void TeamFortress_TeamSetColor(int tno);
void TeamFortress_TeamIncreaseScore(int tno, int scoretoadd);
int TeamFortress_TeamGetScoreFrags(int tno);
int TeamFortress_TeamGetNoPlayers(int tno);
float TeamEqualiseDamage(CBaseEntity *targ, CBaseEntity *attacker, float damage);
BOOL IsSpawnPointValid( Vector &pos );
BOOL TeamFortress_SortTeams( void );
void DumpClanScores( void );
void CalculateTeamEqualiser();

// mapscript funcs
void ParseTFServerSettings();
void ParseTFMapSettings();
CBaseEntity* Finditem(int ino);
CBaseEntity* Findgoal(int gno);
CBaseEntity* Findteamspawn(int gno);
void RemoveGoal(CBaseEntity *Goal);
void tfgoalitem_GiveToPlayer(CBaseEntity *Item, CBasePlayer *AP, CBaseEntity *Goal);
void dremove( CBaseEntity *te );
void tfgoalitem_RemoveFromPlayer(CBaseEntity *Item, CBasePlayer *AP, int iMethod);
void tfgoalitem_drop(CBaseEntity *Item, BOOL PAlive, CBasePlayer *P);
void DisplayItemStatus(CBaseEntity *Goal, CBasePlayer *Player, CBaseEntity *Item);
void tfgoalitem_checkgoalreturn(CBaseEntity *Item);
void DoGoalWork(CBaseEntity *Goal, CBasePlayer *AP);
void DoResults(CBaseEntity *Goal, CBasePlayer *AP, BOOL bAddBonuses);
void DoGroupWork(CBaseEntity *Goal, CBasePlayer *AP);
// hooks into the mapscript for all entities
BOOL ActivateDoResults(CBaseEntity *Goal, CBasePlayer *AP, CBaseEntity *ActivatingGoal);
BOOL ActivationSucceeded(CBaseEntity *Goal, CBasePlayer *AP, CBaseEntity *ActivatingGoal);

// prematch & ceasefire
void Display_Prematch();
void Check_Ceasefire();

// admin
void KickPlayer( CBaseEntity *pTarget );
void BanPlayer( CBaseEntity *pTarget );
CGhost *FindGhost( int iGhostID );
int GetBattleID( edict_t *pEntity );

extern cvar_t	tfc_spam_penalty1;// the initial gag penalty for a spammer (seconds)
extern cvar_t	tfc_spam_penalty2;// incremental gag penalty (seconds) for each time gagged spammer continues to speak.
extern cvar_t	tfc_spam_limit; // at this many points, gag the spammer
extern cvar_t	tfc_clanbattle, tfc_clanbattle_prematch, tfc_prematch, tfc_clanbattle_ceasefire, tfc_balance_teams, tfc_balance_scores;
extern cvar_t   tfc_clanbattle_locked, tfc_birthday, tfc_autokick_kills, tfc_fragscoring, tfc_autokick_time, tfc_adminpwd;
extern cvar_t	weaponstay, footsteps, flashlight, aimcrosshair, falldamage, teamplay;

/*==========================================================================*/
class CTFFlame : public CBaseMonster
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	EXPORT FlameThink( void );
	static  CTFFlame *FlameSpawn( CBaseEntity *pOwner, CBaseEntity *pTarget );
	void	FlameDestroy( void );

	float	m_flNextDamageTime;
};

/*==========================================================================*/
// MAPSCRIPT CLASSES
class CTFGoal : public CBaseAnimating
{
public:
	void	Spawn( void );
	void	StartGoal( void );
	void	EXPORT PlaceGoal( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int		Classify ( void ) { return	CLASS_TFGOAL; }

	void	SetObjectCollisionBox( void );
};
 
class CTFGoalItem : public CTFGoal
{
public:
	void	Spawn( void );
	void	StartItem( void );
	void	EXPORT PlaceItem( void );
	int		Classify ( void ) { return	CLASS_TFGOAL_ITEM; }

	float	m_flDroppedAt;
};

class CTFTimerGoal : public CTFGoal
{
public:
	void	Spawn( void );
	int		Classify ( void ) { return	CLASS_TFGOAL_TIMER; }
};

class CTFSpawn : public CBaseEntity
{
public:
	void	Spawn( void );
	int		Classify ( void ) { return	CLASS_TFSPAWN; }
};

class CTFDetect : public CBaseEntity
{
public:
	void	Spawn( void );
	int		Classify ( void ) { return	CLASS_TFGOAL; }
};

class CTelefragDeath : public CBaseEntity
{
public:
	void		Spawn( void );
	void		EXPORT	DeathTouch( CBaseEntity *pOther );
};

#endif // TF_DEFS_ONLY
#endif // __TF_DEFS_H


