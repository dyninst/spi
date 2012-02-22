// Google test/mock headers
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Self-propelled stuffs
#include "SpInc.h"
#include "patchAPI/h/PatchMgr.h"

using namespace sp;
using namespace std;

// Test Parser
namespace {

  class AddressSpaceTest : public testing::Test {
	protected:
    virtual void SetUp() {
		}
    virtual void TearDown() {
		}
	};

  TEST_F(AddressSpaceTest, malloc_free) {
		sp::SpAgent::ptr agent = sp::SpAgent::create();
		agent->set_parse_only(true);
		agent->go();
		sp::SpParser::ptr parser = agent->parser();
		ph::PatchMgrPtr mgr = parser->mgr();
		ph::AddrSpace* as = mgr->as();

		dt::Address buf = 0;
		for (int i = 0; i < 100; i++) {
			buf = as->malloc(parser->exe_obj(), 68, 0);
			//			memset((void*)buf, 0, 68);
			as->free(parser->exe_obj(), buf);

			buf = as->malloc(parser->exe_obj(), 168, 0);
			//			memset((void*)buf, 1, 168);
			as->free(parser->exe_obj(), buf);

			buf = as->malloc(parser->exe_obj(), 800, 0);
			//			memset((void*)buf, 2, 800);
			as->free(parser->exe_obj(), buf);

			buf = as->malloc(parser->exe_obj(), 8000, 0);
			//			memset((void*)buf, 3, 8000);
			as->free(parser->exe_obj(), buf);
		}
	}
}
