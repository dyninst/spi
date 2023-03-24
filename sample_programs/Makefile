
CFLAGS = -Wall
CC = gcc

CXXFLAGS = -Wall
CXX = g++


all: helloc hellocc calltest calltest_fork client server simple_fork

helloc: hello.c
	$(CC) -o helloc hello.c $(CFLAGS)

hellocc: hello.cc
	$(CXX) -o hellocc hello.cc $(CFLAGS)

calltest: calltest.cc
	$(CXX) -o calltest calltest.cc $(CFLAGS)

calltest_fork: calltest_fork.cc
	$(CXX) -o calltest_fork calltest_fork.cc $(CFLAGS)

client: tcp_client.c
	$(CC) -o tcp_client tcp_client.c $(CFLAGS)

server: tcp_server4.c
	$(CC) -o tcp_server4 tcp_server4.c $(CFLAGS)

simple_fork: simple_fork.cc
	$(CXX) -o simple_fork simple_fork.cc $(CFLAGS)

clean: 
	$(RM) \
        helloc hellocc calltest calltest_fork tcp_client tcp_server4 simple_fork \
	*.o
