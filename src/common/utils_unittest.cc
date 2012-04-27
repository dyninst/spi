#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

namespace {

class UtilsTest : public testing::Test {
  public:
  UtilsTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
    
	}
};

TEST_F(UtilsTest, GetPidsFromAddrs) {
  char mutatee[1024];
  snprintf(mutatee, 1024,
           "LD_LIBRARY_PATH=%s/%s/test_mutatee:$LD_LIBRARY_PATH"
           " %s/%s/test_mutatee/tcp_server6.exe",
           getenv("SP_DIR"), getenv("PLATFORM"),
           getenv("SP_DIR"), getenv("PLATFORM"));

  FILE* server_ = popen(mutatee, "r");
  if (server_ == NULL) {
    sp_perror("Failed to start tcp_server");
  }
  
  PidSet pid_set;
  GetPidsFromAddrs("0.0.0.0", "0", "0.0.0.0", "3490", pid_set);
  EXPECT_TRUE(pid_set.size() == 1);
  
  system("killall tcp_server6.exe");
  int status;
  wait(&status);
  pclose(server_);
  sleep(2);
}

}
