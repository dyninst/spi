#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpEvent

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


TEST_F(EventTest, pid_inject) {
}

}
