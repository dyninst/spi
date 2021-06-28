GXX        = g++
RM         = rm -f

all: $(AGENT_NAME)

AGENT_DIR      = ../
AGENT_OBJS    += $(notdir $(AGENT_SRCS:%.C=%.o))

$(AGENT_OBJS): %.o : $(AGENT_DIR)/%.C
	@echo Compiling $*.C
	$(GXX) -c -o $@ $(AG_IFLAGS) $(AG_FLAGS) $(AGENT_FLAGS) $(AGENT_DIR)/$*.C 

$(AGENT_NAME): $(AGENT_OBJS)
	@echo Linking $@
	$(GXX) -o $@  $(AGENT_OBJS) $(AG_LDFLAGS) $(AGENT_LDFLAGS) -L$(SP_DIR)/$(PLATFORM) -shared -lagent
	mv $(AGENT_NAME) $(SP_DIR)/$(PLATFORM)

#==============================================
# clean up
#==============================================
clean:
	@$(RM) *.o *.so $(AGENT_DEP)
