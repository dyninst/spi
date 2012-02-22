#ifndef SP_PAYLOAD_H_
#define SP_PAYLOAD_H_

#include "agent/cfg.h"
#include "common/common.h"

namespace sp {
  class SpContext;
  class SpPoint;

  // ------------------------
  //       Private things
  // ------------------------
  typedef void (*PayloadFunc_t)(ph::Point* pt);
  typedef void* PayloadFunc;
  struct ArgumentHandle {
    ArgumentHandle();
    ~ArgumentHandle();
    char* insert_buf(size_t s);

    long offset;
    long num;
    std::vector<char*> bufs;
  };
  void wrapper_before(SpPoint* pt, PayloadFunc_t before);
  void wrapper_after(SpPoint* pt, PayloadFunc_t before);

  // ------------------------
  //       Public things
  // ------------------------

  // Only used in before_payload
  void propel(SpPoint* pt_);
  void* pop_argument(SpPoint* pt, ArgumentHandle* h, size_t size);

	// Implicitly call start_tracing()
  bool is_ipc_write(SpPoint*); 
  bool is_ipc_read(SpPoint*); 

  // Only used in after_payload
  long retval(SpPoint* pt);

  // Used in both payloads
  SpFunction* callee(SpPoint* pt_);
  char start_tracing(int fd);

}

#endif /* SP_PAYLOAD_H_ */
