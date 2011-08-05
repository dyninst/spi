source ../../make.config
export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$SP_DIR/test/Agent/$PLATFORM:$LD_LIBRARY_PATH

$SP_DIR/test/Injector/$PLATFORM/Mutatee & 
PID=$!

sleep 1
$SP_DIR/$PLATFORM/Injector $PID $SP_DIR/test/Injector/$PLATFORM/libagent.so

kill $PID
