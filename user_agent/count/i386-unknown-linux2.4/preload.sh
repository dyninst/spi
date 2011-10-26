SP=/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi
BIN=/scratch/wenbin/binaries/bin/bin

source ../make.config
export LD_LIBRARY_PATH=$SP_DIR/$PLATFORM:$LD_LIBRARY_PATH
#export SP_DEBUG=0

#START=`date`
#echo 1; LD_PRELOAD="./$AGENT_NAME" $BIN/[
#echo 2; LD_PRELOAD="./$AGENT_NAME" $BIN/base64 /tmp/tmp
#echo 3; LD_PRELOAD="./$AGENT_NAME" $BIN/basename /tmp/tmp
#echo 4; LD_PRELOAD="./$AGENT_NAME" $BIN/cat /tmp/tmp
#echo 5; LD_PRELOAD="./$AGENT_NAME" $BIN/chmod 755 /tmp/tmp
#echo 6; LD_PRELOAD="./$AGENT_NAME" $BIN/head /tmp/tmp
#echo 7; LD_PRELOAD="./$AGENT_NAME" $BIN/mv /tmp/tmp /tmp/tmp1
#echo 8; LD_PRELOAD="./$AGENT_NAME" $BIN/cp -f /tmp/tmp1 /tmp/tmp
#echo 9; LD_PRELOAD="./$AGENT_NAME" $BIN/rm /tmp/tmp1
#echo 10; LD_PRELOAD="./$AGENT_NAME" $BIN/uname -a
#echo 11; LD_PRELOAD="./$AGENT_NAME" $BIN/df
#echo 12; LD_PRELOAD="./$AGENT_NAME" $BIN/hostid
#echo 13; LD_PRELOAD="./$AGENT_NAME" $BIN/nice
#echo 14; LD_PRELOAD="./$AGENT_NAME" $BIN/rmdir /tmp/a
#echo 15; LD_PRELOAD="./$AGENT_NAME" $BIN/stty
#echo 16; LD_PRELOAD="./$AGENT_NAME" $BIN/unexpand /tmp/tmp
#echo 17; LD_PRELOAD="./$AGENT_NAME" $BIN/dir
#echo 18; LD_PRELOAD="./$AGENT_NAME" $BIN/id
#echo 19; LD_PRELOAD="./$AGENT_NAME" $BIN/nl /tmp/tmp
#echo 20; LD_PRELOAD="./$AGENT_NAME" $BIN/sum /tmp/tmp
#echo 21; LD_PRELOAD="./$AGENT_NAME" $BIN/uniq /tmp/tmp
#echo 22; LD_PRELOAD="./$AGENT_NAME" $BIN/dircolors
#echo 23; LD_PRELOAD="./$AGENT_NAME" $BIN/install /tmp/tmp /tmp/tmp1
#echo 24; LD_PRELOAD="./$AGENT_NAME" $BIN/seq 1 2 3
#echo 25; LD_PRELOAD="./$AGENT_NAME" $BIN/unlink /tmp/tmp1
#echo 26; LD_PRELOAD="./$AGENT_NAME" $BIN/dirname /tmp/tmp1
#echo 27; LD_PRELOAD="./$AGENT_NAME" $BIN/tac /tmp/tmp
#echo 28; LD_PRELOAD="./$AGENT_NAME" $BIN/uptime
#echo 29; LD_PRELOAD="./$AGENT_NAME" $BIN/sha1sum /tmp/tmp
#echo 30; LD_PRELOAD="./$AGENT_NAME" $BIN/du
#echo 31; LD_PRELOAD="./$AGENT_NAME" $BIN/tail /tmp/tmp
#echo 32; LD_PRELOAD="./$AGENT_NAME" $BIN/users
#echo 33; LD_PRELOAD="./$AGENT_NAME" $BIN/sha224sum /tmp/tmp
#echo 34; LD_PRELOAD="./$AGENT_NAME" $BIN/echo /tmp/tmp
#echo 35; LD_PRELOAD="./$AGENT_NAME" $BIN/link /tmp/tmp /tmp/tmp3
#echo 36; LD_PRELOAD="./$AGENT_NAME" $BIN/sha256sum /tmp/tmp
#echo 37; LD_PRELOAD="./$AGENT_NAME" $BIN/tee
#echo 38; LD_PRELOAD="./$AGENT_NAME" $BIN/vdir
#echo 39; LD_PRELOAD="./$AGENT_NAME" $BIN/wc -c /tmp/tmp
#echo 40; LD_PRELOAD="./$AGENT_NAME" $BIN/chown wenbin /tmp/tmp
#echo 41; LD_PRELOAD="./$AGENT_NAME" $BIN/env
#echo 42; LD_PRELOAD="./$AGENT_NAME" $BIN/ln /tmp/tmp /tmp/tmp5
#echo 43; LD_PRELOAD="./$AGENT_NAME" $BIN/ln /tmp/tmp /tmp/tmp5
#echo 44; LD_PRELOAD="./$AGENT_NAME" $BIN/pathchk /tmp/tmp
#echo 45; LD_PRELOAD="./$AGENT_NAME" $BIN/test
#echo 46; LD_PRELOAD="./$AGENT_NAME" $BIN/sha384sum /tmp/tmp
#echo 47; LD_PRELOAD="./$AGENT_NAME" $BIN/date
#echo 48; LD_PRELOAD="./$AGENT_NAME" $BIN/expand /tmp/tmp
#echo 49; LD_PRELOAD="./$AGENT_NAME" $BIN/logname
#echo 50; LD_PRELOAD="./$AGENT_NAME" $BIN/pinky
#echo 51; LD_PRELOAD="./$AGENT_NAME" $BIN/sha512sum /tmp/tmp
#echo 52; LD_PRELOAD="./$AGENT_NAME" $BIN/who
#echo 53; LD_PRELOAD="./$AGENT_NAME" $BIN/cksum /tmp/tmp
#echo 54; LD_PRELOAD="./$AGENT_NAME" $BIN/whoami
#echo 55; LD_PRELOAD="./$AGENT_NAME" $BIN/ls /
#echo 56; LD_PRELOAD="./$AGENT_NAME" $BIN/ls
#echo 57; LD_PRELOAD="./$AGENT_NAME" $BIN/true
#echo 58; LD_PRELOAD="./$AGENT_NAME" $BIN/false
#echo 59; LD_PRELOAD="./$AGENT_NAME" $BIN/sleep 2
#echo 60; LD_PRELOAD="./$AGENT_NAME" $BIN/sort /tmp/tmp
#echo 61; LD_PRELOAD="./$AGENT_NAME" $BIN/tsort /tmp/tmp
#echo 62; time LD_PRELOAD="./$AGENT_NAME" $BIN/pwd
#echo 63; LD_PRELOAD="./$AGENT_NAME" $BIN/md5sum /tmp/tmp
#echo 64; LD_PRELOAD="./$AGENT_NAME" $BIN/ptx /tmp/tmp
#echo 65; LD_PRELOAD="./$AGENT_NAME" $BIN/stat /tmp/tmp
#echo 66; LD_PRELOAD="./$AGENT_NAME" $BIN/yes
#echo 67; LD_PRELOAD="./$AGENT_NAME" $BIN/factor 10286
#echo 68; LD_PRELOAD="./$AGENT_NAME" $BIN/printenv
#echo 69; LD_PRELOAD="./$AGENT_NAME" $BIN/printenv
#echo 70; LD_PRELOAD="./$AGENT_NAME" $BIN/readlink /tmp/tmp
#echo 71; LD_PRELOAD="./$AGENT_NAME" $BIN/mktemp
#echo 72; LD_PRELOAD="./$AGENT_NAME" $BIN/groups
#echo 73; LD_PRELOAD="./$AGENT_NAME" $BIN/mkdir -p /tmp/tmp99/tmp3
#echo 74; LD_PRELOAD="./$AGENT_NAME" $BIN/printf "abasdfaf%s" "aaa" 
#echo 75; LD_PRELOAD="./$AGENT_NAME" $BIN/od /tmp/tmp
#$BIN/od /tmp/tmp
#./od /tmp/tmp
#LD_PRELOAD="./$AGENT_NAME" find /scratch -name "*.h"
#time LD_PRELOAD="./$AGENT_NAME" $BIN/cat /scratch/wenbin/binaries/coreutils-8.12/src/*.c
#$BIN/cat /scratch/wenbin/binaries/coreutils-8.12/src/*.c


#time LD_PRELOAD="./$AGENT_NAME" /afs/cs.wisc.edu/s/gcc-4.1.0/amd64_cent40/libexec/gcc/x86_64-unknown-linux-gnu/4.1.0/cc1plus  /afs/cs.wisc.edu/p/paradyn/development/wenbin/dyninst/dyninst/dyninstAPI/src/util.C -I/afs/cs.wisc.edu/p/paradyn/development/wenbin/dyninst/include -I/afs/cs.wisc.edu/p/paradyn/development/wenbin/dyninst/dyninst/common/h -I/afs/cs.wisc.edu/p/paradyn/development/wenbin/dyninst/dyninst/dynutil/h -I/afs/cs.wisc.edu/p/paradyn/development/wenbin/dyninst/dyninst -Wall -g -Dos_linux -Darch_x86_64 -Darch_64bit -Dx86_64_unknown_linux2_4  -Dcap_ptrace -Dcap_stripped_binaries -Dcap_async_events -Dcap_threads -Dcap_dynamic_heap -Dcap_relocation -Dcap_dwarf -Dcap_32_64 -Dcap_liveness -Dcap_fixpoint_gen -Dcap_noaddr_gen -Dcap_mutatee_traps -Dcap_binary_rewriter -Dcap_registers -Dcap_instruction_api -Dcap_serialization -Dcap_instruction_replacement -Dcap_tramp_liveness -Dbug_syscall_changepc_rewind -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I/afs/cs.wisc.edu/p/paradyn/development/wenbin/dyninst/dyninst/patchAPI/h

time LD_PRELOAD="./$AGENT_NAME" /afs/cs.wisc.edu/s/gcc-4.6.0/i386_rhel5/libexec/gcc/i686-pc-linux-gnu/4.6.0/cc1 200.i

#time /afs/cs.wisc.edu/s/gcc-4.6.0/i386_rhel5/libexec/gcc/i686-pc-linux-gnu/4.6.0/cc1 200.i

#echo 75; time $BIN/sort /afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/src/Agent/*.C
#echo 75; time LD_PRELOAD="./$AGENT_NAME" $BIN/sort /afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/src/Agent/*.C

#echo 75; valgrind --tool=callgrind "time LD_PRELOAD="./$AGENT_NAME" $BIN/wc -w ~/devel/dyninst/dyninst/dyninstAPI/src/*.C"
#echo 75; time $BIN/wc -w ~/devel/dyninst/dyninst/dyninstAPI/src/*.C
#echo 75; time LD_PRELOAD="./$AGENT_NAME" $BIN/wc -w ~/devel/dyninst/dyninst/dyninstAPI/src/*.C

#valgrind --tool=callgrind ./uptime
#valgrind --tool=callgrind ./sort /afs/cs.wisc.edu/p/paradyn/development/wenbin/dyninst/dyninst/dyninstAPI/src/*.C /afs/cs.wisc.edu/p/paradyn/development/wenbin/dyninst/dyninst/dyninstAPI/h/*.h
#valgrind --tool=callgrind $BIN/sort /afs/cs.wisc.edu/p/paradyn/development/wenbin/dyninst/dyninst/dyninstAPI/src/*.C
#time sort /afs/cs.wisc.edu/p/paradyn/development/wenbin/dyninst/dyninst/dyninstAPI/src/*.C /afs/cs.wisc.edu/p/paradyn/development/wenbin/dyninst/dyninst/dyninstAPI/h/*.h


#END=`date`
#echo "START $START"
#echo "END $END"
