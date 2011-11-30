#ifndef SP_PAYLOAD_H_
#define SP_PAYLOAD_H_

#include "SpCommon.h"
#include "PatchCFG.h"

namespace sp {
  class SpContext;
  class SpPoint;

  typedef void (*PayloadFunc_t)(Dyninst::PatchAPI::Point* pt);
  typedef void* PayloadFunc;

  Dyninst::PatchAPI::PatchFunction* callee(Dyninst::PatchAPI::Point* pt_);
  void propel(Dyninst::PatchAPI::Point* pt_);
  long retval(SpPoint* pt);

  struct ArgumentHandle {
    ArgumentHandle();
    ~ArgumentHandle();
    char* insert_buf(size_t s);

    long offset;
    long num;
    std::vector<char*> bufs;
  };
  void* pop_argument(Dyninst::PatchAPI::Point* pt, ArgumentHandle* h, size_t size);
  void wrapper_before(Dyninst::PatchAPI::Point* pt, PayloadFunc_t before);
  void wrapper_after(Dyninst::PatchAPI::Point* pt, PayloadFunc_t before);
  bool can_work();
}

#endif /* SP_PAYLOAD_H_ */
