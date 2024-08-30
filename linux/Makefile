
#
# Half-life Makefile for x86 Linux
#
# Feb 2001 by Leon Hartwig (hartwig@valvesoftware.com)
#

OS:=$(shell uname)
HOSTNAME:=$(shell hostname)

ifeq "$(CFG)" ""
CFG=release
endif

SOURCE_DIR=..
BUILD_DIR=$(CFG)

ENGINE_SRC_DIR=$(SOURCE_DIR)/engine
COMMON_SRC_DIR=$(SOURCE_DIR)/common
PM_SRC_DIR=$(SOURCE_DIR)/pm_shared
GAME_SHARED_SRC_DIR=$(SOURCE_DIR)/game_shared
GAMEDB_SRC_DIR=$(COMMON_SRC_DIR)/gamedb
PUBLIC_SRC_DIR=$(SOURCE_DIR)/public
DBG_SRC_DIR=$(SOURCE_DIR)/dbg

BUILD_OBJ_DIR=$(BUILD_DIR)/obj

P4_EDIT_START = chmod -R +w
P4_EDIT_END = || true
P4_REVERT_START = true
P4_REVERT_END =

ARCH=i386
ELF-GC-DYNSTR=./elf-gc-dynstr

# Note that we expect to be building inside a Steam Runtime "scout" container
ifeq ($(OS),Linux)
		CC="gcc-5 -m32 -std=gnu++11 -fno-delete-null-pointer-checks -fno-gnu-unique -fvisibility=hidden"
		CPLUS="g++-5 -m32 -std=gnu++11 -fno-delete-null-pointer-checks -fno-gnu-unique -fvisibility=hidden"
		CPP_LIB:=-L$(shell g++-5 -m32 -print-file-name=libstdc++.so | xargs dirname) -lstdc++ -ldl -lpthread
				CLINK=$(CC)
endif

ifeq ($(OS),Darwin)
    OSXVER := $(shell sw_vers -productVersion)
	DEVELOPER_DIR := $(shell /usr/bin/xcode-select -print-path)
	BUILDING_ON_LION := 1
	COMPILER_BIN_DIR := $(DEVELOPER_DIR)/Toolchains/XcodeDefault.xctoolchain/usr/bin
	SDK_DIR := $(DEVELOPER_DIR)/Platforms/MacOSX.platform/Developer/SDKs

   	SDKROOT ?= $(SDK_DIR)/MacOSX10.7.sdk

	ARCH_FLAGS ?= -arch i386 -m32 -march=prescott -gdwarf-2 -g2  -Wno-typedef-redefinition -momit-leaf-frame-pointer -mtune=core2
	CPP_LIB=-lpthread

    ifeq ($(origin AR), default)
		AR = libtool -static -o
	endif
	ifeq ($(origin CC), default)
		CC ="$(COMPILER_BIN_DIR)/clang -Qunused-arguments -isysroot $(SDKROOT) -mmacosx-version-min=10.7 -fasm-blocks -I$(SDKROOT)/usr/include/malloc $(ARCH_FLAGS)"
	endif
	ifeq ($(origin CXX), default)
		CPLUS ="$(COMPILER_BIN_DIR)/clang++ -Qunused-arguments -isysroot $(SDKROOT) -mmacosx-version-min=10.7 -fasm-blocks -I$(SDKROOT)/usr/include/malloc $(ARCH_FLAGS)"
	endif
	LINK?=$(CPLUS)
    	CLINK = $(CPLUS)
endif


ifeq "$(CFG)" "release"
ARCH_CFLAGS_I486=-O3 -gdwarf-2 -g2 
ARCH_CFLAGS_I686=-O3 -gdwarf-2 -g2 
ARCH_CFLAGS_AMD=-O3 -gdwarf-2 -g2 
ARCH_CFLAGS_AMD64=-m64 -O3 -gdwarf-2 -g2 
else
ARCH_CFLAGS_I486=-gdwarf-2 -g2 
ARCH_CFLAGS_I686=-gdwarf-2 -g2 
ARCH_CFLAGS_AMD=-gdwarf-2 -g2 
ARCH_CFLAGS_AMD64=-m64 -gdwarf-2 -g2 
endif

ifeq ($(OS),Linux)
ARCH_CFLAGS_I486+=-march=pentium-m -mfpmath=387
ARCH_CFLAGS_I686+=-march=pentium-m -mfpmath=387
ARCH_CFLAGS_AMD+=-march=k6 -mfpmath=387
endif

ifeq ($(OS),Darwin)
# force 387 for FP math so the precision between win32 and linux and osx match
ARCH_CFLAGS_I486+=-mno-sse -march=pentium-m -mfpmath=387 -D__m128i=uint64_t -D__m128d=uint64_t -D__m128=uint64_t
ARCH_CFLAGS_I686+=-mno-sse -march=pentium-m -mfpmath=387 -D__m128i=uint64_t -D__m128d=uint64_t -D__m128=uint64_t
ARCH_CFLAGS_AMD+=-mfpmath=387
endif


ARCH_CFLAGS="$(ARCH_CFLAGS_I486)"

BASE_CFLAGS=-fpermissive -fno-strict-aliasing -DNDEBUG -DPOSIX -D_POSIX -DLINUX -D_LINUX -DGNUC -DHL1 -Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -DQUIVER -DQUAKE2 -DVALVE_DLL -D_alloca=alloca -fexpensive-optimizations -D_vsnprintf=vsnprintf -DNO_MALLOC_OVERRIDE -Werror=return-type
BASE_CFLAGS+=-w

ifeq ($(OS),Darwin)
	BASE_CFLAGS += -DOSX -D_OSX -fvisibility=hidden
else
	BASE_CFLAGS+= -DLINUX -D_LINUX
endif

DEDICATED_CFLAGS="-DDEDICATED -DSWDS"
ifeq ($(OS),Darwin)
SHLIBEXT=dylib
SHLIBCFLAGS=
SHLIBLDFLAGS="-shared -gdwarf-2 -g2"
else
SHLIBEXT=so
SHLIBCFLAGS=
SHLIBLDFLAGS="-shared -gdwarf-2 -g2 -Wl,-Map,$@_map.txt"
endif
AR=ar
LIBEXT=a
MAKE+= -j8

MAKE_HL_LIB=$(MAKE) -f Makefile.hldll
MAKE_DMC_LIB=$(MAKE) -f Makefile.dmcdll
MAKE_RICOCHET_LIB=$(MAKE) -f Makefile.ricochetdll
MAKE_HL_CDLL=$(MAKE) -f Makefile.hl_cdll
MAKE_DMC_CDLL=$(MAKE) -f Makefile.dmc_cdll
MAKE_RICOCHET_CDLL=$(MAKE) -f Makefile.ricochet_cdll

#############################################################################
# SETUP AND BUILD
#############################################################################

all: build_dir targets

amd64: override ARCH=amd64
amd64: override CC=/usr/bin/gcc
amd64: override CPLUS=/usr/bin/g++
amd64: override CPP_LIB = /usr/lib64/libstdc++.a
amd64: override ARCH_CFLAGS=$(ARCH_CFLAGS_AMD64)
amd64: override ELF-GC-DYNSTR=echo
amd64: build_dir targets_amd64

TARGETS_AMD64 = \
	hlds_amd64 \
	engine_amd64 \
	stdio \
	hl \
	cs \
 	
TARGETS=

TARGETS+= \
 	hl \
 	dmc \
 	ricochet \
	hl_cdll \
	dmc_cdll \
	ricochet_cdll \

build_dir:
	-mkdir -p $(BUILD_DIR);
	cd $(BUILD_DIR)

targets: $(TARGETS)

targets_amd64: $(TARGETS_AMD64)


hl_cdll: build_dir
	$(MAKE_HL_CDLL) ARCH=i686 P4_EDIT_START="$(P4_EDIT_START)" P4_EDIT_END="$(P4_EDIT_END)" P4_REVERT_START="$(P4_REVERT_START)" P4_REVERT_END="$(P4_REVERT_END)" CC=$(CC) CPLUS=$(CPLUS) CPP_LIB="$(CPP_LIB)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) DBG_SRC_DIR=$(DBG_SRC_DIR) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)" GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) CLINK=$(CLINK) PM_SRC_DIR=$(PM_SRC_DIR) SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) CFG=$(CFG) OS=$(OS)

dmc_cdll: build_dir
	$(MAKE_DMC_CDLL) ARCH=i686 P4_EDIT_START="$(P4_EDIT_START)" P4_EDIT_END="$(P4_EDIT_END)" P4_REVERT_START="$(P4_REVERT_START)" P4_REVERT_END="$(P4_REVERT_END)" CC=$(CC) CPLUS=$(CPLUS) CPP_LIB="$(CPP_LIB)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) DBG_SRC_DIR=$(DBG_SRC_DIR) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)" GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) CLINK=$(CLINK) PM_SRC_DIR=$(PM_SRC_DIR) SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) CFG=$(CFG) OS=$(OS)

ricochet_cdll:
	$(MAKE_RICOCHET_CDLL) ARCH=i686 P4_EDIT_START="$(P4_EDIT_START)" P4_EDIT_END="$(P4_EDIT_END)" P4_REVERT_START="$(P4_REVERT_START)" P4_REVERT_END="$(P4_REVERT_END)" CC=$(CC) CPLUS=$(CPLUS) CPP_LIB="$(CPP_LIB)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) DBG_SRC_DIR=$(DBG_SRC_DIR) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)" GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) CLINK=$(CLINK) PM_SRC_DIR=$(PM_SRC_DIR) SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) CFG=$(CFG) OS=$(OS)

hl: build_dir
	$(MAKE_HL_LIB) P4_EDIT_START="$(P4_EDIT_START)" P4_EDIT_END="$(P4_EDIT_END)" P4_REVERT_START="$(P4_REVERT_START)" P4_REVERT_END="$(P4_REVERT_END)" CC=$(CC) ARCH=$(ARCH) CPP_LIB="$(CPP_LIB)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) PM_SRC_DIR=$(PM_SRC_DIR) GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) GAMEDB_SRC_DIR=$(GAMEDB_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) CFG=$(CFG) OS=$(OS) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)" CLINK=$(CLINK)

dmc: build_dir
	$(MAKE_DMC_LIB) P4_EDIT_START="$(P4_EDIT_START)" P4_EDIT_END="$(P4_EDIT_END)" P4_REVERT_START="$(P4_REVERT_START)" P4_REVERT_END="$(P4_REVERT_END)" CC=$(CC) CPLUS=$(CPLUS) CPP_LIB="$(CPP_LIB)" ARCH=$(ARCH) BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) CFG=$(CFG) OS=$(OS)  CLINK=$(CLINK) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)"

ricochet: build_dir
	$(MAKE_RICOCHET_LIB) P4_EDIT_START="$(P4_EDIT_START)" P4_EDIT_END="$(P4_EDIT_END)" P4_REVERT_START="$(P4_REVERT_START)" P4_REVERT_END="$(P4_REVERT_END)" CC=$(CC) CPLUS=$(CPLUS) CPP_LIB="$(CPP_LIB)" ARCH=$(ARCH) BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) CFG=$(CFG) OS=$(OS)   CLINK=$(CLINK) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)"

clean:
	$(MAKE_HL_CDLL) BUILD_DIR=$(BUILD_DIR) ARCH=$(ARCH) CFG=$(CFG) OS=$(OS) SHLIBEXT=$(SHLIBEXT) clean
	$(MAKE_DMC_CDLL) BUILD_DIR=$(BUILD_DIR) ARCH=$(ARCH) CFG=$(CFG) OS=$(OS) SHLIBEXT=$(SHLIBEXT) clean
	$(MAKE_RICOCHET_CDLL) BUILD_DIR=$(BUILD_DIR) ARCH=$(ARCH) CFG=$(CFG) OS=$(OS) SHLIBEXT=$(SHLIBEXT) clean
	$(MAKE_HL_LIB) BUILD_DIR=$(BUILD_DIR) ARCH=$(ARCH) CFG=$(CFG) OS=$(OS) SHLIBEXT=$(SHLIBEXT) clean
	$(MAKE_DMC_LIB) BUILD_DIR=$(BUILD_DIR) ARCH=$(ARCH) CFG=$(CFG) OS=$(OS) SHLIBEXT=$(SHLIBEXT) clean
	$(MAKE_RICOCHET_LIB) BUILD_DIR=$(BUILD_DIR) ARCH=$(ARCH) CFG=$(CFG) OS=$(OS) SHLIBEXT=$(SHLIBEXT) clean
	-rm -rf $(BUILD_OBJ_DIR)
