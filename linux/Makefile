
#
# Half-life Makefile for x86 Linux
#
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

ARCH=i386
ELF-GC-DYNSTR=./elf-gc-dynstr

ifeq ($(OS),Linux)
	CC="/valve/bin/gcc-4.6 -m32"
	CPLUS="/valve/bin/g++-4.6 -m32"
	CPP_LIB:=-L$(shell /valve/bin/g++-4.6 -m32 -print-file-name=libstdc++.so | xargs dirname) -lstdc++ -ldl -lpthread
endif

ifeq ($(OS),Darwin)
    OSXVER := $(shell sw_vers -productVersion)
	DEVELOPER_DIR := $(shell /usr/bin/xcode-select -print-path)
	ifeq (,$(findstring 10.7, $(OSXVER))) 
		BUILDING_ON_LION := 0
		COMPILER_BIN_DIR := $(DEVELOPER_DIR)/usr/bin
		SDK_DIR := $(DEVELOPER_DIR)/SDKs
	else
		BUILDING_ON_LION := 1
		COMPILER_BIN_DIR := $(DEVELOPER_DIR)/Toolchains/XcodeDefault.xctoolchain/usr/bin
		SDK_DIR := $(DEVELOPER_DIR)/Platforms/MacOSX.platform/Developer/SDKs
	endif

	#SDKROOT ?= $(SDK_DIR)/MacOSX10.6.sdk
	SDKROOT ?= $(SDK_DIR)/MacOSX10.8.sdk

	ARCH_FLAGS ?= -arch i386 -m32 -march=prescott -gdwarf-2 -g2  -Wno-typedef-redefinition -momit-leaf-frame-pointer -mtune=core2
	CPP_LIB=-lstdc++ -lpthread

	ifeq ($(origin AR), default)
		AR = libtool -static -o
	endif
	ifeq ($(origin CC), default)
		CC ="$(COMPILER_BIN_DIR)/clang -Qunused-arguments -isysroot $(SDKROOT) -mmacosx-version-min=10.5 -fasm-blocks -I$(SDKROOT)/usr/include/malloc $(ARCH_FLAGS)"
	endif
	ifeq ($(origin CXX), default)
		CPLUS ="$(COMPILER_BIN_DIR)/clang++ -Qunused-arguments -isysroot $(SDKROOT) -mmacosx-version-min=10.5 -fasm-blocks -I$(SDKROOT)/usr/include/malloc $(ARCH_FLAGS)"
	endif
	LINK ?= $(CXX)
endif

CLINK=$(CC)

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
ARCH_CFLAGS_I486+=-march=pentium-m -mfpmath=387
ARCH_CFLAGS_I686+=-march=pentium-m -mfpmath=387
ARCH_CFLAGS_AMD+=-mfpmath=387
endif


ARCH_CFLAGS="$(ARCH_CFLAGS_I486)"

BASE_CFLAGS=-fpermissive -fno-strict-aliasing -DNDEBUG -DPOSIX -D_POSIX -DLINUX -D_LINUX -DGNUC -DHL1 -Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -DQUIVER -DQUAKE2 -DVALVE_DLL -D_alloca=alloca -fno-exceptions -fexpensive-optimizations -D_vsnprintf=vsnprintf -DNO_MALLOC_OVERRIDE -Werror=return-type
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

TARGETS=

TARGETS+= \
 	hl \
 	dmc \
 	ricochet \
	hl_cdll \
	dmc_cdll \
	ricochet_cdll \

build_dir:
	-mkdir $(BUILD_DIR);
	cd $(BUILD_DIR)

targets: $(TARGETS)


hl_cdll: build_dir
	$(MAKE_HL_CDLL) ARCH=i686 CC=$(CC) CPLUS=$(CPLUS) CPP_LIB="$(CPP_LIB)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) DBG_SRC_DIR=$(DBG_SRC_DIR) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)" GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) CLINK=$(CLINK) PM_SRC_DIR=$(PM_SRC_DIR) SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) CFG=$(CFG) OS=$(OS)

dmc_cdll: build_dir
	$(MAKE_DMC_CDLL) ARCH=i686 CC=$(CC) CPLUS=$(CPLUS) CPP_LIB="$(CPP_LIB)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) DBG_SRC_DIR=$(DBG_SRC_DIR) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)" GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) CLINK=$(CLINK) PM_SRC_DIR=$(PM_SRC_DIR) SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) CFG=$(CFG) OS=$(OS)

ricochet_cdll: build_dir
	$(MAKE_RICOCHET_CDLL) ARCH=i686 CC=$(CC) CPLUS=$(CPLUS) CPP_LIB="$(CPP_LIB)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) DBG_SRC_DIR=$(DBG_SRC_DIR) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)" GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) CLINK=$(CLINK) PM_SRC_DIR=$(PM_SRC_DIR) SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) CFG=$(CFG) OS=$(OS)

hl: build_dir
	$(MAKE_HL_LIB) CC=$(CC) ARCH=$(ARCH) CPP_LIB="$(CPP_LIB)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) PM_SRC_DIR=$(PM_SRC_DIR) GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) GAMEDB_SRC_DIR=$(GAMEDB_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) CFG=$(CFG) OS=$(OS) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)" 

dmc: build_dir
	$(MAKE_DMC_LIB) CC=$(CC) CPLUS=$(CPLUS) CPP_LIB="$(CPP_LIB)" ARCH=$(ARCH) BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) CFG=$(CFG) OS=$(OS)  CLINK=$(CLINK) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)"

ricochet: build_dir
	$(MAKE_RICOCHET_LIB) CC=$(CC) CPLUS=$(CPLUS) CPP_LIB="$(CPP_LIB)" ARCH=$(ARCH) BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) CFG=$(CFG) OS=$(OS)   CLINK=$(CLINK) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)"

clean:
	-rm -rf $(BUILD_OBJ_DIR)
