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

  class ComponentTest : public testing::Test {
	protected:
    virtual void SetUp() {
		}
    virtual void TearDown() {
		}
	};

  TEST_F(ComponentTest, direct_call_only) {
		FILE* fp = popen("LD_PRELOAD=./directcall_test_agent.so ./indcall", "r");
		char buf[1024];

		// bar (direct)
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "bar\n");

		pclose(fp);
	}


	// Instrument indirect function call
  TEST_F(ComponentTest, indirect_call) {
		FILE* fp = popen("LD_PRELOAD=./comp_test_agent.so ./indcall", "r");
		char buf[1024];

		// foo (indirect)
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "foo\n");

		// sleep (called by above foo)
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "sleep\n");

		// bar (indirect)
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "bar\n");

		// bar (direct)
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "bar\n");

		pclose(fp);
	}


	// Instrument functions in shared library
  TEST_F(ComponentTest, shared_lib_call) {
		FILE* fp = popen("LD_PRELOAD=./comp_test_agent.so ./libcall", "r");
		char buf[1024];

		// test_lib_foo() in the shared library
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "test_lib_foo\n");

		// printf (called by above test_lib_foo)
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "printf\n");

		// hello, 1980
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "hello, 1980\n");

		pclose(fp);
	}

  TEST_F(ComponentTest, spring_board) {
		FILE* fp = popen("SP_TEST_SPRING=1 LD_PRELOAD=./comp_test_agent.so ./libcall", "r");
		char buf[1024];

		// bar (direct)
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "test_lib_foo\n");

		pclose(fp);
	}

	// Trap-only instrumentation
  TEST_F(ComponentTest, trap_only) {
		FILE* fp = popen("LD_PRELOAD=./trap_test_agent.so ./libcall", "r");
		char buf[1024];

		// test_lib_foo() in the shared library
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "test_lib_foo\n");

		// printf (called by above test_lib_foo)
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "printf\n");

		// hello, 1980
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "hello, 1980\n");

		pclose(fp);
	}

}
