#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpPropeller

namespace {


class PropellerTest : public testing::Test {
  public:
  PropellerTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(PropellerTest, pid_inject) {
}

}
