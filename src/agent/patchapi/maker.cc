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
#include "agent/patchapi/maker.h"
#include "agent/patchapi/point.h"

namespace sp {

  // For SpPoint
  ph::Point*
  SpPointMaker::mkFuncPoint(ph::Point::Type    t,
                            ph::PatchMgrPtr    m,
                            ph::PatchFunction* f) {
    return new SpPoint(t, m, f);
  }

  ph::Point*
  SpPointMaker::mkFuncSitePoint(ph::Point::Type    t,
                                ph::PatchMgrPtr    m,
                                ph::PatchFunction* f,
                                ph::PatchBlock*    b) {
    return new SpPoint(t, m, f, b);
  }

  ph::Point*
  SpPointMaker::mkBlockPoint(ph::Point::Type    t,
                             ph::PatchMgrPtr    m,
                             ph::PatchBlock*    b,
                             ph::PatchFunction* f) {
    return new SpPoint(t, m, b, f);
  }

  ph::Point*
  SpPointMaker::mkInsnPoint(ph::Point::Type      t,
                            ph::PatchMgrPtr      m,
                            ph::PatchBlock*      b,
                            Dyninst::Address     a,
                            in::Instruction i,
                            ph::PatchFunction *f) {
    return new SpPoint(t, m, b, a, i, f);
  }

  ph::Point*
  SpPointMaker::mkEdgePoint(ph::Point::Type    t,
                            ph::PatchMgrPtr    m,
                            ph::PatchEdge*     e,
                            ph::PatchFunction* f) {
    return new SpPoint(t, m, e, f);
  }

  // For CFGMaker
  ph::PatchFunction*
  SpCFGMaker::makeFunction(pe::Function* f,
                           ph::PatchObject* obj) {
    return new SpFunction(f, obj);
  }

  ph::PatchFunction*
  SpCFGMaker::copyFunction(ph::PatchFunction* f,
                           ph::PatchObject* obj) {
    return new SpFunction(f, obj);
  }

  ph::PatchBlock*
  SpCFGMaker::makeBlock(pe::Block* b,
                        ph::PatchObject* obj) {
    return new SpBlock(b, obj);
  }

  ph::PatchBlock*
  SpCFGMaker::copyBlock(ph::PatchBlock* b,
                        ph::PatchObject* obj) {
    return new SpBlock(b, obj);
  }

  ph::PatchEdge*
  SpCFGMaker::makeEdge(pe::Edge* e,
                       ph::PatchBlock* s,
                       ph::PatchBlock* t,
                       ph::PatchObject* o) {
    return (new SpEdge(e,
                       s ? s : o->getBlock(e->src()),
                       t ? t : o->getBlock(e->trg())));
  }

  ph::PatchEdge*
  SpCFGMaker::copyEdge(ph::PatchEdge* e,
                       ph::PatchObject* o) {
    return (new SpEdge(e,
                       o->getBlock(e->src()->block()),
                       o->getBlock(e->trg()->block())));
  }

}
