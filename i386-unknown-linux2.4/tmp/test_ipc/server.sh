LD_LIBRARY_PATH=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/$PLATFORM:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/$PLATFORM/test_mutatee:$LD_LIBRARY_PATH
AG=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/$PLATFORM/test_agent/print_test_agent.so


#LD_PRELOAD=$AG ../sbin/lighttpd -f conf -D
../sbin/lighttpd -f conf -D
#../../test_mutatee/tcp_server4.exe

SERVER_PID=$! 
echo $SERVER_PID