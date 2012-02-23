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

#ifndef _SPPOINTMAKER_H_
#define _SPPOINTMAKER_H_

#include "patchAPI/h/CFGMaker.h"

#include "common/common.h"

// PatchAPI stuffs: build customized points and CFG
namespace sp {
  class SpPointMaker : public ph::PointMaker {
  protected:

    virtual ph::Point*
			mkFuncPoint(ph::Point::Type    t,
                  ph::PatchMgrPtr    m,
                  ph::PatchFunction* f);

    virtual ph::Point*
			mkFuncSitePoint(ph::Point::Type    t,
                      ph::PatchMgrPtr    m,
                      ph::PatchFunction* f,
                      ph::PatchBlock*    b);

    virtual ph::Point*
			mkBlockPoint(ph::Point::Type    t,
                   ph::PatchMgrPtr    m,
                   ph::PatchBlock*    b,
                   ph::PatchFunction* f);

    virtual ph::Point*
			mkInsnPoint(ph::Point::Type      t,
                  ph::PatchMgrPtr      m,
                  ph::PatchBlock*      b,
                  Dyninst::Address     a,
                  in::Instruction::Ptr i,
                  ph::PatchFunction *f);

    virtual ph::Point*
			mkEdgePoint(ph::Point::Type    t,
                  ph::PatchMgrPtr    m,
                  ph::PatchEdge*     e,
                  ph::PatchFunction* f);
  };


	class SpCFGMaker : public ph::CFGMaker {
  public:
    SpCFGMaker() {}
    virtual ~SpCFGMaker() {}

    virtual ph::PatchFunction*
			makeFunction(pe::Function* f,
									 ph::PatchObject* obj);

    virtual ph::PatchFunction*
			copyFunction(ph::PatchFunction* f,
									 ph::PatchObject* obj);

    virtual ph::PatchBlock*
			makeBlock(pe::Block* b,
								ph::PatchObject* obj);

    virtual ph::PatchBlock*
			copyBlock(ph::PatchBlock* b,
								ph::PatchObject* obj);

    virtual ph::PatchEdge*
			makeEdge(pe::Edge* e,
							 ph::PatchBlock* s,
							 ph::PatchBlock* t,
							 ph::PatchObject* o);

    virtual ph::PatchEdge*
			copyEdge(ph::PatchEdge* e,
							 ph::PatchObject* o);
	};

}

#endif /* _SPPOINTMAKER_H_ */
