source ../../../make.config
export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$LD_LIBRARY_PATH

LD_PRELOAD=./TestAgent.so ./pipe
