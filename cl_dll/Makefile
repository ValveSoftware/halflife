# Half-Life client.dll Makefile for Microsoft C++ compiler

TARGET=client.dll
HL_DIR=C:\Program Files (x86)\Steam\steamapps\common\Half-Life
MOD_NAME=valve

######################################################################################

CFLAGS=/Zi /nologo /W3 /WX- /O2 /Oi /Oy- /GL /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "CLIENT_DLL" /D "CLIENT_WEAPONS" /D "HL_DLL" /D "_WINDLL" \
	/Gm- /EHsc /MT /GS /Gy /GR /D _CRT_SECURE_NO_WARNINGS
LDFLAGS=/LTCG /INCREMENTAL:NO /SUBSYSTEM:WINDOWS

HL_SRC_DIR=.
HL_HL_SRC_DIR=.\hl
HL_SERVER_SRC_DIR=..\dlls
ENGINE_SRC_DIR=..\engine
COMMON_SRC_DIR=..\common
PM_SHARED_SRC_DIR=..\pm_shared
GAME_SHARED_SRC_DIR=..\game_shared
PUBLIC_SRC_DIR=..\public
WPN_SHARED_SRC_DIR=$(HL_SERVER_SRC_DIR)\wpn_shared

MAIN_OBJ_DIR=.\obj
HL1_OBJ_DIR=$(MAIN_OBJ_DIR)\cl_dll
PUBLIC_OBJ_DIR=$(MAIN_OBJ_DIR)\public
COMMON_OBJ_DIR=$(MAIN_OBJ_DIR)\common
GAME_SHARED_OBJ_DIR=$(MAIN_OBJ_DIR)\game_shared
HL1_SERVER_OBJ_DIR=$(MAIN_OBJ_DIR)\dlls
PM_SHARED_OBJ_DIR=$(MAIN_OBJ_DIR)\pm_shared
WPN_SHARED_OBJ_DIR=$(HL1_SERVER_OBJ_DIR)\wpn_shared
HL_HL_OBJ_DIR=$(HL1_OBJ_DIR)\hl

INCLUDEDIRS=/I. /I../dlls /I..\common /I..\public /I../pm_shared /I../engine /I../utils/vgui/include /I ../game_shared /I../external

#####################################################################################

LIBS = \
	..\utils\vgui\lib\win32_vc6\vgui.lib \
	..\lib\public\sdl2.lib \
	..\lib\public\game_controls.lib \
	wsock32.lib \
	user32.lib

HL1_OBJS = \
	$(HL1_OBJ_DIR)\hud_spectator.obj \
	$(HL1_OBJ_DIR)\ev_hldm.obj \
	$(HL1_OBJ_DIR)\hud.obj \
	$(HL1_OBJ_DIR)\inputw32.obj \
	$(HL1_OBJ_DIR)\ammo.obj \
	$(HL1_OBJ_DIR)\ammo_secondary.obj \
	$(HL1_OBJ_DIR)\ammohistory.obj \
	$(HL1_OBJ_DIR)\battery.obj \
	$(HL1_OBJ_DIR)\cdll_int.obj \
	$(HL1_OBJ_DIR)\com_weapons.obj \
	$(HL1_OBJ_DIR)\death.obj \
	$(HL1_OBJ_DIR)\demo.obj \
	$(HL1_OBJ_DIR)\entity.obj \
	$(HL1_OBJ_DIR)\ev_common.obj \
	$(HL1_OBJ_DIR)\events.obj \
	$(HL1_OBJ_DIR)\flashlight.obj \
	$(HL1_OBJ_DIR)\GameStudioModelRenderer.obj \
	$(HL1_OBJ_DIR)\geiger.obj \
	$(HL1_OBJ_DIR)\health.obj \
	$(HL1_OBJ_DIR)\hud_bench.obj \
	$(HL1_OBJ_DIR)\hud_benchtrace.obj \
	$(HL1_OBJ_DIR)\hud_msg.obj \
	$(HL1_OBJ_DIR)\hud_redraw.obj \
	$(HL1_OBJ_DIR)\hud_update.obj \
	$(HL1_OBJ_DIR)\in_camera.obj \
	$(HL1_OBJ_DIR)\input.obj \
	$(HL1_OBJ_DIR)\interpolation.obj \
	$(HL1_OBJ_DIR)\menu.obj \
	$(HL1_OBJ_DIR)\message.obj \
	$(HL1_OBJ_DIR)\saytext.obj \
	$(HL1_OBJ_DIR)\status_icons.obj \
	$(HL1_OBJ_DIR)\statusbar.obj \
	$(HL1_OBJ_DIR)\studio_util.obj \
	$(HL1_OBJ_DIR)\StudioModelRenderer.obj \
	$(HL1_OBJ_DIR)\text_message.obj \
	$(HL1_OBJ_DIR)\train.obj \
	$(HL1_OBJ_DIR)\tri.obj \
	$(HL1_OBJ_DIR)\util.obj \
	$(HL1_OBJ_DIR)\view.obj \
	$(HL1_OBJ_DIR)\vgui_int.obj \
	$(HL1_OBJ_DIR)\vgui_ClassMenu.obj \
	$(HL1_OBJ_DIR)\vgui_ConsolePanel.obj \
	$(HL1_OBJ_DIR)\vgui_ControlConfigPanel.obj \
	$(HL1_OBJ_DIR)\vgui_CustomObjects.obj \
	$(HL1_OBJ_DIR)\vgui_MOTDWindow.obj \
	$(HL1_OBJ_DIR)\vgui_SchemeManager.obj \
	$(HL1_OBJ_DIR)\vgui_ScorePanel.obj \
	$(HL1_OBJ_DIR)\vgui_ServerBrowser.obj \
	$(HL1_OBJ_DIR)\vgui_TeamFortressViewport.obj \
	$(HL1_OBJ_DIR)\vgui_SpectatorPanel.obj \
	$(HL1_OBJ_DIR)\vgui_teammenu.obj \
	$(HL1_OBJ_DIR)\voice_status.obj \
	$(HL1_OBJ_DIR)\hud_servers.obj
	
HL1_HL_OBJS = \
	$(HL_HL_OBJ_DIR)\hl_baseentity.obj \
	$(HL_HL_OBJ_DIR)\hl_events.obj \
	$(HL_HL_OBJ_DIR)\hl_objects.obj \
	$(HL_HL_OBJ_DIR)\hl_weapons.obj

DLL_OBJS = \
	$(HL1_SERVER_OBJ_DIR)\crossbow.obj \
	$(HL1_SERVER_OBJ_DIR)\crowbar.obj \
	$(HL1_SERVER_OBJ_DIR)\egon.obj \
	$(HL1_SERVER_OBJ_DIR)\gauss.obj \
	$(HL1_SERVER_OBJ_DIR)\handgrenade.obj \
	$(HL1_SERVER_OBJ_DIR)\hornetgun.obj \
	$(HL1_SERVER_OBJ_DIR)\mp5.obj \
	$(HL1_SERVER_OBJ_DIR)\python.obj \
	$(HL1_SERVER_OBJ_DIR)\rpg.obj \
	$(HL1_SERVER_OBJ_DIR)\satchel.obj \
	$(HL1_SERVER_OBJ_DIR)\shotgun.obj \
	$(HL1_SERVER_OBJ_DIR)\squeakgrenade.obj \
	$(HL1_SERVER_OBJ_DIR)\tripmine.obj

WPN_SHARED_OBJS = \
	$(WPN_SHARED_OBJ_DIR)\hl_wpn_glock.obj

PUBLIC_OBJS = \
	$(PUBLIC_OBJ_DIR)\interface.obj \

COMMON_OBJS = \
	$(COMMON_OBJ_DIR)\parsemsg.obj \

GAME_SHARED_OBJS = \
	$(GAME_SHARED_OBJ_DIR)\vgui_checkbutton2.obj \
	$(GAME_SHARED_OBJ_DIR)\vgui_grid.obj \
	$(GAME_SHARED_OBJ_DIR)\vgui_helpers.obj \
	$(GAME_SHARED_OBJ_DIR)\vgui_listbox.obj \
	$(GAME_SHARED_OBJ_DIR)\vgui_loadtga.obj \
	$(GAME_SHARED_OBJ_DIR)\vgui_scrollbar2.obj \
	$(GAME_SHARED_OBJ_DIR)\vgui_slider2.obj \
	$(GAME_SHARED_OBJ_DIR)\voice_banmgr.obj
	
PM_SHARED_OBJS = \
	$(PM_SHARED_OBJ_DIR)\pm_debug.obj \
	$(PM_SHARED_OBJ_DIR)\pm_shared.obj \
	$(PM_SHARED_OBJ_DIR)\pm_math.obj
	

all : neat $(TARGET)


{$(HL_SRC_DIR)}.cpp{$(HL1_OBJ_DIR)}.obj :
	cl $** /c /Fo$(HL1_OBJ_DIR)\ $(CFLAGS) $(INCLUDEDIRS)
	
{$(HL_HL_SRC_DIR)}.cpp{$(HL_HL_OBJ_DIR)}.obj :
	cl $** /c /Fo$(HL_HL_OBJ_DIR)\ $(CFLAGS) $(INCLUDEDIRS)
	
{$(HL_SERVER_SRC_DIR)}.cpp{$(HL1_SERVER_OBJ_DIR)}.obj :
	cl $** /c /Fo$(HL1_SERVER_OBJ_DIR)\ $(CFLAGS) $(INCLUDEDIRS)

{$(WPN_SHARED_SRC_DIR)}.cpp{$(WPN_SHARED_OBJ_DIR)}.obj :
	cl $** /c /Fo$(WPN_SHARED_OBJ_DIR)\ $(CFLAGS) $(INCLUDEDIRS)

{$(PUBLIC_SRC_DIR)}.cpp{$(PUBLIC_OBJ_DIR)}.obj :
	cl $** /c /Fo$(PUBLIC_OBJ_DIR)\ $(CFLAGS) $(INCLUDEDIRS)

{$(COMMON_SRC_DIR)}.cpp{$(COMMON_OBJ_DIR)}.obj :
	cl $** /c /Fo$(COMMON_OBJ_DIR)\ $(CFLAGS) $(INCLUDEDIRS)

{$(GAME_SHARED_SRC_DIR)}.cpp{$(GAME_SHARED_OBJ_DIR)}.obj :
	cl $** /c /Fo$(GAME_SHARED_OBJ_DIR)\ $(CFLAGS) $(INCLUDEDIRS)
	
{$(PM_SHARED_SRC_DIR)}.c{$(PM_SHARED_OBJ_DIR)}.obj :
	cl Tc $** /c /Fo$(PM_SHARED_OBJ_DIR)\ $(CFLAGS) $(INCLUDEDIRS)
	
$(TARGET): $(HL1_OBJS) $(HL1_HL_OBJS) $(DLL_OBJS) $(WPN_SHARED_OBJS) $(PUBLIC_OBJS) $(COMMON_OBJS) $(GAME_SHARED_OBJS) $(PM_SHARED_OBJS)
	link $** $(LIBS) /OUT:$@ /DLL $(LDFLAGS)


install : all
	copy $(TARGET) "$(HL_DIR)\$(MOD_NAME)\cl_dlls\$(TARGET)"

neat :
	@if not exist $(MAIN_OBJ_DIR) mkdir $(MAIN_OBJ_DIR)
	@if not exist $(HL1_OBJ_DIR) mkdir $(HL1_OBJ_DIR)
	@if not exist $(HL_HL_OBJ_DIR) mkdir $(HL_HL_OBJ_DIR)
	@if not exist $(HL1_SERVER_OBJ_DIR) mkdir $(HL1_SERVER_OBJ_DIR)
	@if not exist $(WPN_SHARED_OBJ_DIR) mkdir $(WPN_SHARED_OBJ_DIR)
	@if not exist $(PUBLIC_OBJ_DIR) mkdir $(PUBLIC_OBJ_DIR)
	@if not exist $(COMMON_OBJ_DIR) mkdir $(COMMON_OBJ_DIR)
	@if not exist $(GAME_SHARED_OBJ_DIR) mkdir $(GAME_SHARED_OBJ_DIR)
	@if not exist $(PM_SHARED_OBJ_DIR) mkdir $(PM_SHARED_OBJ_DIR)
	
clean :
	@if exist $(HL1_OBJ_DIR)\*.obj del $(HL1_OBJ_DIR)\*.obj
	@if exist $(HL_HL_OBJ_DIR)\*.obj del $(HL_HL_OBJ_DIR)\*.obj
	@if exist $(HL1_SERVER_OBJ_DIR)\*.obj del $(HL1_SERVER_OBJ_DIR)\*.obj
	@if exist $(WPN_SHARED_OBJ_DIR)\*.obj del $(WPN_SHARED_OBJ_DIR)\*.obj
	@if exist $(PUBLIC_OBJ_DIR)\*.obj del $(PUBLIC_OBJ_DIR)\*.obj
	@if exist $(COMMON_OBJ_DIR)\*.obj del $(COMMON_OBJ_DIR)\*.obj
	@if exist $(GAME_SHARED_OBJ_DIR)\*.obj del $(GAME_SHARED_OBJ_DIR)\*.obj
	@if exist $(PM_SHARED_OBJ_DIR)\*.obj del $(PM_SHARED_OBJ_DIR)\*.obj
	@if exist $(TARGET) del $(TARGET)
	@if exist *.lib @del *.lib
	@if exist *.exp @del *.exp
	@if exist *.pdb @del *.pdb
	
spotless : clean
	@if exist $(HL1_OBJ_DIR) rmdir /S /Q $(HL1_OBJ_DIR)
	@if exist $(HL_HL_OBJ_DIR) rmdir /S /Q $(HL_HL_OBJ_DIR)
	@if exist $(HL1_SERVER_OBJ_DIR) rmdir /S /Q $(HL1_SERVER_OBJ_DIR)
	@if exist $(WPN_SHARED_OBJ_DIR) rmdir /S /Q $(WPN_SHARED_OBJ_DIR)
	@if exist $(PUBLIC_OBJ_DIR) rmdir /S /Q $(PUBLIC_OBJ_DIR)
	@if exist $(COMMON_OBJ_DIR) rmdir /S /Q $(COMMON_OBJ_DIR)
	@if exist $(GAME_SHARED_OBJ_DIR) rmdir /S /Q $(GAME_SHARED_OBJ_DIR)
	@if exist $(PM_SHARED_OBJ_DIR) rmdir /S /Q $(PM_SHARED_OBJ_DIR)
	@if exist $(MAIN_OBJ_DIR) rmdir /S /Q $(MAIN_OBJ_DIR)
