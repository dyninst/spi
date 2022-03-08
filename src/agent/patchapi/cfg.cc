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

#include "agent/patchapi/cfg.h"
#include "agent/patchapi/object.h"

namespace sp {

// SpFunction

	SpObject* SpFunction::GetObject() const {
    return OBJ_CAST(obj());
	}

  std::string SpFunction::GetMangledName() {
    sb::Symtab* sym = GetObject()->symtab();
    assert(sym);
    dt::Address offset = function()->addr();
    std::vector<sb::Symbol*> symbols = sym->findSymbolByOffset(offset);
    if (!symbols.empty()) {
      sb::Region* reg;
      if ((reg = sym->findEnclosingRegion(offset))) {
        if (reg->getRegionName().compare(".plt") == 0) {
          return name();
        }
      }
    }
    if (symbols.size() == 0) {
      //sp_debug_patchapi("GetMangledName: found no symbol, returning empty string");
      return "";
    }
    return (symbols)[0]->getMangledName();
  }

  std::string SpFunction::GetPrettyName() {
    sb::Symtab* sym = GetObject()->symtab();
    assert(sym);
    dt::Address offset = function()->addr();
    /*
    sp_print("In %s, codeBase %lx, load_addr %lx, offset: %lx",
             sym->name().c_str(),
             GetObject()->codeBase(),
             GetObject()->load_addr(),
             offset);
    */
    std::vector<sb::Symbol*> symbols = sym->findSymbolByOffset(offset);

    if (!symbols.empty()) {
      sb::Region* reg;
      if ((reg = sym->findEnclosingRegion(offset))) {
        if (reg->getRegionName().compare(".plt") == 0) {
          return name();
        }
      }
    }

    if (symbols.size() == 0) {
      return "";
    }

    return (symbols)[0]->getPrettyName();
  }

// SpBlock

	bool SpBlock::save() {

		// Save the call instruction
		orig_call_addr_ = last();
		orig_call_insn_ = getInsn(orig_call_addr_);
    if (!orig_call_insn_.ptr()) return false;

		// Save the entire block
    char* blk_buf = (char*)start();
    for (unsigned i = 0; i < size(); i++) {
      orig_code_ += (char)blk_buf[i];
    }
		

		return true;
	}

	SpObject* SpBlock::GetObject() const {
    return OBJ_CAST(obj());
	}

}
