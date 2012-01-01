source ../../../make.config
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH

make
#./test -pipe1
./test -pipe2
#./test -pipe3
#./test -pipe4
#./test -pipe5
#LD_PRELOAD=./TestAgent.so /scratch/wenbin/binaries/bin/bin/bash
