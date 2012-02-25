#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing instrumentation on unix core utilities

namespace {


class CoreUtilsTest : public testing::Test {
  public:
  CoreUtilsTest() {
	}

  protected:
		string cmd_;
		char last_[1024];

		void run(const char* agent, const char* prog) {
			cmd_ = "LD_PRELOAD=./";
			cmd_ += agent;
			cmd_ += " ";
			cmd_ += prog;
			FILE* fp = popen(cmd_.c_str(), "r");
			char buf[1024];
			while (fgets(buf, 1024, fp) != NULL) {
				memcpy(last_, buf, 1024);
			}
			pclose(fp);
		}

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(CoreUtilsTest, ls) {
  run("test_agent/print_test_agent.so", "test_mutatee/ls.exe");
	EXPECT_STREQ(last_, "exit\n");
}

}
