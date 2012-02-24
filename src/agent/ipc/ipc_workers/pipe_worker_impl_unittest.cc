#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpPipeWorker

namespace {


class PipeWorkerTest : public testing::Test {
  public:
  PipeWorkerTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(PipeWorkerTest, pid_inject) {
}

}
