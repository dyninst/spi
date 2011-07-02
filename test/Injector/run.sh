./Mutatee & 
PID=$!
echo "runing mutatee $PID"

#cat /proc/$PID/maps

echo "running injector on $PID"
../../i386-unknown-linux2.4/Injector $PID libagent.so

#echo "check maps at /proc/$PID"
#cat /proc/$PID/maps

echo "mutatee $PID gets killed"
kill $PID
