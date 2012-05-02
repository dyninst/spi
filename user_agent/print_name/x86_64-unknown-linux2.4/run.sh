source ../../../config.mk
AGENT_NAME=libmyagent.so
export LD_LIBRARY_PATH
AGENT=`pwd`/$AGENT_NAME
BIN=./ 
#---------------------
# run program
#---------------------

# For injector
#../../../$PLATFORM/Injector $PID `pwd`/lib*.so

# For unix utility
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
#echo 55; LD_PRELOAD="./$AGENT_NAME" $BIN/ls
#echo 56; LD_PRELOAD="./$AGENT_NAME" $BIN/ls $1
#echo 57; LD_PRELOAD="./$AGENT_NAME" $BIN/true
#echo 58; LD_PRELOAD="./$AGENT_NAME" $BIN/false
#echo 59; LD_PRELOAD="./$AGENT_NAME" $BIN/sleep 2
#echo 60; LD_PRELOAD="./$AGENT_NAME" $BIN/sort /tmp/tmp
#echo 61; LD_PRELOAD="./$AGENT_NAME" $BIN/tsort /tmp/tmp
#echo 62; LD_PRELOAD="./$AGENT_NAME" $BIN/pwd
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

# For gcc
#time LD_PRELOAD="$AGENT" /scratch/wenbin/software/libexec/gcc/x86_64-unknown-linux-gnu/4.5.2/cc1 200.i

#PATH=/scratch/chromium/gold/bin:/scratch/chromium/bin:/scratch/chromium/depot_tools:/afs/cs.wisc.edu/s/gcc-4.6.1/@sys/bin:/afs/cs.wisc.edu/s/gperf-3.0.2/@sys/bin:/afs/cs.wisc.edu/unsup/git-1.7.6/@sys/bin:/afs/cs.wisc.edu/s/python-2.7.1/@sys/bin:/u/w/e/wenbin/bin:/s/std/bin:/usr/afsws/bin:/opt/SUNWspro/bin:/usr/ccs/bin:/usr/ucb:/bin:/usr/bin:/usr/stat/bin:/usr/X11R6/bin:$PATH
#LD_LIBRARY_PATH=/scratch/chromium/lib64:/scratch/chromium/lib:./:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/dyninst/x86_64-unknown-linux2.4/:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/dyninst/x86_64-unknown-linux2.4/lib:/p/paradyn/packages/tcl8.4.5/lib:/p/paradyn/packages/libdwarf/lib:/p/paradyn/packages/libelf/bin:/p/paradyn/packages/libxml2/lib:$LD_LIBRARY_PATH
#export PATH LD_LIBRARY_PATH



#LD_PRELOAD="./$AGENT_NAME" /scratch/chromium/src/out/Release/chrome "$@"
#gdb /scratch/chromium/src/out/Release/chrome ./core*
make clean; make
LD_PRELOAD="$AGENT_NAME" /afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/test/x86_64-unknown-linux2.4/mutatee/bin/curl localhost:3900/index.html