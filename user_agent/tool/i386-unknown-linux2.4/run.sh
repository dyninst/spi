source ../../../make.config
AGENT_NAME=libmyagent.so
export LD_LIBRARY_PATH

AGENT=`pwd`/$AGENT_NAME
if test $# -lt 1; then
    echo "Usage: sh run.sh TARGET_PROG_PATH"
    exit
fi

#-----------------------------------------------------------
# Edit this to add your user program in ../target/$PLATFORM
#-----------------------------------------------------------                     

LD_PRELOAD="$AGENT" $1
