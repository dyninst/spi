$PLATFORM/Mutatee & 
PID=$!
echo "runing mutatee $PID"

sleep 1
cat /proc/$PID/maps
echo "running injector on $PID"
../../$PLATFORM/Injector $PID `pwd`/$PLATFORM/libagent.so
cat /proc/$PID/maps

echo "mutatee $PID gets killed"
kill $PID
