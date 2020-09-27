export SP_COREDUMP=1
export SP_FDEBUG=1
export SP_DIR=/p/paradyn/development/tylerg/spi
export SP_AGENT_DIR=/p/paradyn/development/tylerg/spi/x86_64-unknown-linux2.4/test_agent
export LD_LIBRARY_PATH=$SP_DIR/x86_64-unknown-linux2.4:$DYNINST_ROOT/lib:$LD_LIBRARY_PATH
export PLATFORM=x86_64-unknown-linux2.4
#export SP_TEST_SPRING=0
#export SP_TEST_TRAP=1

rm -f tmp/spi-*

#_condor_MASTER_TIMEOUT_MULTIPLIER=100 _condor_TOOL_TIMEOUT_MULTIPLIER=100 _condor_SUBMIT_TIMEOUT_MULTIPLIER=100 LD_PRELOAD=/home/wenbin/devel/spi/x86_64-unknown-linux2.4/test_agent/condor_test_agent.so condor_master

#service condor stop
#_condor_MASTER_TIMEOUT_MULTIPLIER=100 _condor_TOOL_TIMEOUT_MULTIPLIER=100 _condor_SUBMIT_TIMEOUT_MULTIPLIER=100 service condor start

mkdir -p tmp/spi
LD_PRELOAD=/p/paradyn/development/tylerg/spi/x86_64-unknown-linux2.4/test_agent/print_test_agent.so ./test_mutatee/pipe1.exe
