source ../../../make.config
export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$LD_LIBRARY_PATH
AGENT=$1
MUTATEE=$2

$MUTATEE &
../../../$PLATFORM/Injector $! ./$AGENT
wait $!

#./test $1 $2
#./parser_mutatee &
#echo $!
#../../../$PLATFORM/Injector $! ./parser_agent.so
