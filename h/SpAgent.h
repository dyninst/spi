#ifndef SP_AGENT_H_
#define SP_AGENT_H_

#include "PatchMgr.h"
#include "SpParser.h"
#include "SpEvent.h"
#include "SpPayload.h"
#include "SpPropeller.h"

#define AGENT_INIT __attribute__((constructor))

namespace sp {

/* Agent is to manage Agent's configuration parameters, including:
   - The event that causes the activation of instrumentation.
     - Default: the callees of current function
   - The event that causes the deactivation of instrumentation.
     - Default: the exit of this process
   - The initial user-defined payload code to execute when activation.
     - Default: print out the current function name
   - The parser to parse CFG structures of current running process.
     - Default: runtime parsing
 */
class SpAgent : public dyn_detail::boost::enable_shared_from_this<SpAgent> {
  friend class SpContext;
  public:
    typedef dyn_detail::boost::shared_ptr<SpAgent> ptr;
    static ptr create();
    virtual ~SpAgent();

    void set_parser(SpParser::ptr);
    void set_init_event(SpEvent::ptr);
    void set_fini_event(SpEvent::ptr);
    void set_init_head(string);
    void set_init_tail(string);
    void set_init_propeller(SpPropeller::ptr);

    void go();

  protected:
    SpEvent::ptr init_event_;
    SpEvent::ptr fini_event_;
    SpParser::ptr parser_;
    SpPropeller::ptr init_propeller_;
    string init_head_;
    string init_tail_;
    Dyninst::PatchAPI::PatchMgrPtr mgr_;
    SpContext* context_;

    SpAgent();
};

}

#endif /* SP_AGENT_H_ */
