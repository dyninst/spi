export SP_DEBUG=1

#export SP_TEST_RELOCINSN=1
#export SP_TEST_RELOCBLK=1
#export SP_TEST_SPRING=1
#export SP_TEST_TRAP=1

LD_PRELOAD=test_agent/condor_test_agent.so /unsup/condor/bin/condor_submit condor.task
#LD_PRELOAD=test_agent/count_test_agent.so test_mutatee/cc1 /u/w/e/wenbin/devel/spi/spi/user_agent/count/x86_64-unknown-linux2.4/200.i >& /tmp/log
#LD_PRELOAD=test_agent/print_test_agent.so test_mutatee/cc1 /u/w/e/wenbin/devel/spi/spi/user_agent/count/x86_64-unknown-linux2.4/200.i >& /tmp/log2
