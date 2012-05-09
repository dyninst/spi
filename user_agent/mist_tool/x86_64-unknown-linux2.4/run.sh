source ../../../config.mk
AGENT_NAME=libmyagent.so
LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$SP_DIR/$PLATFORM/test_mutatee:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH

AGENT=`pwd`/$AGENT_NAME

#-----------------------------------------------------------                     
# Edit this to add your user program in ../target/$PLATFORM
#-----------------------------------------------------------                     

#LD_PRELOAD="$AGENT" /mnt/hgfs/shared/spi/x86_64-unknown-linux2.4/test_mutatee/tcp_server6.exe

#export SP_TEST_RELOCINSN=1
#export SP_TEST_RELOCBLK=1
#export SP_TEST_SPRING=1
#export SP_TEST_TRAP=1

export SP_AGENT_DIR=`pwd`

rm -rf *.xml
#$SP_DIR/$PLATFORM/test_mutatee/tcp_server6.exe &
$SP_DIR/$PLATFORM/test_mutatee/tcp_server4.exe &
SERVER_PID=$!
LD_PRELOAD="$AGENT" $SP_DIR/$PLATFORM/test_mutatee/tcp_client.exe localhost
kill -9 $SERVER_PID
