source ../../../make.config
AGENT_NAME=libmyagent.so
export LD_LIBRARY_PATH
AGENT=`pwd`/$AGENT_NAME

#---------------------
# run program
#---------------------
time LD_PRELOAD="$AGENT" /scratch/wenbin/software/libexec/gcc/x86_64-unknown-linux-gnu/4.5.2/cc1 200.i

