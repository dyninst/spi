export LD_LIBRARY_PATH=`pwd`/../../../$PLATFORM:$LD_LIBRARY_PATH
#../../../$PLATFORM/Injector $1 `pwd`/lib*.so
#../../../$PLATFORM/Injector $1 `pwd`/../../../$PLATFORM/libagent.so
ldd lib*
