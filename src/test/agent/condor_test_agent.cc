#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;


void test_entry(SpPoint* pt) {

PatchFunction* f = Callee(pt);
  if (!f) return;
	sp_print("%s() in %s",
           f->name().c_str(),
           pt->GetObject()->name().c_str());
  sp::Propel(pt);
}

void spi_test_exit(SpPoint* pt) {
  PatchFunction* f = Callee(pt);
  if (!f) return;
  sp_print("Exit %s", f->name().c_str());
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();

  StringSet libs_to_inst;
  libs_to_inst.insert("libcondor_utils.so");
  libs_to_inst.insert("libclassad.so");
  libs_to_inst.insert("libvomsapi_gcc64dbg.so");
  agent->SetLibrariesToInstrument(libs_to_inst);

  StringSet funcs_not_to_inst;
  funcs_not_to_inst.insert("ExprTreeToString");
  //funcs_not_to_inst.insert("classad::ClassAdUnParser::Unparse()");
  //funcs_not_to_inst.insert("setbuf");
  agent->SetFuncsNotToInstrument(funcs_not_to_inst);
  
  agent->SetInitEntry("test_entry");
  agent->SetInitExit("spi_test_exit");
  agent->Go();
}

AGENT_FINI
void DumpOutput() {
}
