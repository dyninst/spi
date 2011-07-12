#include "Parser.h"
#include "Common.h"

#include "Symtab.h"
#include "AddrLookup.h"
#include "CodeObject.h"

using sp::Parser;
using Dyninst::SymtabAPI::AddressLookup;
using Dyninst::SymtabAPI::Symtab;
using Dyninst::ParseAPI::CodeObject;
using Dyninst::ParseAPI::SymtabCodeSource;

Parser::Parser() : exe_co_(NULL) {
}

Parser::~Parser() {
  for (CodeSources::iterator i = code_srcs_.begin(); i != code_srcs_.end(); i++) {
    SymtabCodeSource* scs = static_cast<SymtabCodeSource*>(*i);
    delete scs;
  }
  for (CodeObjects::iterator i = code_objs_.begin(); i != code_objs_.end(); i++)
    delete *i;
}

Parser::ptr Parser::create() {
  return ptr(new Parser());
}

/* Default implementation is runtime parsing. */
Parser::CodeObjects& Parser::parse() {
  AddressLookup* al = AddressLookup::createAddressLookup(getpid());
  al->refresh();
  std::vector<Symtab*> tabs;
  al->getAllSymtabs(tabs);
  sp_debug("%d symtabs found", tabs.size());
  for (std::vector<Symtab*>::iterator i = tabs.begin(); i != tabs.end(); i++) {
    Symtab* sym = *i;
    Dyninst::Address load_addr;
    al->getLoadAddress(sym, load_addr);
    sp_debug("%x: %s", load_addr, sym->file().c_str());

    SymtabCodeSource* scs = new SymtabCodeSource(sym);
    code_srcs_.push_back(scs);
    CodeObject* co = new CodeObject(scs);
    code_objs_.push_back(co);
    co->parse();
    if (sym->isExec()) exe_co_ = co;
  }
  return code_objs_;
}

CodeObject* Parser::exe_co() {
  if (!exe_co_) {
    parse();
    if (!exe_co_) sp_perror("failed to parse binary");
  }
  return exe_co_;
}

Dyninst::ParseAPI::Function* Parser::findFunction(std::string name) {
  for (CodeObjects::iterator ci = code_objs_.begin(); ci != code_objs_.end(); ci++) {
    CodeObject::funclist& all = (*ci)->funcs();
    for (CodeObject::funclist::iterator fi = all.begin(); fi != all.end(); fi++) {
      if ((*fi)->name().compare(name) == 0) return *fi;
    }
  }
  return NULL;
}
