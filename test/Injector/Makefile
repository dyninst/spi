all: Mutatee libagent.so

Mutatee: Mutatee.c
	gcc -o Mutatee Mutatee.c 
libagent.so: Agent.c
	gcc -shared -o libagent.so Agent.c -fPIC -O0
clean:
	rm -f Mutatee core* libagent.so
