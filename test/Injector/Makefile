MUTATEE  = $(PLATFORM)/Mutatee
AGENT    = $(PLATFORM)/libagent.so

all: $(MUTATEE) $(AGENT)

$(MUTATEE): Mutatee.c
	gcc -o $@ $< 
$(AGENT): Agent.c
	gcc -shared -o $@ $< -fPIC -O0
clean:
	cd $(PLATFORM)
	rm -f $(MUTATEE) $(AGENT) core.*
