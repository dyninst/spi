#include "SpInc.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  sp::SpParser::ptr parser = sp::SpParser::create();
  sp::SyncEvent::ptr event = sp::SyncEvent::create();

  agent->set_parser(parser);
  agent->set_parse_only(true);

  agent->go();
}
