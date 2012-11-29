#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing instrumentation on multithreaded programs
// Multithread, single process

namespace {


class MultithreadTest : public testing::Test {
  public:
  MultithreadTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(MultithreadTest, simple) {
  std::string cmd;
  cmd = "LD_LIBRARY_PATH=test_mutatee:$LD_LIBRARY_PATH ";
  cmd += "LD_PRELOAD=test_agent/multithread_test_agent.so ";
	cmd += "test_mutatee/multithread.exe";
  //  system(cmd.c_str());

	FILE* fp = popen(cmd.c_str(), "r");
	char buf[1024];
  int count = 0;
	while (fgets(buf, 1024, fp) != NULL) {
    count++;
    // fprintf(stderr, "%s", buf);
  }
  pclose(fp);
  EXPECT_TRUE(count > 40);

}

}
