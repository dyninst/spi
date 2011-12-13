source ../../../make.config
export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$LD_LIBRARY_PATH

#./test -pipe1
#./test -pipe2
LD_PRELOAD=./TestAgent.so ./pipe3
#LD_PRELOAD=./TestAgent.so /scratch/wenbin/binaries/bin/bin/bash
