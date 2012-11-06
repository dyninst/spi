LD_LIBRARY_PATH=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/$PLATFORM:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/$PLATFORM/test_mutatee:$LD_LIBRARY_PATH
AG=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/$PLATFORM/test_agent/ipc_test_agent.so
#AG=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/$PLATFORM/test_agent/print_test_agent.so
#AG=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/user_agent/sc2012_demo/$PLATFORM/libmyagent.so


rm -f tmp.txt
rm -f /tmp/*.xml

#export SP_DEBUG=1
#export SP_COREDUMP=1
LD_PRELOAD=$AG ../bin/wget localhost:8000/tmp.txt
#../bin/wget localhost:8000/tmp.txt
#LD_PRELOAD=$AG ../../test_mutatee/tcp_client.exe localhost

CLIENT_PID=$!
