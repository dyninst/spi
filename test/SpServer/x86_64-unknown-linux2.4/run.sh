source ../../../make.config
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH

make
#LD_PRELOAD=./TestAgent.so /scratch/wenbin/binaries/bin/bin/bash

#./test -pipe1
#./test -pipe2
#./test -pipe3
#./test -pipe4
#./test -pipe5
./test -tcp1
