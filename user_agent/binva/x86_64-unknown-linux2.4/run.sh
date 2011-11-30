source ../../../make.config
AGENT_NAME=libmyagent.so
export LD_LIBRARY_PATH

AGENT=`pwd`/$AGENT_NAME
cd ../target/$PLATFORM

#-----------------------------------------------------------
# Edit this to add your user program in ../target/$PLATFORM 
#-----------------------------------------------------------                      

#LD_PRELOAD="$AGENT" ./1_Format_string_problem `perl -e 'print "A"x5000'`
#LD_PRELOAD="$AGENT" ./2_Using_freed_memory
#LD_PRELOAD="$AGENT" ./3_Stack_overflow `perl -e 'print "a"x257'`
#LD_PRELOAD="$AGENT" ./4_ahscpy-bad1
#LD_PRELOAD="$AGENT" ./5_mem-bad2 44
LD_PRELOAD="$AGENT" ./array

#valgrind --tool=memcheck ./3_Stack_overflow  `perl -e 'print "a"x257'`
#valgrind --tool=memcheck ./5_mem-bad2 44
#valgrind --tool=memcheck ./7_Heap_overflow 
#valgrind --leak-check=full --tool=memcheck ./CWE476_NULL_Pointer_Dereference__char_01
