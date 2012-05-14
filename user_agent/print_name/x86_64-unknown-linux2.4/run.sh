source ../../../config.mk
AGENT_NAME=libmyagent.so
export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$LD_LIBRARY_PATH
AGENT=`pwd`/$AGENT_NAME
BIN=./ 

#---------------------
# run program
#---------------------
LD_PRELOAD="$AGENT_NAME" ./ls