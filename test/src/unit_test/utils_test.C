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

TEST_F(UtilsTest, addr_to_pid_without_injector) {
	FILE* fp = popen("./tcp_server6", "r");

	PidSet pid_set;
	pid_t p = -1;
	int retry = 4;
	do {
		addr_to_pids((char*)"0.0.0.0", (char*)"0", (char*)"0.0.0.0", (char*)"3490", pid_set);
		for (PidSet::iterator i = pid_set.begin(); i != pid_set.end(); i++) {
			p = *i;
		}
		--retry;
		if (pid_set.size() <= 0) sleep(1);
	} while (pid_set.size() <=0 && retry > 0);
	EXPECT_TRUE(p > 0);
	system("killall tcp_server6");
	pclose(fp);
}

TEST_F(UtilsTest, get_exe_name) {
	string exe = sp::get_exe_name();
	sp_print("%s at pid=%d", exe.c_str(), getpid());
	while(1);
}
