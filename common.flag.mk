DYNLINK ?= false

#=================================================
# For COMMON
#=================================================

COMMON_FLAGS     = -fPIC -g -Wall -Wextra
ifdef SP_RELEASE
  COMMON_FLAGS  += -DSP_RELEASE
endif

COMMON_IFLAGS     = -I$(SP_DIR)/h
COMMON_IFLAGS    += -I$(SP_DIR)/src
COMMON_IFLAGS    += -I$(DYNINST_DIR)
COMMON_IFLAGS    += -I$(DYNINST_DIR)/include

COMMON_LDFLAGS  += -L$(DYNINST_DIR)/lib
COMMON_LDFLAGS  += -lpatchAPI
COMMON_LDFLAGS  += -lstackwalk
COMMON_LDFLAGS   += -lpcontrol
COMMON_LDFLAGS  += -lparseAPI
COMMON_LDFLAGS  += -linstructionAPI
COMMON_LDFLAGS   += -lsymtabAPI
COMMON_LDFLAGS   += -lsymLite
COMMON_LDFLAGS   += -ldynDwarf
COMMON_LDFLAGS   += -ldynElf
COMMON_LDFLAGS   += -lcommon
COMMON_LDFLAGS   += -lboost_system

COMMON_LDFLAGS   += -lpthread -lthread_db

COMMON_LDFLAGS  += -ltbbmalloc


#COMMON_LDFLAGS   += -L$(LIBDWARF_LIB)
#COMMON_LDFLAGS   += -ldwarf

UTILS_DIR         = $(SP_DIR)/src/common

OBJS_DIR          = $(SP_DIR)/$(PLATFORM)/objs

#=================================================
# For Injector
#=================================================

IJ_DIR      = $(SP_DIR)/src/injector

IJ_FLAGS   += $(COMMON_FLAGS)
IJ_FLAGS   += -D_GNU_SOURCE

IJ_LDFLAGS  = $(COMMON_LDFLAGS)

IJ_IFLAGS  += $(COMMON_IFLAGS)
IJ_IFLAGS  += -I$(DYNINST_DIR)/include

IJA_FLAGS   += -fPIC

IJA_LDFLAGS += -ldl -shared

#=================================================
# For Agent
#=================================================

AG_DIR      = $(SP_DIR)/src/agent

AG_FLAGS   += $(COMMON_FLAGS)
AG_FLAGS   += -fPIC -Dos_linux -O0

AG_LDFLAGS  = $(COMMON_LDFLAGS)
#AG_LDFLAGS  += -L$(DYNINST_DIR)/stackwalk/$(PLATFORM)
#AG_LDFLAGS  += -lstackwalk
#AG_LDFLAGS  += -L$(DYNINST_DIR)/parseAPI/$(PLATFORM)
#AG_LDFLAGS  += -lparseAPI
#AG_LDFLAGS  += -L$(DYNINST_DIR)/patchAPI/$(PLATFORM)
#AG_LDFLAGS  += -lpatchAPI
#AG_LDFLAGS  += -L$(DYNINST_DIR)/instructionAPI/$(PLATFORM)
#AG_LDFLAGS  += -linstructionAPI

AG_IFLAGS  += $(COMMON_IFLAGS)
AG_IFLAGS  += $(IJ_IFLAGS)
AG_IFLAGS  += -I$(DYNINST_DIR)/include

#=================================================
# For system tests
#=================================================
ST_DIR      = $(SP_DIR)/src/test/systest
ML_DIR      = $(SP_DIR)/src/test/mutatee/lib
ME_DIR      = $(SP_DIR)/src/test/mutatee
TG_DIR      = $(SP_DIR)/src/test/agent
TC_DIR      = $(SP_DIR)/src/test/common
