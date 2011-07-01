cc = gcc
CC = g++
flag = -fPIC -O0 -g

DYNINST_ROOT = /afs/cs.wisc.edu/p/paradyn/development/wenbin/dyninst
INC_DIR =  -I$(DYNINST_ROOT)/include -I$(DYNINST_ROOT)/dyninst/patchAPI/h -I$(DYNINST_ROOT)/dyninst -I$(DYNINST_ROOT)/dyninst/dyninstAPI/src -I$(DYNINST_ROOT)/dyninst/patchAPI/src -I$(DYNINST_ROOT)/dyninst/patchAPI/src/dyninst -I$(DYNINST_ROOT)/dyninst/dyninstAPI/src/Relocation

LIB_DIR = -L$(DYNINST_ROOT)/$(PLATFORM)/lib 
LIB     = -lcommon -lpcontrol -lpthread
CC  = g++ 
cc  = gcc
CXXFLAG = -Wall -g -Dos_linux -Darch_x86 -Darch_32bit -Di386-unknown-linux2.4  -Dcap_ptrace -Dcap_stripped_binaries -Dcap_async_events -Dcap_threads -Dcap_dynamic_heap -Dcap_relocation -Dcap_dwarf -Dcap_32_64 -Dcap_liveness -Dcap_fixpoint_gen -Dcap_noaddr_gen -Dcap_mutatee_traps -Dcap_binary_rewriter -Dcap_registers -Dcap_instruction_api -Dcap_serialization -Dcap_instruction_replacement -Dcap_tramp_liveness -Dbug_syscall_changepc_rewind -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
CFLAG = -Wall -fPIC

PLATFORM = i386-unknown-linux2.4

VPATH += $(DYNINST_ROOT)/dyninst/common/h
VPATH += $(DYNINST_ROOT)/dyninst/common
VPATH += $(DYNINST_ROOT)/dyninst/

all:  Mutatee libagent.so Injector

Injector: Injector.C
	$(CC) -o Injector Injector.C $(LIB_DIR) $(INC_DIR) $(CXXFLAG) $(LIB)

libagent.so: Agent.c
	$(cc) -shared -o $@ $< -ldl -fPIC -O0 -g

Mutatee: Mutatee.c
	$(cc) -o $@ $< -g
	execstack -s $@

clean:
	rm -f *.o *.so Injector Mutatee  core*
