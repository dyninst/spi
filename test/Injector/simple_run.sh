
export LD_LIBRARY_PATH=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/$PLATFORM:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/test/Agent/$PLATFORM:$LD_LIBRARY_PATH

$PLATFORM/Mutatee & 
PID=$!
echo "runing mutatee $PID"

sleep 1
#cat /proc/$PID/maps
echo "running injector on $PID"
../../$PLATFORM/Injector $PID `pwd`/$PLATFORM/libagent.so
#../../$PLATFORM/Injector $PID `pwd`/../Agent/$PLATFORM/parser_agent.so
#../../$PLATFORM/Injector $PID `pwd`/../Agent/$PLATFORM/event_agent.so
#../../$PLATFORM/Injector $PID `pwd`/../../$PLATFORM/libagent.so
#../../$PLATFORM/Injector $PID /afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/user_agent/tmpl/i386-unknown-linux2.4/libagent_tmpl.so
#cat /proc/$PID/maps

echo "mutatee $PID gets killed"
kill $PID
