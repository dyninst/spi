#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing instrumentation on c++ mutatees

namespace {


class CppTest : public testing::Test {
  public:
  CppTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(CppTest, call_names) {
  std::string cmd;
  cmd = "LD_LIBRARY_PATH=test_mutatee:$LD_LIBRARY_PATH ";
  cmd += "LD_PRELOAD=test_agent/print_test_agent.so ";
	cmd += "test_mutatee/cpp_prog.exe";
  system(cmd.c_str());
  /*
	FILE* fp = popen(cmd.c_str(), "r");
	char buf[1024];
	while (fgets(buf, 1024, fp) != NULL) {
    fprintf(stderr, "%s", buf);
  }
  pclose(fp);
  */
}

}
