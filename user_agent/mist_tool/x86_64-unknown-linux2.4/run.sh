source ../../../config.mk
AGENT_NAME=libmyagent.so
LD_LIBRARY_PATH=/mnt/hgfs/shared/spi/x86_64-unknown-linux2.4:/mnt/hgfs/shared/spi/x86_64-unknown-linux2.4/test_mutatee:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH

AGENT=`pwd`/$AGENT_NAME
#if test $# -lt 1; then
#    echo "Usage: sh run.sh TARGET_PROG_PATH"
#    exit
#fi

#-----------------------------------------------------------
# Edit this to add your user program in ../target/$PLATFORM
#-----------------------------------------------------------

LD_PRELOAD="$AGENT" /mnt/hgfs/shared/spi/x86_64-unknown-linux2.4/test_mutatee/tcp_server6.exe
