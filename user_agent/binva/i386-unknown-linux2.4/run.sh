source ../../../make.config
AGENT_NAME=libmyagent.so
export LD_LIBRARY_PATH

AGENT=`pwd`/$AGENT_NAME
cd ../target/$PLATFORM

#-----------------------------------------------------------
# Edit this to add your user program in ../target/$PLATFORM 
#-----------------------------------------------------------                      

LD_PRELOAD="$AGENT" ./fileaccess

