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

  class IntegrationTest : public testing::Test {
	public:
		IntegrationTest() {
		}
	protected:
		string cmd_;
		char last_[1024];

		void run(const char* agent, const char* prog) {
			cmd_ = "LD_PRELOAD=./";
			cmd_ += agent;
			cmd_ += " ";
			// cmd_ += "comp_test_agent.so ";
			cmd_ += prog;
			
			// fprintf(stderr, "%s\n", cmd_.c_str());
			FILE* fp = popen(cmd_.c_str(), "r");
			char buf[1024];
			while (fgets(buf, 1024, fp) != NULL) {
				// fprintf(stderr, buf);
				memcpy(last_, buf, 1024);
			}

			pclose(fp);
		}

    virtual void SetUp() {
		}
    virtual void TearDown() {
		}
	};

  TEST_F(IntegrationTest, ls) {
		run("comp_test_agent.so", "mutatee/bin/ls");
		// The last function is exit()
		EXPECT_STREQ(last_, "exit\n");

	}

  TEST_F(IntegrationTest, df) {
		run("comp_test_agent.so", "mutatee/bin/df");
		// The last function is exit()
		EXPECT_STREQ(last_, "exit\n");

	}

  TEST_F(IntegrationTest, od) {
		run("comp_test_agent.so", "mutatee/bin/od indcall.o");
		// The last function is exit()
		EXPECT_STREQ(last_, "exit\n");

	}

  TEST_F(IntegrationTest, sort) {
		run("comp_test_agent.so", "mutatee/bin/sort Makefile");
		// The last function is exit()
		EXPECT_STREQ(last_, "exit\n");

	}

  TEST_F(IntegrationTest, gcc) {
		string gcc_path = "LD_LIBRARY_PATH=/scratch/wenbin/software/lib:$LD_LIBRARY_PATH /scratch/wenbin/software/libexec/gcc/x86_64-unknown-linux-gnu/4.5.2/cc1";
		string arg_path = "/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/user_agent/count/x86_64-unknown-linux2.4/200.i";
		string cmd = gcc_path + " " + arg_path;
		run("count_test_agent.so", cmd.c_str());
	}

}
