#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpFunction and SpBlock

namespace {


class CfgTest : public testing::Test {
  public:
  CfgTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(CfgTest, pid_inject) {
}

}
