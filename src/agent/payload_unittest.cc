#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpPayload

namespace {


class PayloadTest : public testing::Test {
  public:
  PayloadTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(PayloadTest, exit_payload) {
  std::string cmd;
  cmd = "LD_LIBRARY_PATH=test_mutatee:$LD_LIBRARY_PATH ";
  cmd += "LD_PRELOAD=test_agent/payload_test_agent.so ";
	cmd += "test_mutatee/indcall.exe";
  system(cmd.c_str());
  /*
	FILE* fp = popen(cmd.c_str(), "r");
	char buf[1024];
	while (fgets(buf, 1024, fp) != NULL) {
    fprintf(stderr, "%s", buf);
  }
  pclose(fp);
  */
}

}
