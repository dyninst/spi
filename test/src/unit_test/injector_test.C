// Google test/mock headers
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

namespace {

// ----------------------------------------------------------------------------- 
// For testing Injector 
// 1. pid_inject: inject the known process
// 2. network_inject: inject a ip/port pair
// -----------------------------------------------------------------------------
class InjectorTest : public testing::Test {
  public:
  InjectorTest(): pid_(0), server_(NULL) {
		injector_path_ = getenv("SP_DIR");
		injector_path_ += "/";
		injector_path_ += getenv("PLATFORM");
		injector_path_ += "/Injector ";
	}

  protected:
	pid_t pid_;
	FILE* server_;
	string injector_path_;

  virtual void SetUp() {
		// Start a tcp_server via popen
		server_ = popen("./tcp_server6", "r");
		if (server_ == NULL) {
			sp_perror("Failed to start tcp_server");
		}

		// Get the tcp_server's pid
		PidSet pid_set;
		GetPidsFromFileDesc(fileno(server_), pid_set);
		pid_ = *(pid_set.begin());
		ASSERT_TRUE(pid_set.size() > 0);
	}

	virtual void TearDown() {
		kill(pid_, SIGKILL);
		int status;
		wait(&status);
		pclose(server_);
	}
};


TEST_F(InjectorTest, pid_inject) {
	char pid_str[255];
	sprintf(pid_str, "%d", pid_);

	string cmd = injector_path_;
	cmd += " ";
	cmd += pid_str;
	cmd += " ./inject_test_agent.so";

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
	cmd += " ./comp_test_agent.so";

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
	cmd += "80 ";          // Fake port number
	cmd += "127.0.0.1 ";
	cmd += "3490 ";        // The only thing we use for injection
	cmd += "./inject_test_agent.so";

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
	cmd += " ./inject_test_agent.so";

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
