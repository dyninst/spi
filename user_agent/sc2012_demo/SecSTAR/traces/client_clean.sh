source config

export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$SP_DIR/$PLATFORM/test_mutatee:$LD_LIBRARY_PATH
export SP_AGENT_DIR=`pwd`

cp -f ../../x86_64-unknown-linux2.4/getpidfromport /tmp/getpidfromport

../../$PLATFORM/tcp_client.exe $SERVER
