AGENT_SRCS    +=

include ../../../config.mk
include ../config.mk
include ../../../common.flag.mk
include	../../target.mk

# Auxilary things for the demo
aux: getpidfromport tcp_client.exe tcp_server4.exe

getpidfromport:	../getpidfromport.C
								g++ -o getpidfromport ../getpidfromport.C 

tcp_client.exe: ../tcp_client.c
								gcc -o tcp_client.exe ../tcp_client.c

tcp_server4.exe: ../tcp_client.c
								gcc -o tcp_server4.exe ../tcp_server4.c

clean_all:
	@make clean
	rm -f getpidfromport tcp_client.exe tcp_server4.exe
