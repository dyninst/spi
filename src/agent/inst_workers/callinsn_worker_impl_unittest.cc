#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing RelocCallInsnWorker

namespace {


class RelocCallInsnWorkerTest : public testing::Test {
  public:
  RelocCallInsnWorkerTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(RelocCallInsnWorkerTest, pid_inject) {
}

}
