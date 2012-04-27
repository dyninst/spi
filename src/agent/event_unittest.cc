#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

namespace base {

void foo1() {
  rand();
  rand();
}

void foo2() {
  srand(0);
  srand(0);
  srand(0);
}

void foo3() {
  time(0);
  time(0);
  time(0);
}

void foo4() {
  sleep(1);
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

  ++printf_count;

  sp::Propel(pt);
}

TEST_F(EventTest, func_event) {

  printf_count = 0;
  
  StringSet preinst_funcs;
  preinst_funcs.insert("base::foo1");
  preinst_funcs.insert("main");
  
  FuncEvent::ptr event = FuncEvent::Create(preinst_funcs);
  SpAgent::ptr agent = SpAgent::Create();
  agent->SetInitEvent(event);
  agent->SetInitEntry("count_entry");
  agent->Go();

  base::foo1(); // 2
  base::foo1(); // 2
  base::foo1(); // 2

  // 2 + 2 + 2 = 6
  EXPECT_EQ(printf_count, 6);
}

TEST_F(EventTest, call_event) {

  printf_count = 0;

  StringSet preinst_calls;
  preinst_calls.insert("srand");
  
  CallEvent::ptr event = CallEvent::Create(preinst_calls);
  SpAgent::ptr agent = SpAgent::Create();
  agent->SetInitEvent(event);
  agent->SetInitEntry("count_entry");
  agent->Go();

  base::foo2(); // 3
  base::foo2(); // 3
  base::foo2(); // 3
  base::foo2(); // 3

  // 3 + 3 + 3 + 3 = 12
  EXPECT_EQ(printf_count, 12);
}

TEST_F(EventTest, comb_event) {

  printf_count = 0;

  StringSet preinst_calls;
  preinst_calls.insert("time");
  CallEvent::ptr call_event = CallEvent::Create(preinst_calls);
  StringSet preinst_funcs;
  preinst_funcs.insert("base::foo4");
  FuncEvent::ptr func_event = FuncEvent::Create(preinst_funcs);

  EventSet event_set;
  event_set.insert(call_event);
  event_set.insert(func_event);
  CombEvent::ptr event = CombEvent::Create(event_set);
      
  SpAgent::ptr agent = SpAgent::Create();
  agent->SetInitEvent(event);
  agent->SetInitEntry("count_entry");
  agent->Go();

  base::foo3(); // 3
  base::foo3(); // 3
  base::foo4(); // 1

  // 3+3+1 = 7
  EXPECT_EQ(printf_count, 7);
}

}
