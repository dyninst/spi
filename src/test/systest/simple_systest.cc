#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing instrumentation on simple mutatees

namespace {


class SimpleTest : public testing::Test {
  public:
  SimpleTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(SimpleTest, pid_inject) {
}

}
