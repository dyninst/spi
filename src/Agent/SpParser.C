#include "SpParser.h"
#include "SpCommon.h"

#include "Symtab.h"
#include "AddrLookup.h"
#include "CodeObject.h"

using sp::SpParser;
using Dyninst::SymtabAPI::AddressLookup;
using Dyninst::SymtabAPI::Symtab;
using Dyninst::ParseAPI::CodeObject;
using Dyninst::ParseAPI::SymtabCodeSource;
using Dyninst::PatchAPI::PatchObject;

SpParser::SpParser() : exe_obj_(NULL) {
}

SpParser::~SpParser() {
  for (CodeSources::iterator i = code_srcs_.begin(); i != code_srcs_.end(); i++) {
    SymtabCodeSource* scs = static_cast<SymtabCodeSource*>(*i);
    delete scs;
  }
  for (CodeObjects::iterator i = code_objs_.begin(); i != code_objs_.end(); i++)
    delete *i;
}

SpParser::ptr SpParser::create() {
  return ptr(new SpParser());
}

/* Default implementation is runtime parsing. */
SpParser::PatchObjects& SpParser::parse() {
  AddressLookup* al = AddressLookup::createAddressLookup(getpid());
  al->refresh();
  std::vector<Symtab*> tabs;
  al->getAllSymtabs(tabs);
  sp_debug("%d symtabs found", tabs.size());
  for (std::vector<Symtab*>::iterator i = tabs.begin(); i != tabs.end(); i++) {
    Symtab* sym = *i;
    Dyninst::Address load_addr;
    al->getLoadAddress(sym, load_addr);

    SymtabCodeSource* scs = new SymtabCodeSource(sym);
    code_srcs_.push_back(scs);
    CodeObject* co = new CodeObject(scs);
    code_objs_.push_back(co);
    co->parse();

    PatchObject* patch_obj = PatchObject::create(co, load_addr ? load_addr : sym->getLoadAddress());
    patch_objs_.push_back(patch_obj);
    if (sym->isExec()) exe_obj_ = patch_obj;
  }
  return patch_objs_;
}

PatchObject* SpParser::exe_obj() {
  if (!exe_obj_) {
    parse();
    if (!exe_obj_) sp_perror("failed to parse binary");
  }
  return exe_obj_;
}

Dyninst::ParseAPI::Function* SpParser::findFunction(std::string name) {
  for (CodeObjects::iterator ci = code_objs_.begin(); ci != code_objs_.end(); ci++) {
    CodeObject::funclist& all = (*ci)->funcs();
    for (CodeObject::funclist::iterator fi = all.begin(); fi != all.end(); fi++) {
      if ((*fi)->name().compare(name) == 0) return *fi;
    }
  }
  return NULL;
}
