MUTATEE  = $(PLATFORM)/Mutatee
AGENT    = $(PLATFORM)/libagent.so

all: $(MUTATEE) $(AGENT)

$(MUTATEE): Mutatee.c
	mkdir -p $(PLATFORM)
	gcc -o $@ $< 
$(AGENT): Agent.c
	gcc -shared -o $@ $< -fPIC -O0
clean:
	cd $(PLATFORM) ; rm * 
	rm -f $(MUTATEE) $(AGENT) core.*
