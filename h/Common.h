#ifndef SP_COMMON_H_
#define SP_COMMON_H_

#include <elf.h>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <elf.h>
#include <unistd.h>
#include <errno.h>

const int MAX_LEN = 256;        // Maximum symbol name length, in bytes
const int MAX_BUF_LEN = 10000;  // Maximum size of a buffer, in bytes
const int MAX_MM  = 50;         // Maximum number of loaded modules

typedef unsigned long ulong;

#endif /* SP_COMMON_H_ */
