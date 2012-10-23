export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$SP_DIR/$PLATFORM/test_mutatee:$LD_LIBRARY_PATH
export SP_AGENT_DIR=`pwd`/../../$PLATFORM

rm -f /tmp/*.xml
cp -f ../../x86_64-unknown-linux2.4/getpidfromport /tmp/getpidfromport

$SP_DIR/$PLATFORM/test_mutatee/tcp_server4.exe
