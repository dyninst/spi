#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing instrumentation on multi-process programs

namespace {


class IpcSystest : public testing::Test {
  public:
  IpcSystest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(IpcSystest, pid_inject) {
}

}
