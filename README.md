# Self-Propelled Instrumentation

## Note
1. Currently SPI depends on a customized dyninstAPI
  - build dyninst with arch-x86.h exposed to public
  - build dyninst with OpenMP disabled
2. Dynamically linked functions may appear twice during instrumentation, because these functions exist in .plt sections too.

## TODOs
- [x] Inter-host propel
- [ ] Solve the arch-x86.h dependency problem
  - A possible solution is to expose codegenAPI in dyninst
- [ ] OpenMP hangs during parsing
- [ ] Test suite
- [ ] Update MIST and SecStar

## How to Install
1. Copy example-make.config to be `config.mk`.
2. Edit `config.mk` to define each macro variable.
  - DYNINST_DIR: absolute path of Dyninst's lib directory
  - SP_DIR: absolute path of this package (the output of `pwd`), because this
    file is also used by Makefile, I don't put `pwd` in it.
  - DYNLINK: true for building shared library for agent, otherwise for building
    static library
  - PLATFORM: set to x86_64-unknown-linux2.4 or i386-unknown-linux2.4
3. Run `make spi` in directory x86_64-unknown-linux2.4/ or i386-unknown-linux2.4/ to build injector and libagent.so.  
4. Run `make test_agent` in one of the above directories to build example user agents
5. For more make options, see [Make Arguments](#make-arguments)

## How to Run
1. Export Runtime environment variables, see Environment Variable section for [detail](#environment-variables):
  1. `SP_DIR`
  2. `PLATFORM`
  3. `SP_AGENT_DIR`
  4. `PLATFORM`
2. Make sure that your system does not block non-child ptrace
  - To temporarily disable this measure (until a reboot), execute the following command:  
    `echo 0 > /proc/sys/kernel/yama/ptrace_scope`
  - To permanently disable this measure, edit the file `/etc/sysctl.d/10-ptrace.conf` and follow the directions in that file.
  - For more information, see https://wiki.ubuntu.com/SecurityTeam/Roadmap/KernelHardening
3. There are two ways to kick start the Self-Propel
   1. To use the environment variable LD_PRELOAD when starting the user process.  
   Ex:  `LD_PRELOAD=$SP_DIR/PLATFORM/test_agent/print_test_agent.so [EXECUTABLE]`
   1. To use the injector to force a running process to load agent library, note that injector has two modes, pid injection and port injection    
   Ex:  `$SP_DIR/$PLATFORM/injector.exe pid [PID]` or `$SP_DIR/$PLATFORM/injector.exe port [PORT NUMBER]`
### Interprocess Propel
- Local Machine
  - Interprocess propelling relies on the following environment variables: `SP_DIR`, `PLATFORM`, `SP_AGENT_DIR`
- Inter-host
  - Export desired environment variables in your .bashrc file. Following environment variables are necessary:
    - SP_DIR
    - PLATFORM
    - SP_AGENT_DIR
    - Add `DYNINST_ROOT/lib`, `SP_DIR/PLATFORM` to your LD_LIBRARY_PATH


## Environment Variables
1. FOR DEBUGGING
  1. SP_COREDUMP: enables core dump when segfault happens
  2. SP_DEBUG: enables printing out debugging messages
  3. SP_TEST_RELOCINSN: only uses instruction relocation instrumentation worker
  4. SP_TEST_RELOCBLK: only uses call block relocation instrumentation worker
  5. SP_TEST_SPRING: only uses sprint block instrumentation worker
  6. SP_TEST_TRAP: only uses trap instrumentation worker
  7. SP_NO_TAILCALL: don't instrument tail calls
  8. SP_LIBC_MALLOC: will always use libc malloc
  9. SP_NO_LIBC_MALLOC: will never use libc malloc
2. FOR RUNTIME
  1. PLATFORM: 'i386-unknown-linux2.4' for x86 or 'x86_64-unknown-linux2.4' for x86-64
  2. SP_DIR: the root directory of self-propelled instrumentation.
  3. SP_AGENT_DIR: the directory path of agent shared library that will be injected.

## Shared memory id used
1. 1986+[user_process_pid]: for communication between injector process and user process

## Make Arguments
### For testing
  - make unittest: build unittests
  - make mutatee: build simple mutatees
  - make external_mutatee: build real world mutatees
  - make test: unittest + mutatee + external_mutatee

### For main self propelled
  - make injector_exe
  - make agent_lib
  - make spi: agent_lib + injector_exe

### For everything
  - make / make all: spi + test

### For cleaning
  - make clean_test: clean test stuffs
  - make clean: only clean core self-propelled stuffs, excluding dependency
  - make clean_all: clean everything, including dependency
  - make clean_objs: clean core self-propelled objs
