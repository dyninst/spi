#ifndef SP_PAYLOAD_H_
#define SP_PAYLOAD_H_

#include "SpCommon.h"
#include "PatchCFG.h"

namespace sp {
  class SpContext;
  typedef void (*PayloadFunc_t)(Dyninst::PatchAPI::Point* pt);
  typedef void* PayloadFunc;

  Dyninst::PatchAPI::PatchFunction* callee(Dyninst::PatchAPI::Point* pt_);
  void propel(Dyninst::PatchAPI::Point* pt_);

/*
class SpPayload {
  public:
    SpPayload(SpContext* c);
    //static void create(SpContext* c);
    //static void destroy();

    Dyninst::PatchAPI::PatchFunction* callee();
    void propell();
    SpPayload* instance(Dyninst::PatchAPI::Point*);
    //void set_point(Dyninst::PatchAPI::Point* p) { pt_ = p; }
  protected:
    // static SpPayload* instance_;
    Dyninst::PatchAPI::Point* pt_;
    SpContext* context_;
};
*/

}
//extern sp::SpPayload* g_payload;
//#define payload_mgr(p) g_payload->instance(p)


#endif /* SP_PAYLOAD_H_ */
