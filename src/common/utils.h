/*
 * Copyright (c) 1996-2011 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// This module provides a set of common utilities, including timers for
// profiling, ipc utilities, /proc manipulation utilities ...

#ifndef _SPUTILS_H_
#define _SPUTILS_H_

#include <netinet/in.h>
#include <sys/types.h>

#include <set>
#include <string>

#include "common/common.h"

namespace sp {
  typedef std::set<pid_t> PidSet;
  typedef std::set<std::string> StringSet;

  // Timer for profiling.
  // There are only 10 timers to use (0 ~ 10). So the timer_id should be
  // within [0, 9]. Otherwise, those functions does nothing.
  COMMON_EXPORT void SetupTimer(const int timer_id);
  COMMON_EXPORT void StartTimer(const int timer_id);
  COMMON_EXPORT void StopTimer(const int timer_id);
  COMMON_EXPORT void ResetTimer(const int timer_id);
  COMMON_EXPORT double GetTimer(const int timer_id);
  COMMON_EXPORT void PrintTime(const char* message,
                           const int timer_id);

  // Is 32-bit displacement?
  COMMON_EXPORT bool IsDisp32(const long disp);

  // Is 8-bit displacement?
  COMMON_EXPORT bool IsDisp8(const long disp);

  COMMON_EXPORT bool IsPcRegister(const dt::MachRegister reg);

  COMMON_EXPORT ino_t GetInodeFromFileDesc(const int fd);

  // Is this process using this inode?
  COMMON_EXPORT bool PidUsesInode(const int pid,
                              const ino_t inode);

  COMMON_EXPORT void GetPidsFromFileDesc(const int fd,
                                     PidSet& pid_set);

  // Get pids that are associated with the local/remote address pair
  COMMON_EXPORT void GetPidsFromAddrs(const char* const loc_ip,
                                  const char* const loc_port,
                                  const char* const rem_ip,
                                  const char* const rem_port,
                                  PidSet& pid_set);

  COMMON_EXPORT in_addr_t GetIpFromHostname(const char* const hostname,
                                        char* const ip,
                                        const size_t ip_len);


  // Is current executable an illegal program in `illegal_prog_names`?
  // We use this, because we want to avoid instrumenting some programs, e.g.,
  // unix utilities used in self-propelled core
  COMMON_EXPORT bool IsIllegalProgram();

  // Gets text content from a file. If file doesn't exist, return "".
  COMMON_EXPORT const char* GetFileText(const char* filename);

  // Gets full path of the executable file
  COMMON_EXPORT const char* GetExeName();

  // IPC stuffs
  COMMON_EXPORT bool IsPipe(const int fd);
  COMMON_EXPORT bool IsTcp(const int fd);
  COMMON_EXPORT bool IsUdp(const int fd);
  COMMON_EXPORT bool IsIpc(const int fd);

  // Socket programing
  COMMON_EXPORT bool GetAddress(sockaddr_storage* const sa,
                            char* const host,
                            const size_t host_len,
                            char* const service,
                            const size_t service_len);

  COMMON_EXPORT bool GetLocalAddress(const int fd,
                                 sockaddr_storage* const out);

  COMMON_EXPORT bool GetRemoteAddress(const int fd,
                                  sockaddr_storage* const out);

  // Serialization for CodeObject
  // TODO(wenbin)
  COMMON_EXPORT bool SerializeCodeObject(pe::CodeObject* co,
                                     const char* dir);
  COMMON_EXPORT pe::CodeObject* DeserializeCodeObject(const char* dir,
                                                  const char* file);

  // Lock / unlock for multithreading
  typedef void* SpTid;
  enum {
    SP_INITIAL_LOCK_TID = -129,
    SP_LIVE_LOCK        = -1,
    SP_DEAD_LOCK        = -2
  };
  typedef struct {
    volatile int mutex;
    SpTid tid;
  } SpLock;
  
  COMMON_EXPORT SpTid GetThreadId();
  COMMON_EXPORT int FreeLock(SpLock* const lock);
  COMMON_EXPORT int InitLock(SpLock* const lock);
  COMMON_EXPORT int Lock(SpLock* const lock);
  COMMON_EXPORT int Unlock(SpLock* const lock);

  // Shared memory
  COMMON_EXPORT void* GetSharedMemory(int id,
                                      size_t size);
}

#endif   // _SPUTILS_H_
