export SP_FDEBUG=1

rm -f /tmp/spi-*

#_condor_MASTER_TIMEOUT_MULTIPLIER=100 _condor_TOOL_TIMEOUT_MULTIPLIER=100 _condor_SUBMIT_TIMEOUT_MULTIPLIER=100 LD_PRELOAD=/home/wenbin/devel/spi/x86_64-unknown-linux2.4/test_agent/condor_test_agent.so condor_master

#service condor stop
_condor_MASTER_TIMEOUT_MULTIPLIER=100 _condor_TOOL_TIMEOUT_MULTIPLIER=100 _condor_SUBMIT_TIMEOUT_MULTIPLIER=100 service condor start