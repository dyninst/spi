#ifndef SP_PAYLOAD_H_
#define SP_PAYLOAD_H_

#include "SpAgentCommon.h"

namespace sp {
  class SpContext;
  class SpPoint;

  typedef void (*PayloadFunc_t)(ph::Point* pt);
  typedef void* PayloadFunc;

  ph::PatchFunction* callee(ph::Point* pt_);
  void propel(ph::Point* pt_);
  long retval(SpPoint* pt);

  struct ArgumentHandle {
    ArgumentHandle();
    ~ArgumentHandle();
    char* insert_buf(size_t s);

    long offset;
    long num;
    std::vector<char*> bufs;
  };
  void* pop_argument(ph::Point* pt, ArgumentHandle* h, size_t size);
  void wrapper_before(ph::Point* pt, PayloadFunc_t before);
  void wrapper_after(ph::Point* pt, PayloadFunc_t before);
  char start_tracing();
  bool is_ipc(int fd); 
}

#endif /* SP_PAYLOAD_H_ */
