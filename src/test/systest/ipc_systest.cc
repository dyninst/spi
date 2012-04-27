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
    mutatee_prefix_ = " LD_LIBRARY_PATH=test_mutatee::tmp/lib:$LD_LIBRARY_PATH ";
    preload_prefix_ = " LD_PRELOAD=$SP_DIR/$PLATFORM/test_agent/ipc_test_agent.so ";
	}

  protected:
  string mutatee_prefix_;
  string preload_prefix_;
  
  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(IpcSystest, pipe1_preload_before_fork) {
  string cmd;
  cmd = mutatee_prefix_ + preload_prefix_ + " test_mutatee/pipe1.exe";
  FILE* fp = popen(cmd.c_str(), "r");
  char buf[1024];
  int count = 0;
  while (fgets(buf, 1024, fp) != NULL) {
    // printf(buf);
    ++count;
  }
  // printf("count: %d\n", count);
  EXPECT_TRUE(count > 200);
  pclose(fp);
}

TEST_F(IpcSystest, pipe2_preload_after_fork) {
  string cmd;
  cmd = mutatee_prefix_ + " test_mutatee/pipe2.exe";
  system(cmd.c_str());
  /*
  FILE* fp = popen(cmd.c_str(), "r");
  char buf[1024];
  int count = 0;
  while (fgets(buf, 1024, fp) != NULL) {
    // printf(buf);
    ++count;
  }
  // printf("count: %d\n", count);
  EXPECT_TRUE(count > 200);
  pclose(fp);
  */
}

}
