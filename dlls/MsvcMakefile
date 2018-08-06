# hl.dll Makefile for Microsoft C++ compiler

TARGET=hl.dll
DEF_FILE=hl.def
HL_DIR=C:\Program Files (x86)\Steam\steamapps\common\Half-Life
MOD_NAME=valve


############################################################################

CC=cl
LINK=link
CFLAGS=/Gd /GR /MT /GL /Gy /Oi /O2 /D NDEBUG /D WIN32 /D _WINDOWS /D VALVE_DLL /D CLIENT_WEAPONS /D QUIVER /D VOXEL /D QUAKE2 /D _WINDLL
LDFLAGS= /INCREMENTAL:NO /SUBSYSTEM:WINDOWS /LTCG /TLBID:1 /DYNAMICBASE /NXCOMPAT

INCLUDEDIRS=/I ..\dlls /I ..\engine /I ..\common /I ..\pm_shared /I ..\game_shared /I ..\public

DLL_SRCDIR=.
ENGINE_SRCDIR=..\engine
COMMON_SRCDIR=..\common
WPN_SHARED_SRCDIR=.\wpn_shared
PM_SHARED_SRCDIR=..\pm_shared
GAME_SHARED_SRCDIR=..\game_shared

MAIN_OBJ_DIR=.\obj
DLL_OBJDIR=$(MAIN_OBJ_DIR)\dlls
WPN_SHARED_OBJDIR=$(DLL_OBJDIR)\wpn_shared
PM_SHARED_OBJDIR=$(MAIN_OBJ_DIR)\pm_shared
GAME_SHARED_OBJDIR=$(MAIN_OBJ_DIR)\game_shared


#############################################################################

OBJ = \
	$(DLL_OBJDIR)\aflock.obj \
	$(DLL_OBJDIR)\agrunt.obj \
	$(DLL_OBJDIR)\airtank.obj \
	$(DLL_OBJDIR)\animating.obj \
	$(DLL_OBJDIR)\animation.obj	\
	$(DLL_OBJDIR)\apache.obj \
	$(DLL_OBJDIR)\barnacle.obj \
	$(DLL_OBJDIR)\barney.obj \
	$(DLL_OBJDIR)\bigmomma.obj \
	$(DLL_OBJDIR)\bloater.obj \
	$(DLL_OBJDIR)\bmodels.obj \
	$(DLL_OBJDIR)\bullsquid.obj \
	$(DLL_OBJDIR)\buttons.obj \
	$(DLL_OBJDIR)\cbase.obj \
	$(DLL_OBJDIR)\client.obj \
	$(DLL_OBJDIR)\combat.obj \
	$(DLL_OBJDIR)\controller.obj \
	$(DLL_OBJDIR)\crossbow.obj \
	$(DLL_OBJDIR)\crowbar.obj \
	$(DLL_OBJDIR)\defaultai.obj \
	$(DLL_OBJDIR)\doors.obj \
	$(DLL_OBJDIR)\effects.obj \
	$(DLL_OBJDIR)\egon.obj \
	$(DLL_OBJDIR)\explode.obj \
	$(DLL_OBJDIR)\flyingmonster.obj \
	$(DLL_OBJDIR)\func_break.obj \
	$(DLL_OBJDIR)\func_tank.obj \
	$(DLL_OBJDIR)\game.obj \
	$(DLL_OBJDIR)\gamerules.obj \
	$(DLL_OBJDIR)\gargantua.obj \
	$(DLL_OBJDIR)\gauss.obj \
	$(DLL_OBJDIR)\genericmonster.obj \
	$(DLL_OBJDIR)\ggrenade.obj \
	$(DLL_OBJDIR)\globals.obj \
	$(DLL_OBJDIR)\gman.obj \
	$(DLL_OBJDIR)\h_ai.obj \
	$(DLL_OBJDIR)\h_battery.obj \
	$(DLL_OBJDIR)\h_cine.obj \
	$(DLL_OBJDIR)\h_cycler.obj \
	$(DLL_OBJDIR)\h_export.obj \
	$(DLL_OBJDIR)\handgrenade.obj \
	$(DLL_OBJDIR)\hassassin.obj \
	$(DLL_OBJDIR)\headcrab.obj \
	$(DLL_OBJDIR)\healthkit.obj \
	$(DLL_OBJDIR)\hgrunt.obj \
	$(DLL_OBJDIR)\hornet.obj \
	$(DLL_OBJDIR)\hornetgun.obj \
	$(DLL_OBJDIR)\houndeye.obj \
	$(DLL_OBJDIR)\ichthyosaur.obj \
	$(DLL_OBJDIR)\islave.obj \
	$(DLL_OBJDIR)\items.obj \
	$(DLL_OBJDIR)\leech.obj \
	$(DLL_OBJDIR)\lights.obj \
	$(DLL_OBJDIR)\maprules.obj \
	$(DLL_OBJDIR)\monstermaker.obj \
	$(DLL_OBJDIR)\monsters.obj \
	$(DLL_OBJDIR)\monsterstate.obj \
	$(DLL_OBJDIR)\mortar.obj \
	$(DLL_OBJDIR)\mp5.obj \
	$(DLL_OBJDIR)\multiplay_gamerules.obj \
	$(DLL_OBJDIR)\nihilanth.obj \
	$(DLL_OBJDIR)\nodes.obj \
	$(DLL_OBJDIR)\observer.obj \
	$(DLL_OBJDIR)\osprey.obj \
	$(DLL_OBJDIR)\pathcorner.obj \
	$(DLL_OBJDIR)\plane.obj \
	$(DLL_OBJDIR)\plats.obj \
	$(DLL_OBJDIR)\player.obj \
	$(DLL_OBJDIR)\python.obj \
	$(DLL_OBJDIR)\rat.obj \
	$(DLL_OBJDIR)\roach.obj \
	$(DLL_OBJDIR)\rpg.obj \
	$(DLL_OBJDIR)\satchel.obj \
	$(DLL_OBJDIR)\schedule.obj \
	$(DLL_OBJDIR)\scientist.obj \
	$(DLL_OBJDIR)\scripted.obj \
	$(DLL_OBJDIR)\shotgun.obj \
	$(DLL_OBJDIR)\singleplay_gamerules.obj \
	$(DLL_OBJDIR)\skill.obj \
	$(DLL_OBJDIR)\sound.obj \
	$(DLL_OBJDIR)\soundent.obj \
	$(DLL_OBJDIR)\spectator.obj \
	$(DLL_OBJDIR)\squadmonster.obj \
	$(DLL_OBJDIR)\squeakgrenade.obj \
	$(DLL_OBJDIR)\subs.obj \
	$(DLL_OBJDIR)\talkmonster.obj \
	$(DLL_OBJDIR)\teamplay_gamerules.obj \
	$(DLL_OBJDIR)\tempmonster.obj \
	$(DLL_OBJDIR)\tentacle.obj \
	$(DLL_OBJDIR)\triggers.obj \
	$(DLL_OBJDIR)\tripmine.obj \
	$(DLL_OBJDIR)\turret.obj \
	$(DLL_OBJDIR)\util.obj \
	$(DLL_OBJDIR)\weapons.obj \
	$(DLL_OBJDIR)\world.obj \
	$(DLL_OBJDIR)\xen.obj \
	$(DLL_OBJDIR)\zombie.obj \
	$(WPN_SHARED_OBJDIR)\hl_wpn_glock.obj \
	$(GAME_SHARED_OBJDIR)\voice_gamemgr.obj \
	$(PM_SHARED_OBJDIR)\pm_debug.obj \
	$(PM_SHARED_OBJDIR)\pm_math.obj \
	$(PM_SHARED_OBJDIR)\pm_shared.obj

all : neat $(TARGET)
	
{$(GAME_SHARED_SRCDIR)}.cpp{$(GAME_SHARED_OBJDIR)}.obj :
	$(CC) $** /c /Fo$(GAME_SHARED_OBJDIR)\ $(CFLAGS) $(INCLUDEDIRS)
	
{$(PM_SHARED_SRCDIR)}.c{$(PM_SHARED_OBJDIR)}.obj :
	$(CC) $** /c /Fo$(PM_SHARED_OBJDIR)\ $(CFLAGS) $(INCLUDEDIRS)
	
{$(WPN_SHARED_SRCDIR)}.cpp{$(WPN_SHARED_OBJDIR)}.obj :
	$(CC) $** /c /Fo$(WPN_SHARED_OBJDIR)\ $(CFLAGS) $(INCLUDEDIRS)
		
{$(DLL_SRCDIR)}.cpp{$(DLL_OBJDIR)}.obj :
	$(CC) $** /c /Fo$(DLL_OBJDIR)\ $(CFLAGS) $(INCLUDEDIRS)
	
$(TARGET): $(OBJ)
	$(LINK) $** /OUT:$@ /DEF:$(DEF_FILE) /DLL $(LDFLAGS)

install : all
	@if exist "$(HL_DIR)\$(MOD_NAME)\dlls\$(TARGET)" del "$(HL_DIR)\$(MOD_NAME)\dlls\$(TARGET)"
	copy $(TARGET) "$(HL_DIR)\$(MOD_NAME)\dlls\$(TARGET)"

neat :
	@if not exist $(DLL_OBJDIR) mkdir $(DLL_OBJDIR)
	@if not exist $(WPN_SHARED_OBJDIR) mkdir $(WPN_SHARED_OBJDIR)
	@if not exist $(GAME_SHARED_OBJDIR) mkdir $(GAME_SHARED_OBJDIR)
	@if not exist $(PM_SHARED_OBJDIR) mkdir $(PM_SHARED_OBJDIR)
	
clean :
	@del $(OBJ)
	@del $(TARGET)
	@del *.lib
	@del *.exp
	
spotless :
	@if exist $(MAIN_OBJ_DIR) rmdir /S /Q $(MAIN_OBJ_DIR)
	@if exist $(DLL_OBJDIR) rmdir /S /Q $(DLL_OBJDIR)
	@if exist $(WPN_SHARED_OBJDIR) rmdir /S /Q $(WPN_SHARED_OBJDIR)
	@if exist $(GAME_SHARED_OBJDIR) rmdir /S /Q $(GAME_SHARED_OBJDIR)
	@if exist $(PM_SHARED_OBJDIR) rmdir /S /Q $(PM_SHARED_OBJDIR)

	



