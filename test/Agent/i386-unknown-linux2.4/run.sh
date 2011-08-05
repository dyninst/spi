source ../../../make.config
export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$LD_LIBRARY_PATH
./test $1
