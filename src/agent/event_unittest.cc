#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

namespace base {

void foo() {
  rand();
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

int printf_count = 0;

void count_entry(SpPoint* pt) {

  SpFunction* f = Callee(pt);
  if (!f) return;

  if (f->name().compare("rand") == 0) {
    ++printf_count;
  }
  sp::Propel(pt);
}

TEST_F(EventTest, func_event) {

  StringSet preinst_funcs;
  preinst_funcs.insert("base::foo");
  preinst_funcs.insert("main");
  
  FuncEvent::ptr event = FuncEvent::Create(preinst_funcs);
  SpAgent::ptr agent = SpAgent::Create();
  agent->SetInitEvent(event);
  agent->SetInitEntry("count_entry");
  agent->Go();

  base::foo();
  base::foo();
  base::foo();

  EXPECT_EQ(printf_count, 3);
}

}
