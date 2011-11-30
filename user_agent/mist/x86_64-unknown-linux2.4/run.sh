source ../../../make.config
AGENT_NAME=libmyagent.so

AGENT=`pwd`/$AGENT_NAME
cd ../target/$PLATFORM

#-----------------------------------------------------------                     # Edit this to add your user program in ../target/$PLATFORM                      #-----------------------------------------------------------                      
LD_PRELOAD="$AGENT" ./demo
#LD_PRELOAD="$AGENT" ./test1                                                     #LD_PRELOAD="$AGENT" ./test2                                                     #LD_PRELOAD="$AGENT" ./test3                                                     #LD_PRELOAD="$AGENT" ./test4 
