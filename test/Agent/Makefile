MUTATEE  = $(PLATFORM)/Mutatee
MYAGENT    = $(PLATFORM)/libmyagent.so

all: $(MUTATEE) $(MYAGENT)

MY_LFLAG += -lagent -L../../$(PLATFORM)

$(MUTATEE): Mutatee.c $(MYAGENT)
	mkdir -p $(PLATFORM)
	gcc -o $@ $<  -lmyagent -L$(PLATFORM) $(MY_LFLAG)

MY_IFLAG += -I../../h
MY_IFLAG += -I../../../dyninst/include
MY_IFLAG += -I../../../dyninst/dyninst/patchAPI/h
MY_IFLAG += -I../../../dyninst/dyninst/common/h
MY_IFLAG += -I../../../dyninst/dyninst

MY_FLAG  += -Dos_linux -fPIC -O0

$(MYAGENT): MyAgent.C
	g++ -shared -o $@ $< $(MY_FLAG) $(MY_LFLAG) $(MY_IFLAG)

clean:
	rm -f $(MUTATEE) $(MYAGENT) $(PLATFORM)/*.so core.*
