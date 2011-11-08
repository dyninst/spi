TRG_DIR      = ./
CC           = gcc 
TRG_SRCS    += $(wildcard ../*.c)
TRG_OBJS    += $(notdir $(TRG_SRCS:%.c=$(TRG_DIR)/%.o))
TRG_EXE     += $(notdir $(TRG_SRCS:%.c=$(TRG_DIR)/%))

all: $(TRG_EXE)

$(TRG_OBJS): %.o : $(TRG_SRCS)
	@echo "Compiling $@"
	@$(CC) -o $(TRG_DIR)/$@ ../$*.c -c

$(TRG_EXE): % : $(TRG_OBJS)
	@echo "Linking $@"
	@$(CC) -o $(TRG_DIR)/$@ $*.o

clean:
	rm -rf $(TRG_OBJS) $(TRG_EXE)

d:
	echo $(TRG_SRCS)
	echo $(TRG_OBJS)
