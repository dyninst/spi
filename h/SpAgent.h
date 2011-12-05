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
    void set_init_before(string);
    void set_init_after(string);
    void set_init_propeller(SpPropeller::ptr);

    void set_parse_only(bool b);
    void set_directcall_only(bool b);
    void set_ipc(bool b);

    void go();

  protected:
    SpEvent::ptr init_event_;
    SpEvent::ptr fini_event_;
    SpParser::ptr parser_;
    SpPropeller::ptr init_propeller_;
    string init_before_;
    string init_after_;
    Dyninst::PatchAPI::PatchMgrPtr mgr_;
    SpContext* context_;

    bool parse_only_;
    bool directcall_only_;
    bool allow_ipc_;

    SpAgent();
};

}

#endif /* SP_AGENT_H_ */
