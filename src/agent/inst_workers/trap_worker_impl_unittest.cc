#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing TrapWorker

namespace {


class TrapWorkerTest : public testing::Test {
  public:
  TrapWorkerTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(TrapWorkerTest, pid_inject) {
}

}
