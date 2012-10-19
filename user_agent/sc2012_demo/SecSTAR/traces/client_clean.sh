export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$SP_DIR/$PLATFORM/test_mutatee:$LD_LIBRARY_PATH
export SP_AGENT_DIR=`pwd`


$SP_DIR/$PLATFORM/test_mutatee/tcp_client.exe grilled
