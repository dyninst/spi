./Mutatee & 
PID=$!
echo "runing mutatee $PID"

sleep 1
echo "running injector on $PID"
Injector $PID libagent.so

sleep 1
echo "check maps at /proc/$PID"
cat /proc/$PID/maps

echo "mutatee $PID gets killed"
kill $PID
