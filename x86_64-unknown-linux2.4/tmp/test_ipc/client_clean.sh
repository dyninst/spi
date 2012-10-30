LD_LIBRARY_PATH=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/x86_64-unknown-linux2.4:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/x86_64-unknown-linux2.4/test_mutatee:$LD_LIBRARY_PATH
export SP_AGENT_DIR=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/user_agent/ipc/$PLATFORM

rm -f tmp.txt
rm -f /tmp/*.xml

#export SP_DEBUG=1
#export SP_COREDUMP=1
#export SP_TEST_RELOCINSN=1
#LD_PRELOAD=$AG ../bin/wget localhost:8000/tmp.txt
#LD_PRELOAD=$AG ../bin/wget grilled:8001/tmp.txt
#../bin/wget localhost:8000/tmp.txt
../bin/wget grilled:8001/tmp.txt
#LD_PRELOAD=$AG ../../test_mutatee/tcp_client.exe localhost
#../../test_mutatee/tcp_client.exe localhost

CLIENT_PID=$!
echo $CLIENT_PID