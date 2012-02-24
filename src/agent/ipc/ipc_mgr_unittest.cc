#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpIpcMgr

namespace {


class IpcMgrTest : public testing::Test {
  public:
  IpcMgrTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(IpcMgrTest, pid_inject) {
}

}
