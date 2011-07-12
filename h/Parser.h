#ifndef SP_PARSER_H_
#define SP_PARSER_H_

#include "PatchObject.h"
#include "PatchMgr.h"

namespace sp {

/*
   Parser is to parse the CFG structures of the mutatee process.
 */
class Parser : public Dyninst::PatchAPI::CFGMaker {
  public:
    typedef dyn_detail::boost::shared_ptr<Parser> ptr;
    virtual ~Parser();
    static ptr create();

    typedef std::vector<Dyninst::ParseAPI::CodeObject*> CodeObjects;
    typedef std::vector<Dyninst::PatchAPI::PatchObject*> PatchObjects;
    virtual PatchObjects& parse();
    Dyninst::PatchAPI::PatchObject* exe_obj();

    // Mainly for debug
    Dyninst::ParseAPI::Function* findFunction(std::string name);

  protected:
    typedef std::vector<Dyninst::ParseAPI::CodeSource*> CodeSources;
    CodeSources code_srcs_;
    CodeObjects code_objs_;
    PatchObjects patch_objs_;
    Dyninst::PatchAPI::PatchObject* exe_obj_;

    Parser();
};

}

#endif /* SP_PARSER_H_ */
