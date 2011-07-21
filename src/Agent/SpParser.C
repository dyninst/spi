#include "SpParser.h"
#include "SpCommon.h"

#include "Symbol.h"
#include "Symtab.h"
#include "Function.h"
#include "AddrLookup.h"
#include "CodeObject.h"

using sp::SpParser;
using Dyninst::SymtabAPI::AddressLookup;
using Dyninst::SymtabAPI::Symtab;
using Dyninst::SymtabAPI::Symbol;
using Dyninst::ParseAPI::CodeObject;
using Dyninst::ParseAPI::CodeRegion;
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
    sp_debug("%s @ %lx", sym->name().c_str(), load_addr ? load_addr : sym->imageOffset());
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

/* Find the function that contains addr */
Dyninst::ParseAPI::Function* SpParser::findFunction(Dyninst::Address addr) {
  for (PatchObjects::iterator ci = patch_objs_.begin(); ci != patch_objs_.end(); ci++) {
    PatchObject* obj = *ci;
    SymtabCodeSource* cs = (SymtabCodeSource*)obj->co()->cs();
    Symtab* sym = cs->getSymtabObject();
    Dyninst::Address upper_bound = obj->codeBase() + cs->length();
    Dyninst::Address lower_bound = obj->codeBase();
    sp_debug("%s = [%lx, %lx]", sym->name().c_str(), lower_bound, upper_bound);
    if (addr >= lower_bound && addr <= upper_bound) {
      sp_debug("Find function in %s", sym->name().c_str());
      Dyninst::Address address = addr;
      Dyninst::SymtabAPI::Function* f;
      if (!sym->getContainingFunction(address, f)) {
        address -= lower_bound;
      }

      for (std::vector<CodeRegion*>::const_iterator ri = cs->regions().begin();
           ri != cs->regions().end(); ri++) {
        std::set<ParseAPI::Function*> funcs;
        obj->co()->findFuncs(*ri, address, funcs);
        if (funcs.size() > 0) {
          sp_debug("got function %s", (*funcs.begin())->name().c_str());
          break;
        }
      }
      break;
    }
  }
  return NULL;
}

Dyninst::Address SpParser::findGlobalVar(char* var) {
  Dyninst::Address pc = 0;
  for (PatchObjects::iterator pi = patch_objs_.begin(); pi != patch_objs_.end(); pi++) {
    PatchObject* obj = *pi;
    CodeObject* co = obj->co();
    SymtabCodeSource* cs = (SymtabCodeSource*)co->cs();
    Symtab* sym = cs->getSymtabObject();
    std::vector<Symbol*> symbols;
    std::string var_name(var);
    sym->findSymbol(symbols, var_name);
    if (symbols.size() > 0) {
      Dyninst::Address offset = symbols[0]->getOffset();
      Dyninst::Address pc_addr = offset + obj->codeBase();
      pc = *((Dyninst::Address*)pc_addr);
      sp_debug("%s's offset: %lx, abs addr: %lx", IJ_PC_VAR, offset, pc_addr);
      break;
    }
  }
  return pc;
}
