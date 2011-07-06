$1 &
PID=$!
echo "runing mutatee $PID"

sleep 1
echo "running injector on $PID"
../../$PLATFORM/Injector $PID `pwd`/$PLATFORM/libagent.so


echo "mutatee $PID gets killed"
kill $PID
