#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpAgent

namespace {


class AgentTest : public testing::Test {
  public:
  AgentTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(AgentTest, pid_inject) {
}

}
