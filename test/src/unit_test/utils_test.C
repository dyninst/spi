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
/*
TEST_F(UtilsTest, get_file_text) {
	string content = get_file_text("/proc/self/cmdline");
	EXPECT_STREQ(content.c_str(), "./utils_test");
}

TEST_F(UtilsTest, is_illegal_exe) {
	StringSet illegal_exe;
	illegal_exe.insert("utils_test");
	EXPECT_TRUE(is_illegal_exe(illegal_exe));
}

TEST_F(UtilsTest, addr_to_pid_without_injector) {
	FILE* fp = popen("./tcp_server", "r");

	PidSet pid_set;
	pid_t p = -1;
	int retry = 4;
	do {
		addr_to_pids(0, 0, 0, 3490, pid_set);
		for (PidSet::iterator i = pid_set.begin(); i != pid_set.end(); i++) {
			// sp_print("pid = %d", *i);
			p = *i;
		}
		--retry;
		if (pid_set.size() <= 0) sleep(1);
	} while (pid_set.size() <=0 && retry > 0);
	EXPECT_TRUE(p > 0);
	system("killall tcp_server");
	pclose(fp);
	// sp_print("PID = %d", p);
}

*/
