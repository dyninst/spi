LD_LIBRARY_PATH=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/x86_64-unknown-linux2.4:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/x86_64-unknown-linux2.4/test_mutatee:$LD_LIBRARY_PATH
#AG=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/x86_64-unknown-linux2.4/test_agent/print_test_agent.so
export SP_AGENT_DIR=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/user_agent/ipc/$PLATFORM
AG=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/x86_64-unknown-linux2.4/test_agent/condor_test_agent.so


#LD_PRELOAD=$AG ../sbin/lighttpd -f conf -D
../sbin/lighttpd -f conf -D
#../../test_mutatee/tcp_server4.exe

SERVER_PID=$! 
echo $SERVER_PID