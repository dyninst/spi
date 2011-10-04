export LD_LIBRARY_PATH=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/$PLATFORM:$LD_LIBRARY_PATH

ssh -t wenbin@feta.cs.wisc.edu vim /tmp/a &
PID=$! 

../../../$PLATFORM/Injector $PID `pwd`/lib*.so

kill $PID
