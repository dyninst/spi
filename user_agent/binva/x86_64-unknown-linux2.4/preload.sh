SP=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi
BIN=/scratch/wenbin/binaries/bin/bin
TRG=../target/$PLATFORM

source ../make.config
export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$LD_LIBRARY_PATH

#LD_PRELOAD="./$AGENT_NAME" $TRG/double_free
#LD_PRELOAD="./$AGENT_NAME" $TRG/dangerous_func
#LD_PRELOAD="./$AGENT_NAME" $TRG/printf
#LD_PRELOAD="./$AGENT_NAME" $TRG/mallocfree

export TEMP_FILE=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/user_agent/binva/target/files/fileaccess.txt
LD_PRELOAD="./$AGENT_NAME" $TRG/fileaccess
