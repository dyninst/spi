SP=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi
BIN=/scratch/wenbin/binaries/bin/bin
TRG=../target/$PLATFORM

source ../make.config
export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$LD_LIBRARY_PATH

#export SP_DEBUG=0
CWD=`pwd`
cd $TRG
#LD_PRELOAD="$CWD/$AGENT_NAME" ./test1 1 2 3
#LD_PRELOAD="$CWD/$AGENT_NAME" ./test2 1 2 3
LD_PRELOAD="$CWD/$AGENT_NAME" ./test3
#LD_PRELOAD="./$AGENT_NAME" $BIN/ls $1
#LD_PRELOAD="./$AGENT_NAME" $BIN/printf "hello"
#echo 3; LD_PRELOAD="./$AGENT_NAME" $BIN/basename /tmp/tmp
