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
    prefix_ = "LD_LIBRARY_PATH=test_mutatee::tmp/lib:$LD_LIBRARY_PATH ";
    prefix_ += "LD_PRELOAD=$SP_DIR/$PLATFORM/test_agent/ipc_test_agent.so";
	}

  protected:
  string prefix_;
  
  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(IpcSystest, pipe1) {
  string cmd;
  cmd = prefix_ + " test_mutatee/pipe1.exe";
  system(cmd.c_str());
}

}
