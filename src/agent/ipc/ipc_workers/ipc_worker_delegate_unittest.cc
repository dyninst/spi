#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpIpcWorkerDelegate

namespace {


class IpcWorkerDelegateTest : public testing::Test {
  public:
  IpcWorkerDelegateTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(IpcWorkerDelegateTest, pid_inject) {
}

}
