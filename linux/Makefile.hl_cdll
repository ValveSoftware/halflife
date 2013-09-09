#
# launcher Makefile for x86 Linux
#
#

HL_SRC_DIR=$(SOURCE_DIR)/cl_dll
HL_SERVER_SRC_DIR=$(SOURCE_DIR)/dlls
GAME_SHARED_SRC_DIR=$(SOURCE_DIR)/game_shared
PM_SHARED_SRC_DIR=$(SOURCE_DIR)/pm_shared

HL1_OBJ_DIR=$(BUILD_OBJ_DIR)/hl1_client
PUBLIC_OBJ_DIR=$(HL1_OBJ_DIR)/public
COMMON_OBJ_DIR=$(HL1_OBJ_DIR)/common
GAME_SHARED_OBJ_DIR=$(HL1_OBJ_DIR)/game_shared
HL1_SERVER_OBJ_DIR=$(HL1_OBJ_DIR)/server
PM_SHARED_OBJ_DIR=$(HL1_OBJ_DIR)/pm_shared

CFLAGS=$(BASE_CFLAGS) $(ARCH_CFLAGS) -DCLIENT_DLL -DCLIENT_WEAPONS -DHL_DLL -I/usr/include/malloc -D_snwprintf=swprintf \
		 -DDISABLE_JUMP_ORIGIN -DDISABLE_VEC_ORIGIN
		 
INCLUDEDIRS=-I$(HL_SRC_DIR) -I../dlls -I../tfc -I$(COMMON_SRC_DIR) -I$(PUBLIC_SRC_DIR) -I../pm_shared -I../engine -I../utils/vgui/include -I ../game_shared -I../external

ifeq ($(OS),Darwin)
LDFLAGS=$(SHLIBLDFLAGS) $(CPP_LIB) -framework Carbon $(CFG)/vgui.dylib -L. -lSDL2-2.0.0
else
LDFLAGS=$(SHLIBLDFLAGS) $(CPP_LIB)  -L$(CFG) vgui.so -L. libSDL2-2.0.so.0
endif

DO_CC=$(CPLUS) $(INCLUDEDIRS) $(CFLAGS) -o $@ -c $<
DO_PUBLIC_CC=$(CPLUS) $(COMMON_INCLUDEDIRS) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_COMMON_CC=$(CPLUS) $(INCLUDEDIRS)  $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_PM_SHARED_CC=$(CC) $(INCLUDEDIRS)  $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

#####################################################################

HL1_OBJS = \
	$(HL1_OBJ_DIR)/hud_spectator.o \
	$(HL1_OBJ_DIR)/ev_hldm.o \
	$(HL1_OBJ_DIR)/hl/hl_baseentity.o \
	$(HL1_OBJ_DIR)/hl/hl_events.o \
	$(HL1_OBJ_DIR)/hl/hl_objects.o \
	$(HL1_OBJ_DIR)/hl/hl_weapons.o \
	$(HL1_OBJ_DIR)/hud.o \
	$(HL1_OBJ_DIR)/inputw32.o \
	$(HL1_OBJ_DIR)/ammo.o \
	$(HL1_OBJ_DIR)/ammo_secondary.o \
	$(HL1_OBJ_DIR)/ammohistory.o \
	$(HL1_OBJ_DIR)/battery.o \
	$(HL1_OBJ_DIR)/cdll_int.o \
	$(HL1_OBJ_DIR)/com_weapons.o \
	$(HL1_OBJ_DIR)/death.o \
	$(HL1_OBJ_DIR)/demo.o \
	$(HL1_OBJ_DIR)/entity.o \
	$(HL1_OBJ_DIR)/ev_common.o \
	$(HL1_OBJ_DIR)/events.o \
	$(HL1_OBJ_DIR)/flashlight.o \
	$(HL1_OBJ_DIR)/GameStudioModelRenderer.o \
	$(HL1_OBJ_DIR)/geiger.o \
	$(HL1_OBJ_DIR)/health.o \
	$(HL1_OBJ_DIR)/hud_bench.o \
	$(HL1_OBJ_DIR)/hud_benchtrace.o \
	$(HL1_OBJ_DIR)/hud_msg.o \
	$(HL1_OBJ_DIR)/hud_redraw.o \
	$(HL1_OBJ_DIR)/hud_update.o \
	$(HL1_OBJ_DIR)/in_camera.o \
	$(HL1_OBJ_DIR)/input.o \
	$(HL1_OBJ_DIR)/interpolation.o \
	$(HL1_OBJ_DIR)/menu.o \
	$(HL1_OBJ_DIR)/message.o \
	$(HL1_OBJ_DIR)/saytext.o \
	$(HL1_OBJ_DIR)/status_icons.o \
	$(HL1_OBJ_DIR)/statusbar.o \
	$(HL1_OBJ_DIR)/studio_util.o \
	$(HL1_OBJ_DIR)/StudioModelRenderer.o \
	$(HL1_OBJ_DIR)/text_message.o \
	$(HL1_OBJ_DIR)/train.o \
	$(HL1_OBJ_DIR)/tri.o \
	$(HL1_OBJ_DIR)/util.o \
	$(HL1_OBJ_DIR)/view.o \
	$(HL1_OBJ_DIR)/vgui_int.o \
	$(HL1_OBJ_DIR)/vgui_ClassMenu.o \
	$(HL1_OBJ_DIR)/vgui_ConsolePanel.o \
	$(HL1_OBJ_DIR)/vgui_ControlConfigPanel.o \
	$(HL1_OBJ_DIR)/vgui_CustomObjects.o \
	$(HL1_OBJ_DIR)/vgui_MOTDWindow.o \
	$(HL1_OBJ_DIR)/vgui_SchemeManager.o \
	$(HL1_OBJ_DIR)/vgui_ScorePanel.o \
	$(HL1_OBJ_DIR)/vgui_ServerBrowser.o \
	$(HL1_OBJ_DIR)/vgui_TeamFortressViewport.o \
	$(HL1_OBJ_DIR)/vgui_SpectatorPanel.o \
	$(HL1_OBJ_DIR)/vgui_teammenu.o \
	$(HL1_OBJ_DIR)/voice_status.o \
	$(HL1_OBJ_DIR)/hud_servers.o \
	

DLL_OBJS = \
	$(HL1_SERVER_OBJ_DIR)/crossbow.o \
	$(HL1_SERVER_OBJ_DIR)/crowbar.o \
	$(HL1_SERVER_OBJ_DIR)/egon.o \
	$(HL1_SERVER_OBJ_DIR)/gauss.o \
	$(HL1_SERVER_OBJ_DIR)/handgrenade.o \
	$(HL1_SERVER_OBJ_DIR)/hornetgun.o \
	$(HL1_SERVER_OBJ_DIR)/mp5.o \
	$(HL1_SERVER_OBJ_DIR)/python.o \
	$(HL1_SERVER_OBJ_DIR)/rpg.o \
	$(HL1_SERVER_OBJ_DIR)/satchel.o \
	$(HL1_SERVER_OBJ_DIR)/shotgun.o \
	$(HL1_SERVER_OBJ_DIR)/squeakgrenade.o \
	$(HL1_SERVER_OBJ_DIR)/tripmine.o \
	$(HL1_SERVER_OBJ_DIR)/wpn_shared/hl_wpn_glock.o \


PUBLIC_OBJS = \
	$(PUBLIC_OBJ_DIR)/interface.o \

COMMON_OBJS = \
	$(COMMON_OBJ_DIR)/parsemsg.o \

GAME_SHARED_OBJS = \
	$(GAME_SHARED_OBJ_DIR)/vgui_checkbutton2.o \
	$(GAME_SHARED_OBJ_DIR)/vgui_grid.o \
	$(GAME_SHARED_OBJ_DIR)/vgui_helpers.o \
	$(GAME_SHARED_OBJ_DIR)/vgui_listbox.o \
	$(GAME_SHARED_OBJ_DIR)/vgui_loadtga.o \
	$(GAME_SHARED_OBJ_DIR)/vgui_scrollbar2.o \
	$(GAME_SHARED_OBJ_DIR)/vgui_slider2.o \
	$(GAME_SHARED_OBJ_DIR)/voice_banmgr.o \
	
PM_SHARED_OBJS = \
	$(PM_SHARED_OBJ_DIR)/pm_debug.o \
	$(PM_SHARED_OBJ_DIR)/pm_shared.o \
	$(PM_SHARED_OBJ_DIR)/pm_math.o \
	


all: client.$(SHLIBEXT)

client.$(SHLIBEXT): $(HL1_OBJS) $(PUBLIC_OBJS) $(COMMON_OBJS) $(GAME_SHARED_OBJS) $(DLL_OBJS) $(PM_SHARED_OBJS)
	$(CLINK) -o $(BUILD_DIR)/$@ $(HL1_OBJS) $(PUBLIC_OBJS) $(COMMON_OBJS) $(GAME_SHARED_OBJS) $(DLL_OBJS) $(PM_SHARED_OBJS) $(LDFLAGS) $(CPP_LIB)
	p4 edit ../../game/mod/cl_dlls/$@
	cp $(BUILD_DIR)/$@  ../../game/mod/cl_dlls
	./gendbg.sh ../../game/mod/cl_dlls/client.$(SHLIBEXT)

$(HL1_OBJ_DIR):
	mkdir -p $(HL1_OBJ_DIR)
	mkdir -p $(HL1_OBJ_DIR)/hl
	mkdir -p $(HL1_OBJ_DIR)/dlls/wpn_shared
	mkdir -p $(HL1_OBJ_DIR)/VGUI

$(HL1_SERVER_OBJ_DIR):
	mkdir -p $(HL1_SERVER_OBJ_DIR)
	mkdir -p $(HL1_SERVER_OBJ_DIR)/wpn_shared

$(PUBLIC_OBJ_DIR):
	mkdir -p $(PUBLIC_OBJ_DIR)

$(COMMON_OBJ_DIR):
	mkdir -p $(COMMON_OBJ_DIR)

$(GAME_SHARED_OBJ_DIR):
	mkdir -p $(GAME_SHARED_OBJ_DIR)

$(PM_SHARED_OBJ_DIR):
	mkdir -p $(PM_SHARED_OBJ_DIR)

$(HL1_OBJ_DIR)/%.o: $(HL_SRC_DIR)/%.cpp $(filter-out $(wildcard  $(HL1_OBJ_DIR)),  $(HL1_OBJ_DIR))
	$(DO_CC)

$(HL1_SERVER_OBJ_DIR)/%.o: $(HL_SERVER_SRC_DIR)/%.cpp $(filter-out $(wildcard  $(HL1_SERVER_OBJ_DIR)),  $(HL1_SERVER_OBJ_DIR))
	$(DO_CC)

$(HL1_OBJ_DIR)/%.o: $(HL_SRC_DIR)/hl/%.cpp $(filter-out $(wildcard  $(HL1_OBJ_DIR)),  $(HL1_OBJ_DIR))
	$(DO_CC)

$(HL1_OBJ_DIR)/%.o: $(HL_SRC_DIR)/dlls/wpn_shared/%.cpp $(filter-out $(wildcard  $(HL1_OBJ_DIR)),  $(HL1_OBJ_DIR))
	$(DO_CC)

$(HL1_OBJ_DIR)/%.o: $(HL_SRC_DIR)/VGUI/%.cpp $(filter-out $(wildcard  $(HL1_OBJ_DIR)),  $(HL1_OBJ_DIR))
	$(DO_CC)

$(PUBLIC_OBJ_DIR)/%.o : $(PUBLIC_SRC_DIR)/%.cpp $(filter-out $(wildcard  $(PUBLIC_OBJ_DIR)),  $(PUBLIC_OBJ_DIR))
	$(DO_PUBLIC_CC)

$(COMMON_OBJ_DIR)/%.o : $(COMMON_SRC_DIR)/%.cpp $(filter-out $(wildcard  $(COMMON_OBJ_DIR)),  $(COMMON_OBJ_DIR))
	$(DO_COMMON_CC)

$(GAME_SHARED_OBJ_DIR)/%.o : $(GAME_SHARED_SRC_DIR)/%.cpp $(filter-out $(wildcard  $(GAME_SHARED_OBJ_DIR)),  $(GAME_SHARED_OBJ_DIR))
	$(DO_COMMON_CC)

$(PM_SHARED_OBJ_DIR)/%.o : $(PM_SHARED_SRC_DIR)/%.c $(filter-out $(wildcard  $(PM_SHARED_OBJ_DIR)),  $(PM_SHARED_OBJ_DIR))
	$(DO_PM_SHARED_CC)

clean:
	-rm -rf $(HL1_OBJ_DIR)
	-rm -f client.$(SHLIBEXT)
