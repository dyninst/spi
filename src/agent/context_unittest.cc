#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpContext

namespace {


class ContextTest : public testing::Test {
  public:
  ContextTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(ContextTest, pid_inject) {
}

}
