# Comment out SP_DEBUG flags to disable debugging output for components
# Debugging output causes problems with trap instrumentation, disable output if not needed
export SP_FDEBUG=1
export SP_DEBUG_INJECTOR=1
export SP_DEBUG_AGENT=1
export SP_DEBUG_COMMON=1
export SP_DEBUG_WORKER=1
export SP_DEBUG_IPC=1
export SP_DEBUG_PATCHAPI=1

#export SP_DEBUG_SIGTRAP=1
#export SP_COREDUMP=1
#export SP_TIMING=1

export SP_DIR=/path/to/spi
export SP_AGENT_DIR=/path/to/spi/x86_64-unknown-linux2.4/test_agent
export SP_DYNINST_DIR=/path/to/dyninst/dyninst_install/lib
export LD_LIBRARY_PATH=$SP_AGENT_DIR:$SP_DIR/$PLATFORM:$SP_DYNINST_DIR:$LD_LIBRARY_PATH
export PLATFORM=x86_64-unknown-linux2.4

rm -f tmp/spi/*
mkdir -p ./tmp/spi

LD_PRELOAD=/path/to/spi/x86_64-unknown-linux2.4/test_agent/Some_Agent.so ./some_program

