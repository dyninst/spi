#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing instrumentation on gcc

namespace {


class GccTest : public testing::Test {
  public:
  GccTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(GccTest, preload) {
  string spi_env = "LD_PRELOAD=test_agent/count_test_agent.so ";

  string gcc_path;
  if (strcmp(getenv("PLATFORM"), "x86_64-unknown-linux2.4") == 0) {
    gcc_path = "LD_LIBRARY_PATH=/scratch/wenbin/software/lib64:/scratch/wenbin/software/lib:$LD_LIBRARY_PATH ";
    gcc_path += "/scratch/wenbin/software/libexec/gcc/x86_64-unknown-linux-gnu/4.5.2/cc1 /u/w/e/wenbin/devel/spi/spi/user_agent/count/x86_64-unknown-linux2.4/200.i";
  } else {
    gcc_path = "";
  }
  
  string cmd = spi_env + gcc_path;
  // sp_print("%s", cmd.c_str());
  system(cmd.c_str());
}

}
