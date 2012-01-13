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

  TEST_F(ComponentTest, indirect_call) {
		FILE* fp = popen("LD_PRELOAD=./comp_test_agent.so ./indcall", "r");
		char buf[1024];

		// foo (indirect)
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "foo\n");

		// puts (called by above foo)
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "puts\n");

		// indirect call from foo
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "indirect call from foo\n");

		// bar (indirect)
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "bar\n");

		// bar (direct)
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "bar\n");

		pclose(fp);
	}

}
