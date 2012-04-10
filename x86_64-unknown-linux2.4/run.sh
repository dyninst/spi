PATH=/scratch/chromium/gold/bin:/scratch/chromium/bin:/scratch/chromium/depot_tools:/afs/cs.wisc.edu/s/gcc-4.6.1/@sys/bin:/afs/cs.wisc.edu/s/gperf-3.0.2/@sys/bin:/afs/cs.wisc.edu/unsup/git-1.7.6/@sys/bin:/afs/cs.wisc.edu/s/python-2.7.1/@sys/bin:/u/w/e/wenbin/bin:/s/std/bin:/usr/afsws/bin:/opt/SUNWspro/bin:/usr/ccs/bin:/usr/ucb:/bin:/usr/bin:/usr/stat/bin:/usr/X11R6/bin

LD_LIBRARY_PATH=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/x86_64-unknown-linux2.4:/scratch/wenbin/software/lib:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/dyninst/dep/lib:/afs/cs.wisc.edu/s/gcc-4.6.1/amd64_rhel5/lib64:/scratch/chromium/lib64:/scratch/chromium/lib:./:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/dyninst/x86_64-unknown-linux2.4/:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/dyninst/x86_64-unknown-linux2.4/lib:/p/paradyn/packages/tcl8.4.5/lib:/p/paradyn/packages/libdwarf/lib:/p/paradyn/packages/libelf/bin:/p/paradyn/packages/libxml2/lib:test_mutatee

export PATH LD_LIBRARY_PATH


SPLIB=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/x86_64-unknown-linux2.4/test_agent/chrome_test_agent.so

#export SP_DEBUG=1
#export SP_COREDUMP=1
export SP_TEST_RELOCINSN=1
export SP_TEST_RELOCBLK=1
#export SP_TEST_SPRING=1
#export SP_TEST_TRAP=1
#export SP_DIRECTCALL_ONLY=1
#export SP_SKIP=1
#export SP_LIBC_MALLOC=1

LD_PRELOAD=$SPLIB /scratch/chromium/src/out/Debug/chrome  >& /tmp/log
#LD_PRELOAD=$SPLIB /scratch/chromium/src/out/Debug/chrome
#rm -rf /scratch/chromium/src/out/Debug/core*
#cp core* /scratch/chromium/src/out/Debug
#gdb /scratch/chromium/src/out/Release/chrome "$@"


#export SP_TEST_RELOCBLK=1
#export SP_TEST_SPRING=1


#export LD_PRELOAD=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/x86_64-unknown-linux2.4/test_agent/print_test_agent.so
#export LD_PRELOAD=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/x86_64-unknown-linux2.4/test_agent/gcc_test_agent.so
#/scratch/wenbin/software/libexec/gcc/x86_64-unknown-linux-gnu/4.5.2/cc1 /afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/user_agent/count/x86_64-unknown-linux2.4/200.i >& /tmp/log

#LD_PRELOAD=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/x86_64-unknown-linux2.4/test_agent/multithread_test_agent.so test_mutatee/multithread.exe
