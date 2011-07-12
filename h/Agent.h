#ifndef SP_AGENT_H_
#define SP_AGENT_H_

#include "PatchMgr.h"
#include "Parser.h"
#include "Event.h"

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
class Agent {
  public:
    typedef dyn_detail::boost::shared_ptr<Agent> ptr;
    static ptr create();

    void setParser(Parser::ptr parser);
    void go();


  protected:
    // initPayload
    Event::ptr initEvent_;
    Event::ptr finiEvent_;
    Parser::ptr parser_;
    Dyninst::PatchAPI::PatchMgrPtr mgr_;

    Agent();
};

}

#endif /* SP_AGENT_H_ */
