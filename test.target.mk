test: unittest mutatee external_mutatee test_agents

unittest: unittests

mutatee: mutatees external_mutatees

test_agent: test_agents

external_mutatee:

#==========================================================
# COMMON TEST MACROS
#==========================================================
TEST_OBJS_DIR = $(SP_DIR)/$(PLATFORM)/test/objs
TEST_EXES_DIR = $(SP_DIR)/$(PLATFORM)/test

MUTATEE_OBJS_DIR = $(SP_DIR)/$(PLATFORM)/test_mutatee/objs
MUTATEE_EXES_DIR = $(SP_DIR)/$(PLATFORM)/test_mutatee

TAGENT_OBJS_DIR = $(SP_DIR)/$(PLATFORM)/test_agent/objs
TAGENT_EXES_DIR = $(SP_DIR)/$(PLATFORM)/test_agent

TEST_DATA_DIR = $(SP_DIR)/$(PLATFORM)/test_data 

#==========================================================
# Google mock and Google test stuffs
#==========================================================
GTEST_DIR     = $(SP_DIR)/src/test/gmock/gtest
GMOCK_DIR     = $(SP_DIR)/src/test/gmock
CPPFLAGS     += -I$(GTEST_DIR)/include -I$(GMOCK_DIR)/include
CXXFLAGS     += -g -Wall -Wextra
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
GMOCK_HEADERS = $(GMOCK_DIR)/include/gmock/*.h \
                $(GMOCK_DIR)/include/gmock/internal/*.h \
                $(GTEST_HEADERS)
GTEST_SRCS_   = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
GMOCK_SRCS_   = $(GMOCK_DIR)/src/*.cc $(GMOCK_HEADERS)
GTEST_OBJ     = $(TEST_OBJS_DIR)/gtest_all.o
GMOCK_OBJ     = $(TEST_OBJS_DIR)/gmock_all.o
GMOCK_MAIN_OBJ= $(TEST_OBJS_DIR)/gmock_main.o
GMOCK_MAIN    = $(TEST_OBJS_DIR)/gmock_main.a

$(GTEST_OBJ) : $(GTEST_SRCS_)
	@echo "Building gtest"
	@$(MKDIR) $(TEST_OBJS_DIR)
	@$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GTEST_DIR)/src/gtest-all.cc -o $@

$(GMOCK_OBJ) : $(GMOCK_SRCS_)
	@echo "Building gmock"
	@$(MKDIR) $(TEST_OBJS_DIR)
	@$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GMOCK_DIR)/src/gmock-all.cc -o $@

$(GMOCK_MAIN_OBJ) : $(GMOCK_SRCS_)
	@$(MKDIR) $(TEST_OBJS_DIR)
	@$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GMOCK_DIR)/src/gmock_main.cc -o $@

$(GMOCK_MAIN) : $(GTEST_OBJ) $(GMOCK_OBJ) $(GMOCK_MAIN_OBJ)
	@$(MKDIR) $(TEST_OBJS_DIR)
	@$(AR) $(ARFLAGS) $@ $^


google-mock: $(GMOCK_MAIN)

#==========================================================
# SPI UNIT TESTS
#==========================================================
UT_FLAGS   = $(AG_IFLAGS) $(AG_FLAGS) $(CPPFLAGS)
UT_LDFLAGS = -lpthread

ifeq ($(DYNLINK), true)
UT_LDFLAGS += $(AG_LDFLAGS)
UT_LDFLAGS += $(IJ_LDFLAGS)
UT_LDFLAGS  += -L$(SP_DIR)/$(PLATFORM)
UT_LDFLAGS  += -lagent
else
UT_LDFLAGS  += $(AG_LDFLAGS)
UT_LDFLAGS  += libagent.a
endif

VPATH    += $(ST_DIR) \
            $(TC_DIR) \
            $(AG_DIR)/patchapi

UT_SRCS   = $(IJ_DIR)/injector_unittest.cc \
            $(AG_DIR)/patchapi/addr_space_unittest.cc \
            $(AG_DIR)/agent_unittest.cc \
            $(AG_DIR)/payload_unittest.cc \
            $(AG_DIR)/event_unittest.cc \
            $(AG_DIR)/context_unittest.cc \
            $(AG_DIR)/parser_unittest.cc \
            $(ST_DIR)/coreutils_systest.cc \
            $(ST_DIR)/chrome_systest.cc \
            $(ST_DIR)/cpp_systest.cc \
            $(ST_DIR)/gcc_systest.cc \
            $(ST_DIR)/multithread_systest.cc \
            $(ST_DIR)/ipc_systest.cc

UT_OBJS   = $(addprefix $(TEST_OBJS_DIR)/, $(notdir $(UT_SRCS:%.cc=%.o)))
UT_OBJS  += $(TEST_OBJS_DIR)/common_unittest.o
UT_EXES   = $(addprefix $(TEST_EXES_DIR)/, $(notdir $(UT_SRCS:%.cc=%.exe)))

UT_ONESTOP_EXE = $(SP_DIR)/$(PLATFORM)/all_tests

unittests: $(UT_EXES) $(UT_ONESTOP_EXE)

$(UT_OBJS): $(TEST_OBJS_DIR)/%.o : %.cc
	@echo Compiling $*.o
	@$(MKDIR) $(TEST_OBJS_DIR)
	@$(GXX) -c -o $@ $< $(UT_FLAGS)

$(UT_EXES): $(TEST_EXES_DIR)/%.exe : $(TEST_OBJS_DIR)/%.o $(GMOCK_MAIN)  $(AGENT) $(TEST_OBJS_DIR)/common_unittest.o
	@echo Linking $*.exe
	@$(GXX) -o $@ $< $(TEST_OBJS_DIR)/common_unittest.o $(GMOCK_MAIN) $(UT_LDFLAGS)

$(UT_ONESTOP_EXE): $(UT_OBJS) $(GMOCK_MAIN) $(AGENT)
	@echo Linking $(notdir $(UT_ONESTOP_EXE))
	@$(GXX) -o $@ $(UT_OBJS) $(GMOCK_MAIN) $(UT_LDFLAGS)

#==========================================================
# Mutatee
#==========================================================

# mutatee libs
MUTATEE_LIB_SRCS  = $(MT_DIR)/libtest1.c
MUTATEE_LIB_OBJS  = $(addprefix $(MUTATEE_OBJS_DIR)/, $(notdir $(MUTATEE_LIB_SRCS:%.c=%.o)))
MUTATEE_LIB_SO    = $(addprefix $(MUTATEE_EXES_DIR)/, $(notdir $(MUTATEE_LIB_SRCS:%.c=%.so)))

MUTATEE_LIB_FLAGS      += -g -fPIC
MUTATEE_LIB_LDFLAGS    += -shared -lpthread

mutatee_libs: $(MUTATEE_LIB_SO)

$(MUTATEE_LIB_OBJS): $(MUTATEE_OBJS_DIR)/%.o : $(ML_DIR)/%.c
	@echo "Compiling $*.o"
	@$(MKDIR) $(MUTATEE_OBJS_DIR)
	@gcc -o $@ $< $(MUTATEE_LIB_IFLAGS) $(MUTATEE_LIB_FLAGS) -c

$(MUTATEE_LIB_SO): $(MUTATEE_EXES_DIR)/%.so : $(MUTATEE_OBJS_DIR)/%.o
	@echo "Linking $*.so"
	@gcc -o $@ $< $(MUTATEE_LIB_LDFLAGS)

# mutatee exes (c language)
MUTATEE_SRCS  = $(ME_DIR)/tcp_client.c \
                $(ME_DIR)/tcp_server4.c \
                $(ME_DIR)/tcp_server6.c \
                $(ME_DIR)/pipe1.c \
                $(ME_DIR)/pipe2.c \
                $(ME_DIR)/indcall.c \
                $(ME_DIR)/libcall.c \
                $(ME_DIR)/multithread.c \
                $(ME_DIR)/stripped.c \

MUTATEE_CPP_SRCS  = $(ME_DIR)/cpp_prog.cc \

MUTATEE_OBJS      = $(addprefix $(MUTATEE_OBJS_DIR)/, $(notdir $(MUTATEE_SRCS:%.c=%.o)))
MUTATEE_EXES      = $(addprefix $(MUTATEE_EXES_DIR)/, $(notdir $(MUTATEE_SRCS:%.c=%.exe)))

MUTATEE_CPP_OBJS  = $(addprefix $(MUTATEE_OBJS_DIR)/, $(notdir $(MUTATEE_CPP_SRCS:%.cc=%.o)))
MUTATEE_CPP_EXES  = $(addprefix $(MUTATEE_EXES_DIR)/, $(notdir $(MUTATEE_CPP_SRCS:%.cc=%.exe)))

MUTATEE_LDFLAGS    = $(notdir $(MUTATEE_LIB_SO:$(MUTATEE_EXES_DIR)/lib%.so=-l%))
MUTATEE_FLAGS      += -g -fPIC
MUTATEE_LDFLAGS    += -lpthread -ldl
MUTATEE_LDFLAGS    += -L$(MUTATEE_EXES_DIR)

mutatee_exes: $(MUTATEE_EXES) $(MUTATEE_CPP_EXES)

$(MUTATEE_OBJS): $(MUTATEE_OBJS_DIR)/%.o : $(ME_DIR)/%.c
	@echo "Compiling $*.o"
	@$(MKDIR) $(MUTATEE_OBJS_DIR)
	@gcc -o $@ $< $(MUTATEE_IFLAGS) $(MUTATEE_FLAGS) -c

$(MUTATEE_EXES): $(MUTATEE_EXES_DIR)/%.exe : $(MUTATEE_OBJS_DIR)/%.o
	@echo "Linking $*.exe"
	@gcc -o $@ $< $(MUTATEE_LDFLAGS)

$(MUTATEE_CPP_OBJS): $(MUTATEE_OBJS_DIR)/%.o : $(ME_DIR)/%.cc
	@echo "Compiling $*.o"
	@$(MKDIR) $(MUTATEE_OBJS_DIR)
	@g++ -o $@ $< $(MUTATEE_IFLAGS) $(MUTATEE_FLAGS) -c


$(MUTATEE_CPP_EXES): $(MUTATEE_EXES_DIR)/%.exe : $(MUTATEE_OBJS_DIR)/%.o
	@echo "Linking $*.exe"
	@g++ -o $@ $< $(MUTATEE_LDFLAGS)

mutatees: mutatee_libs mutatee_exes

#==========================================================
# External mutatee
#==========================================================
external_mutatees:
	@bash ../scripts/build_mutatees.sh
	@cp ../test_data/$(PLATFORM)/test_exes/* test_mutatee/
	@cp ../test_data/$(PLATFORM)/test_libs/* tmp/lib/

#==========================================================
# Test agents
#==========================================================
TG_SRCS  = $(TG_DIR)/count_test_agent.cc \
           $(TG_DIR)/chrome_test_agent.cc \
           $(TG_DIR)/payload_test_agent.cc \
           $(TG_DIR)/ipc_test_agent.cc \
           $(TG_DIR)/gcc_test_agent.cc \
           $(TG_DIR)/inject_test_agent.cc \
           $(TG_DIR)/print_test_agent.cc \
           $(TG_DIR)/condor_test_agent.cc \
           $(TG_DIR)/multithread_test_agent.cc \

TG_OBJS  = $(addprefix $(TAGENT_OBJS_DIR)/, $(notdir $(TG_SRCS:%.cc=%.o)))
TG_SO    = $(addprefix $(TAGENT_EXES_DIR)/, $(notdir $(TG_SRCS:%.cc=%.so)))

TG_FLAGS   = $(AG_FLAGS) $(AG_IFLAGS)
TG_LDFLAGS = -shared

ifeq ($(DYNLINK), true)
TG_LDFLAGS  += -L$(SP_DIR)/$(PLATFORM)
TG_LDFLAGS  += -lagent
else
TG_LDFLAGS  += $(AG_LDFLAGS)
TG_LDFLAGS  += libagent.a
endif

TAGENT_DEP   = .TAGENT_DEPENDS

$(shell if test -f $(TAGENT_DEP); then true; else \
$(GXX) -MM $(TG_FLAGS) $(TG_SRCS) > /tmp/t; \
perl -ane 'print "\$(TAGENT_OBJS_DIR)/" if/\w+.o: /; print;' < /tmp/t > $(TAGENT_DEP); \
fi);

include $(TAGENT_DEP)

test_agents: $(TG_SO)

$(TG_OBJS): $(TAGENT_OBJS_DIR)/%.o : $(TG_DIR)/%.cc
	@echo "Compiling $*.o"
	@$(MKDIR) $(TAGENT_OBJS_DIR)
	@$(GCC) -o $@ $< $(TG_FLAGS) -c

$(TG_SO): $(TAGENT_EXES_DIR)/%.so : $(TAGENT_OBJS_DIR)/%.o  $(AGENT)
	@echo "Linking $*.so"
	@$(GCC) -o $@ $< $(TG_LDFLAGS)
