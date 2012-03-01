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
  gcc_path = "LD_LIBRARY_PATH=tmp/lib:$LD_LIBRARY_PATH ";
  gcc_path += "test_mutatee/cc1.exe test_data/200.i";
  
  string cmd = spi_env + gcc_path;
  system(cmd.c_str());
}

}
