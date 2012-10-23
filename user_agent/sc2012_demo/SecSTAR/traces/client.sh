source ../../../../config.mk
source config

export AGENT_NAME=libmyagent.so
export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$SP_DIR/$PLATFORM/test_mutatee:$LD_LIBRARY_PATH
export AGENT=`pwd`/../../$PLATFORM/$AGENT_NAME
export SP_AGENT_DIR=`pwd`/../../$PLATFORM

cp -f ../../x86_64-unknown-linux2.4/getpidfromport /tmp/getpidfromport

LD_PRELOAD="$AGENT" $SP_DIR/$PLATFORM/test_mutatee/tcp_client.exe $SERVER
