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

// This implements the PatchAPI::AddrSpace, which provides memory
// allocation routines, and other methods to manipulate address space

#ifndef _SPADDRSPACE_H_
#define _SPADDRSPACE_H_

#include "patchAPI/h/AddrSpace.h"

#include "common/common.h"

namespace sp {
class SpObject;
class SpAddrSpace : public ph::AddrSpace {
   friend class SpInstrumenter;
  public:
    static SpAddrSpace* Create(ph::PatchObject*);

    // Implement memory allocation stuffs
    virtual dt::Address malloc(ph::PatchObject* obj,
                               size_t size,
                               dt::Address near) OVERRIDE;

    virtual bool write(ph::PatchObject* obj,
                       dt::Address to,
                       dt::Address from,
                       size_t size) OVERRIDE;

    virtual bool free(ph::PatchObject* obj,
                      dt::Address orig) OVERRIDE;

    // Changes original code's access permission
    bool SetCodePermission(dt::Address addr,
                           size_t length,
                           int perm);

    // Restores original code's access permission
    bool RestoreCodePermission(dt::Address addr,
                               size_t length);
    
    // Changes snippet buffer's access permission
    bool SetSnippetPermission(dt::Address addr,
                              size_t length,
                              int perm);

  protected:
    SpAddrSpace();
};

}

#endif /* _SPADDRSPACE_H_ */
