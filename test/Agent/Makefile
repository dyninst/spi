DYNINST_DIR  = ../../../dyninst/dyninst
GXX          = g++
GCC          = gcc
TESTCASE_DIR = TestCases

# Test driver
TEST_DRIVER           = test
DRIVER_SRCS           = $(wildcard *.C)
DRIVER_LDFLAGS        = -ldl

# Agent
AGENT_SRCS            = $(wildcard TestCases/*agent.C)
AGENT_SO              = $(notdir $(AGENT_SRCS:%.C=%.so))
AGENT_LDFLAGS         = -shared -fPIC
AGENT_LDFLAGS        += -L$(DYNINST_DIR)/symtabAPI/$(PLATFORM) \
                        -L$(DYNINST_DIR)/parseAPI/$(PLATFORM) \
                        -L$(DYNINST_DIR)/patchAPI/$(PLATFORM) \
                        -L$(DYNINST_DIR)/stackwalk/$(PLATFORM) \
                        -L$(DYNINST_DIR)/instructionAPI/$(PLATFORM) \
                        -L../../$(PLATFORM) \

AGENT_LDFLAGS        += -lsymtabAPI \
                        -lparseAPI  \
                        -lpatchAPI  \
                        -lstackwalk \
                        -linstructionAPI  \
                        -lagent     \

AGENT_IFLAGS          = -I$(DYNINST_DIR)/symtabAPI/h \
                        -I$(DYNINST_DIR)/parseAPI/h \
                        -I$(DYNINST_DIR) \
                        -I$(DYNINST_DIR)/common/h \
                        -I$(DYNINST_DIR)/dynutil/h \
                        -I$(DYNINST_DIR)/patchAPI/h \
                        -I$(DYNINST_DIR)/stackwalk/h \
                        -I$(DYNINST_DIR)/instructionAPI/h \
                        -I../../h

AGENT_FLAGS           = -Dos_linux

# Mutatee
MUTATEE_SRCS          = $(wildcard TestCases/*mutatee.c)
MUTATEE_SO            = $(notdir $(MUTATEE_SRCS:%.c=%.so))
MUTATEE_LDFLAGS       = -shared -fPIC
MUTATEE_IFLAGS        =
MUTATEE_FLAGS         =

# Rules
#.PHONY: all
all: $(MUTATEE_SO) $(AGENT_SO) $(TEST_DRIVER)

$(AGENT_SO): %.so : $(AGENT_SRCS)
	$(GXX) -o $(PLATFORM)/$@ $(TESTCASE_DIR)/$*.C $(AGENT_LDFLAGS) $(AGENT_IFLAGS) $(AGENT_FLAGS)

$(MUTATEE_SO): %.so : $(MUTATEE_SRCS)
	$(GCC) -o $(PLATFORM)/$@ $(TESTCASE_DIR)/$*.c $(MUTATEE_LDFLAGS) $(MUTATEE_IFLAGS) $(MUTATEE_FLAGS)

$(TEST_DRIVER): $(DRIVER_SRCS)
	$(GXX) -o $(PLATFORM)/$@ $(DRIVER_SRCS) $(DRIVER_LDFLAGS) $(DRIVER_IFLAGS) $(DRIVER_FLAGS)

# Clean up
clean:
	@rm -f $(PLATFORM)/*.so $(PLATFORM)/$(TEST_DRIVER)

# Debug Makefile
d:
	@echo "AGENT_SRCS    =$(AGENT_SRCS)"
	@echo "MUTATEE_SRCS  =$(MUTATEE_SRCS)"
	@echo "PLATFORM      =$(PLATFORM)"
	@echo "AGENT_SO      =$(AGENT_SO)"
	@echo "MUTATEE_SO    =$(MUTATEE_SO)"
