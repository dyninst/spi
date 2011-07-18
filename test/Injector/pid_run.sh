PID=$1
echo "runing mutatee $PID"

export LD_LIBRARY_PATH=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/$PLATFORM:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/test/Agent/$PLATFORM:$LD_LIBRARY_PATH

sleep 1
echo "running injector on $PID"
#../../$PLATFORM/Injector $PID `pwd`/$PLATFORM/libagent.so
#../../$PLATFORM/Injector $PID $2
#../../$PLATFORM/Injector $PID /afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/user_agent/tmpl/i386-unknown-linux2.4/libagent_tmpl.so
#../../$PLATFORM/Injector $PID `pwd`/../Agent/$PLATFORM/parser_agent.so
../../$PLATFORM/Injector $PID $2


#echo "mutatee $PID gets killed"
#kill $PID
