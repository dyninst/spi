#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

namespace base {

void foo() {
  printf("I'm foo!\n");
}

}

namespace {

class EventTest : public testing::Test {
  public:
  EventTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(EventTest, func_event) {

  StringSet preinst_funcs;
  preinst_funcs.insert("base::foo");
  preinst_funcs.insert("main");
  
  FuncEvent::ptr event = FuncEvent::Create(preinst_funcs);
  SpAgent::ptr agent = SpAgent::Create();
  agent->SetInitEvent(event);
  agent->Go();
}

}
