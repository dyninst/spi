// Google test/mock headers
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Self-propelled stuffs
#include "SpUtils.h"

using namespace sp;
using namespace std;

class UtilsTest : public testing::Test {
protected:
	virtual void SetUp() {
	}
	virtual void TearDown() {
	}
};

TEST_F(UtilsTest, get_file_text) {
	string content = get_file_text("/proc/self/cmdline");
	EXPECT_STREQ(content.c_str(), "./utils_test");
}

TEST_F(UtilsTest, is_illegal_exe) {
	StringSet illegal_exe;
	illegal_exe.insert("utils_test");
	EXPECT_TRUE(is_illegal_exe(illegal_exe));
}

