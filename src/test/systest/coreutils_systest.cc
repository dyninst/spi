#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing instrumentation on unix core utilities

namespace {


class CoreUtilsTest : public testing::Test {
  public:
  CoreUtilsTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(CoreUtilsTest, pid_inject) {
}

}
