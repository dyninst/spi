source ../../../config.mk
AGENT_NAME=libmyagent.so
export LD_LIBRARY_PATH=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/x86_64-unknown-linux2.4:$LD_LIBRARY_PATH

AGENT=`pwd`/$AGENT_NAME


LD_PRELOAD="$AGENT" /u/w/e/wenbin/devel/spi/spi/x86_64-unknown-linux2.4/tmp/bin/ls
