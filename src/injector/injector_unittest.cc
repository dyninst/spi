#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"
#include "test/common/common_unittest.h"

using namespace sp;
using namespace std;

// For testing Injector 

namespace {

// Alarm to exist unit test
void
alarm_handler(int signum) {
  FAIL() << "Failed due to timeout\n";
}

inline void
SetTimeout() {
  signal(SIGALRM, alarm_handler);
  alarm(360);
}

class InjectorTest : public testing::Test {
  public:
  InjectorTest(): pid_(0), server_(NULL) {
		injector_path_ = getenv("SP_DIR");
		injector_path_ += "/";
		injector_path_ += getenv("PLATFORM");
		injector_path_ += "/injector ";
	}

  protected:
	pid_t pid_;
	FILE* server_;
	string injector_path_;

  virtual void SetUp() {
    SetTimeout();
    
		// Start a tcp_server via popen
    char mutatee[1024];
    snprintf(mutatee, 1024,
             "LD_LIBRARY_PATH=%s/%s/test_mutatee:$LD_LIBRARY_PATH"
             " %s/%s/test_mutatee/tcp_server6.exe",
             getenv("SP_DIR"), getenv("PLATFORM"),
             getenv("SP_DIR"), getenv("PLATFORM"));

		server_ = popen(mutatee, "r");
		if (server_ == NULL) {
			sp_perror("Failed to start tcp_server");
		}

    // char buf[256];
    // ASSERT_TRUE(fgets(buf, 256, server_));
		// Get the tcp_server's pid
		PidSet pid_set;
		GetPidsFromFileDesc(fileno(server_), pid_set);
    // It is possible that server hasn't been up
		while (pid_set.size() <= 0) {
      GetPidsFromFileDesc(fileno(server_), pid_set);
    }
		pid_ = *(pid_set.begin());
	}

	virtual void TearDown() {
    system("killall tcp_server6.exe");
		int status;
		wait(&status);
		pclose(server_);
    sleep(2);
	}
};


TEST_F(InjectorTest, pid_inject) {
	char pid_str[255];
	sprintf(pid_str, "%d", pid_);

	string cmd = injector_path_;
	cmd += " ";
	cmd += pid_str;
  cmd += " ";
  cmd += getenv("SP_DIR");
  cmd += "/";
  cmd += getenv("PLATFORM");
  cmd += "/test_agent/inject_test_agent.so";

  // system(cmd.c_str());
	// Execute the injector
	FILE* fp = popen(cmd.c_str(), "r");
	char buf[1024];

  // Skip the first line
	ASSERT_TRUE(fgets(buf, 1024, fp) != NULL);
	ASSERT_TRUE(fgets(buf, 1024, fp) != NULL);  

	// Check "INJECTED" for the second line
	ASSERT_TRUE(strstr(buf, "INJECTED") != NULL);
	pclose(fp);
}

TEST_F(InjectorTest, pid_complex_agent_inject) {
	char pid_str[255];
	sprintf(pid_str, "%d", pid_);

	string cmd = injector_path_;
	cmd += " ";
	cmd += pid_str;
  cmd += " ";
  cmd += getenv("SP_DIR");
  cmd += "/";
  cmd += getenv("PLATFORM");
  cmd += "/test_agent/print_test_agent.so";

	// Execute the injector
	FILE* fp = popen(cmd.c_str(), "r");
	char buf[1024];

  // Skip the first line
	ASSERT_TRUE(fgets(buf, 1024, fp) != NULL);
	ASSERT_TRUE(fgets(buf, 1024, fp) != NULL);  

	// Check "INJECTED" for the second line
	ASSERT_TRUE(strstr(buf, "INJECTED") != NULL);
	pclose(fp);
}

TEST_F(InjectorTest, network_inject) {
	string cmd = injector_path_;
	cmd += " ";
	cmd += "127.0.0.1 ";
	cmd += "3490 ";        // The only thing we use for injection
	cmd += " ";
  cmd += getenv("SP_DIR");
  cmd += "/";
  cmd += getenv("PLATFORM");
  cmd += "/test_agent/inject_test_agent.so";

  // system(cmd.c_str());

	// Execute the injector
	FILE* fp = popen(cmd.c_str(), "r");
	char buf[1024];

  // Skip the first line
	ASSERT_TRUE(fgets(buf, 1024, fp) != NULL);
	ASSERT_TRUE(fgets(buf, 1024, fp) != NULL);  

	// Check "INJECTED" for the second line
	ASSERT_TRUE(strstr(buf, "INJECTED") != NULL);
	pclose(fp);
}

TEST_F(InjectorTest, dup_inject) {
	char pid_str[255];
	sprintf(pid_str, "%d", pid_);

	string cmd = injector_path_;
	cmd += " ";
	cmd += pid_str;
	cmd += " ";
  cmd += getenv("SP_DIR");
  cmd += "/";
  cmd += getenv("PLATFORM");
  cmd += "/test_agent/inject_test_agent.so";

	// Execute the injector
	FILE* fp = popen(cmd.c_str(), "r");
	char buf[1024];

  // Skip the first line
	ASSERT_TRUE(fgets(buf, 1024, fp) != NULL);
	ASSERT_TRUE(fgets(buf, 1024, fp) != NULL);  

	// Check "INJECTED" for the second line
	ASSERT_TRUE(strstr(buf, "INJECTED") != NULL);
  
	pclose(fp);

	// Execute the injector
	fp = popen(cmd.c_str(), "r");
	pclose(fp);
}

}
