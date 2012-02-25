
INJECTOR   = injector
IJAGENT    = libijagent.so

ifeq ($(DYNLINK), false)
AGENT      = libagent.a
else
AGENT      = libagent.so
endif

all: test injector_exe agent_lib

spi: injector_exe agent_lib

GCC        = gcc
GXX        = g++
RM         = rm -f
MKDIR      = mkdir -p

VPATH       = $(AG_DIR) \
              $(AG_DIR)/inst_workers \
              $(AG_DIR)/ipc \
              $(AG_DIR)/ipc/ipc_workers \
              $(AG_DIR)/patchapi \
              $(IJ_DIR) \
              $(UTILS_DIR) \

#======================================
# Utils
#======================================

UTILS_SRCS    += $(UTILS_DIR)/utils.cc
UTILS_OBJS    += $(addprefix $(OBJS_DIR)/, $(notdir $(UTILS_SRCS:%.cc=%.o)))

$(UTILS_OBJS): $(OBJS_DIR)/%.o : %.cc
	@echo Compiling $*.o
	@$(MKDIR) $(OBJS_DIR)
	@$(GCC) -c -o $@ $(COMMON_IFLAGS) $(COMMON_FLAGS) $<

#======================================
# Injector
#======================================

injector_exe: $(INJECTOR) $(IJAGENT)

#---------------- 
# injector objs
#---------------- 
IJ_SRCS    += $(IJ_DIR)/injector.cc
IJ_OBJS    += $(addprefix $(OBJS_DIR)/, $(notdir $(IJ_SRCS:%.cc=%.o)))
IJ_DEP      = .IJ_DEPENDS

$(shell if test -f $(IJ_DEP); then true; else \
$(GXX) -MM $(IJ_IFLAGS) $(IJ_FLAGS) $(IJ_SRCS) > /tmp/t; \
perl -ane 'print "\$(OBJS_DIR)/" if/\w+.o: /; print;' < /tmp/t > $(IJ_DEP); \
fi);

include $(IJ_DEP)

$(IJ_OBJS): $(OBJS_DIR)/%.o : %.cc
	@echo Compiling $*.o
	@$(MKDIR) $(OBJS_DIR)
	@$(GXX) -c -o $@ $(IJ_IFLAGS) $(IJ_FLAGS) $<

#---------------- 
# Injector Exe
#---------------- 
IJMAIN_SRCS    += $(IJ_DIR)/injector_main.cc
IJMAIN_OBJS    += 
IJMAIN_OBJS    += $(addprefix $(OBJS_DIR)/, $(notdir $(IJMAIN_SRCS:%.cc=%.o)))
IJMAIN_DEP      = .IJMAIN_DEPENDS

$(shell if test -f $(IJMAIN_DEP); then true; else \
$(GXX) -MM $(IJ_IFLAGS) $(IJ_FLAGS) $(IJMAIN_SRCS) > /tmp/t; \
perl -ane 'print "\$(OBJS_DIR)/" if/\w+.o: /; print;' < /tmp/t > $(IJMAIN_DEP); \
fi);

include $(IJMAIN_DEP)

$(IJMAIN_OBJS): $(OBJS_DIR)/%.o : %.cc
	@echo Compiling $*.o
	@$(MKDIR) $(OBJS_DIR)
	@$(GXX) -c -o $@ $(IJ_IFLAGS) $(IJ_FLAGS) $<

$(INJECTOR): $(IJMAIN_OBJS) $(IJ_OBJS) $(UTILS_OBJS)
	@echo Linking $@
	@$(GXX) -o $@ $(IJMAIN_OBJS) $(IJ_OBJS) $(UTILS_OBJS) $(IJ_LDFLAGS)

#---------------- 
# libijagent.so
#---------------- 
IJA_SRCS     += $(IJ_DIR)/ijagent.c
IJA_OBJS  = $(addprefix $(OBJS_DIR)/, $(notdir $(IJA_SRCS:%.c=%.o)))

$(IJA_OBJS): $(OBJS_DIR)/%.o : $(IJ_DIR)/%.c
	@echo Compiling $*.o
	@$(MKDIR) $(OBJS_DIR)
	@$(GCC) -c -o $@ $(IJA_IFLAGS) $(IJA_FLAGS) $(IJ_DIR)/$*.c

$(IJAGENT): $(IJA_OBJS)
	@echo Linking $@
	@$(GCC) -o $@ $(IJA_OBJS) $(IJA_LDFLAGS)

#======================================
# Agent
#======================================
AG_SRCS    += $(AG_DIR)/agent.cc \
              $(AG_DIR)/context.cc \
              $(AG_DIR)/event.cc \
              $(AG_DIR)/parser.cc \
              $(AG_DIR)/payload.cc \
              $(AG_DIR)/propeller.cc \
              $(AG_DIR)/inst_workers/trap_worker_impl.cc \
              $(AG_DIR)/inst_workers/callinsn_worker_impl.cc \
              $(AG_DIR)/inst_workers/callblk_worker_impl.cc \
              $(AG_DIR)/inst_workers/spring_worker_impl.cc \
              $(AG_DIR)/ipc/ipc_mgr.cc \
              $(AG_DIR)/ipc/ipc_workers/ipc_worker_delegate.cc \
              $(AG_DIR)/ipc/ipc_workers/pipe_worker_impl.cc \
              $(AG_DIR)/ipc/ipc_workers/tcp_worker_impl.cc \
              $(AG_DIR)/ipc/ipc_workers/udp_worker_impl.cc \
              $(AG_DIR)/patchapi/addr_space.cc \
              $(AG_DIR)/patchapi/cfg.cc \
              $(AG_DIR)/patchapi/instrumenter.cc \
              $(AG_DIR)/patchapi/maker.cc \
              $(AG_DIR)/patchapi/object.cc \
              $(AG_DIR)/patchapi/point.cc \
              $(AG_DIR)/snippet.cc \

AG_OBJS  = $(addprefix $(OBJS_DIR)/, $(notdir $(AG_SRCS:%.cc=%.o)))

AG_DEP  = .AG_DEPENDS
$(shell if test -f $(AG_DEP); then true; else \
$(GXX) -MM $(AG_IFLAGS) $(AG_FLAGS) $(AG_SRCS) > /tmp/t;\
perl -ane 'print "\$(OBJS_DIR)/" if/\w+.o: /; print;' < /tmp/t > $(AG_DEP);\
fi);

include $(AG_DEP)

agent_lib: $(AGENT)

$(AG_OBJS): $(OBJS_DIR)/%.o : %.cc
	@echo Compiling $*.o
	@$(MKDIR) $(OBJS_DIR)
	@$(GXX) -c -o $@ $(AG_IFLAGS) $(AG_FLAGS) $<

$(AGENT): $(AG_OBJS)  $(IJ_OBJS) $(UTILS_OBJS)
ifeq ($(DYNLINK), true)
	@echo Dynamic Linking $@
	@$(GXX) -o $@  $(AG_OBJS) $(IJ_OBJS) $(UTILS_OBJS) $(AG_LDFLAGS) $(IJ_LDFLAGS) -shared
else
	@echo Static Linking $@
	@ar rs $@  $(AG_OBJS) $(IJ_OBJS) $(UTILS_OBJS)
endif

#======================================
# Unit Tests
#======================================
include $(SP_DIR)/test.target.mk

#=================================================
# build dyninst
# XXX: Not used so far ...
#=================================================
DYN_OBJS    += $(wildcard $(DYNINST_DIR)/common/$(PLATFORM)/*.o)
DYN_OBJS    += $(wildcard $(DYNINST_DIR)/instructionAPI/$(PLATFORM)/*.o)
DYN_OBJS    += $(wildcard $(DYNINST_DIR)/parseAPI/$(PLATFORM)/*.o)
DYN_OBJS    += $(wildcard $(DYNINST_DIR)/patchAPI/$(PLATFORM)/*.o)
DYN_OBJS    += $(wildcard $(DYNINST_DIR)/proccontrol/$(PLATFORM)/*.o)
DYN_OBJS    += $(wildcard $(DYNINST_DIR)/stackwalk/$(PLATFORM)/*.o)
DYN_OBJS    += $(wildcard $(DYNINST_DIR)/symtabAPI/$(PLATFORM)/*.o)

build_dyninst: $(DYN_OBJS)
	ar rs dyninst.a $(DYN_OBJS)

#=================================================
# depend
#=================================================
depend:
	@echo "Building dependency ..."
	@$(GXX) -MM $(AG_IFLAGS) $(AG_FLAGS) $(AG_SRCS) > /tmp/t
	@perl -ane 'print "\$(OBJS_DIR)/" if/\w+.o: /; print;' < /tmp/t > $(AG_DEP)
	@$(GXX) -MM $(IJ_IFLAGS) $(IJ_FLAGS) $(IJMAIN_SRCS) > /tmp/t
	@perl -ane 'print "\$(OBJS_DIR)/" if/\w+.o: /; print;' < /tmp/t > $(IJMAIN_DEP)
	@$(GXX) -MM $(IJ_IFLAGS) $(IJ_FLAGS) $(IJ_SRCS) > /tmp/t
	@perl -ane 'print "\$(OBJS_DIR)/" if/\w+.o: /; print;' < /tmp/t > $(IJ_DEP)
	@$(GXX) -MM $(TG_FLAGS) $(TG_SRCS) > /tmp/t
	@perl -ane 'print "\$(TAGENT_OBJS_DIR)/" if/\w+.o: /; print;' < /tmp/t > $(TAGENT_DEP)


#=================================================
# Others
#=================================================
clean_objs:
	$(RM) -r $(OBJS_DIR)

clean:
	@echo "Cleaning all except for dependencies ..."
	@$(RM) -r $(INJECTOR) $(AGENT) $(IJAGENT) *.o *.so *.a $(OBJS_DIR)

clean_all:
	@echo "Cleaning all ..."
	@$(RM) -r $(INJECTOR) $(AGENT) $(IJAGENT) *.o *.so *.a $(OBJS_DIR)
	@$(RM) $(IJ_DEP) $(IJMAIN_DEP) $(AG_DEP) $(TAGENT_DEP)
	@$(RM) -r $(TEST_EXES_DIR) $(UT_ONESTOP_EXE)
	@$(RM) -r $(MUTATEE_EXES_DIR)
	@$(RM) -r $(TAGENT_EXES_DIR)

clean_test:
	@echo "Cleaning tests ..."
	@$(RM) -r $(TEST_EXES_DIR) $(UT_ONESTOP_EXE)
	@$(RM) -r $(MUTATEE_EXES_DIR)
	@$(RM) -r $(TAGENT_EXES_DIR)

msg: agent_lib injector_exe
	@echo ""
	@echo ""
	@echo " Successfully built ..."
	@echo " Please set the environment variable SP_DIR to -- "
	@echo " $(SP_DIR) "
	@echo ""

debug:
