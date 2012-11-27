#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing instrumentation on multi-process programs

namespace {


class PipeSystest : public testing::Test {
  public:
  PipeSystest() {
    mutatee_prefix_ = " LD_LIBRARY_PATH=test_mutatee:tmp/lib:$LD_LIBRARY_PATH ";
    mutatee_prefix_ += " SP_AGENT_DIR=$SP_DIR/$PLATFORM/test_agent ";
    preload_prefix_ = " LD_PRELOAD=$SP_DIR/$PLATFORM/test_agent/ipc_test_agent.so ";
	}

  protected:
  string mutatee_prefix_;
  string preload_prefix_;
  
  virtual void SetUp() {
	}

	virtual void TearDown() {
	}

  void test_result(const char* filename) {
    bool inst = false;
    char line[1024];
    FILE* fp = fopen(filename, "r");
    while (fgets(line, 1024, fp)) {
      if (strstr(line, "Read size:")) {
        inst = true;
        break;
      }
      if (strstr(line, "Write size:")) {
        inst = true;
        break;
      }
    }
    ASSERT_TRUE(inst);
    fclose(fp);
  }
};

TEST_F(PipeSystest, pipe1_preload_before_fork) {
  string cmd;
  cmd = mutatee_prefix_ + preload_prefix_ + " test_mutatee/pipe1.exe >& /tmp/pipe1.log";
  // system(cmd.c_str());

  FILE* fp = popen(cmd.c_str(), "r");
  PidSet client_pid_set;
  GetPidsFromFileDesc(fileno(fp), client_pid_set);
  if (client_pid_set.size() > 0) {
    int status;
    wait(&status);
  }
  pclose(fp);
  test_result("/tmp/pipe1.log");
}

TEST_F(PipeSystest, pipe2_preload_after_fork) {
  string cmd;
  cmd = mutatee_prefix_ + " test_mutatee/pipe2.exe >& /tmp/pipe2.log";
  //  system(cmd.c_str());

  FILE* fp = popen(cmd.c_str(), "r");
  PidSet client_pid_set;
  GetPidsFromFileDesc(fileno(fp), client_pid_set);
  if (client_pid_set.size() > 0) {
    int status;
    wait(&status);
  }
  pclose(fp);
  test_result("/tmp/pipe2.log");
}

TEST_F(PipeSystest, pipe3_preload_before_popen) {
  string cmd;
  cmd = mutatee_prefix_ + preload_prefix_ + " test_mutatee/pipe3.exe >& /tmp/pipe3.log";
  // system(cmd.c_str());

  FILE* fp = popen(cmd.c_str(), "r");
  PidSet client_pid_set;
  GetPidsFromFileDesc(fileno(fp), client_pid_set);
  if (client_pid_set.size() > 0) {
    int status;
    wait(&status);
  }
  pclose(fp);
  test_result("/tmp/pipe3.log");
}

}
