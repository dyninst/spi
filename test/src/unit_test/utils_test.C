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
	pid_t pid = fork();
	if (pid == 0) {
		system("./tcp_server");
	} else if (pid > 0) {
		sleep(10);
		PidSet pid_set;
		// system("./tcp_client localhost");
		addr_to_pids(0, 0, 0, 3490, pid_set);
		for (PidSet::iterator i = pid_set.begin(); i != pid_set.end(); i++) {
			sp_print("pid = %d", *i);
		}
		// system("/usr/sbin/lsof -i TCP");
		// addr_to_pids(0, 0, 0, 3490, pid_set);
		kill(pid, SIGKILL);
	}
}


TEST_F(UtilsTest, addr_to_pid_with_injector_system) {
	pid_t pid = fork();
	if (pid == 0) {
		system("./tcp_server");
	} else if (pid > 0) {
		sleep(10);
		string cmd = "ssh feta cd /afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/test/x86_64-unknown-linux2.4;./Injector 0.0.0.0 0 0.0.0.0 3490 ./ipc_test_agent.so";
		system(cmd.c_str());
		kill(pid, SIGKILL);
		int status;
		wait(&status);
	}
}
*/
TEST_F(UtilsTest, addr_to_pid_with_injector_pipe) {
	pid_t pid = fork();
	if (pid == 0) {
		system("./tcp_server");
	} else if (pid > 0) {
		sleep(10);
		string cmd = "ssh feta cd /afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/test/x86_64-unknown-linux2.4;./Injector 0.0.0.0 0 0.0.0.0 3490 ./ipc_test_agent.so";
		FILE* fp = popen(cmd.c_str(), "r");
		char line[1024];
		while (fgets(line, 1024, fp) != NULL) {
			fprintf(stderr, line);
		}
		pclose(fp);
		system("killall tcp_server");
		kill(pid, SIGKILL);
		int status;
		wait(&status);
	}
}
