SP=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi
BIN=/scratch/wenbin/binaries/bin/bin
TRG=../target/$PLATFORM

source ../make.config
export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$LD_LIBRARY_PATH

#export SP_DEBUG=0
LD_PRELOAD="./$AGENT_NAME" $TRG/test1
