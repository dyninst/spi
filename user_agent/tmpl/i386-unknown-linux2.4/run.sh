source ../../../config.mk
AGENT_NAME=libmyagent.so
export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$LD_LIBRARY_PATH

AGENT=`pwd`/$AGENT_NAME
if test $# -lt 1; then
    echo "Usage: sh run.sh TARGET_PROG_PATH"
    exit
fi

LD_PRELOAD="$AGENT" $1
