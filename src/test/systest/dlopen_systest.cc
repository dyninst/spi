#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing parsing of newly loaded shared library via dlopen

namespace {


class DlopenTest : public testing::Test {
  public:
  DlopenTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(DlopenTest, simple) {
  std::string cmd;
  cmd = "LD_LIBRARY_PATH=test_mutatee:$LD_LIBRARY_PATH ";
  cmd += "LD_PRELOAD=test_agent/dlopen_test_agent.so ";
	cmd += "test_mutatee/dlopen_mutatee.exe";
  // system(cmd.c_str());
	FILE* fp = popen(cmd.c_str(), "r");
	char buf[1024];
	EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
  EXPECT_STREQ(buf, "1");
  pclose(fp);
}

}
