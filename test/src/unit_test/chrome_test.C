// Google test/mock headers
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Self-propelled stuffs
#include "SpIpcMgr.h"
#include "SpChannel.h"
#include "SpUtils.h"

using namespace sp;
using namespace std;

namespace {

  class ChromeTest : public testing::Test {
	public:
		ChromeTest() {
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

  TEST_F(ChromeTest, simple_inject) {
		// run("inject_test_agent.so", "/scratch/chromium/bin/chromium");
		system("LD_PRELOAD=inject_test_agent.so /scratch/chromium/bin/chromium");
	}
}
