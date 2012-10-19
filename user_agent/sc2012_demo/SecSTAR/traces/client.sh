source ../../../../config.mk
export AGENT_NAME=libmyagent.so
export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$SP_DIR/$PLATFORM/test_mutatee:$LD_LIBRARY_PATH
export AGENT=`pwd`/../../$PLATFORM/$AGENT_NAME
export SP_AGENT_DIR=`pwd`/../../$PLATFORM

rm -rf /tmp/*.xml

LD_PRELOAD="$AGENT" $SP_DIR/$PLATFORM/test_mutatee/tcp_client.exe localhost
