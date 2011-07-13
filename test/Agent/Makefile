include ../../make.config
include ../../make.flag.tmpl

GCC        = gcc
GXX        = g++
RM         = rm -f
TESTCASE_DIR = TestCases

# Test driver
TEST_DRIVER           = test
DRIVER_SRCS           = $(wildcard *.C)
DRIVER_LDFLAGS        = -ldl

# Agent
AGENT_SRCS            = $(wildcard TestCases/*agent.C)
AGENT_SO              = $(notdir $(AGENT_SRCS:%.C=%.so))

AG_LDFLAGS           += -L$(SP_DIR)/$(PLATFORM)
AG_LDFLAGS           += -lagent

# Mutatee
MUTATEE_SRCS          = $(wildcard TestCases/*mutatee.c)
MUTATEE_SO            = $(notdir $(MUTATEE_SRCS:%.c=%.so))
MUTATEE_LDFLAGS       = -shared -fPIC
MUTATEE_IFLAGS        =
MUTATEE_FLAGS         =

# Rules
#.PHONY: all
all: MUTATEE AGENT DRIVER
	@echo ""
	@echo "====================="
	@echo " Usage: sh run.sh "
	@echo "====================="
AGENT: $(AGENT_SO)
	@echo "* All agents are done"
$(AGENT_SO): %.so : $(AGENT_SRCS)
	@echo "Building $@"
	@$(GXX) -o $(PLATFORM)/$@ $(TESTCASE_DIR)/$*.C $(AG_LDFLAGS) $(AG_IFLAGS) $(AG_FLAGS)

MUTATEE: $(MUTATEE_SO)
	@echo "* All mutatees are done"
$(MUTATEE_SO): %.so : $(MUTATEE_SRCS)
	@echo "Building $@"
	@$(GCC) -o $(PLATFORM)/$@ $(TESTCASE_DIR)/$*.c $(MUTATEE_LDFLAGS) $(MUTATEE_IFLAGS) $(MUTATEE_FLAGS)

DRIVER: $(TEST_DRIVER)
	@echo "* Test driver is done"
$(TEST_DRIVER): $(DRIVER_SRCS)
	@echo "Building $@"
	@$(GXX) -o $(PLATFORM)/$@ $(DRIVER_SRCS) $(DRIVER_LDFLAGS) $(DRIVER_IFLAGS) $(DRIVER_FLAGS)

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
