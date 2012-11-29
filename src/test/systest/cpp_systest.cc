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

  void test_result(const char* filename, const char* expected) {
    bool inst = false;
    char line[1024];
    FILE* fp = fopen(filename, "r");
    while (fgets(line, 1024, fp)) {
      if (strstr(line, expected)) {
        inst = true;
        break;
      }
    }
    ASSERT_TRUE(inst);
    fclose(fp);
  }
};


TEST_F(CppTest, call_names) {
  std::string cmd;
  cmd = "LD_LIBRARY_PATH=test_mutatee:$LD_LIBRARY_PATH ";
  cmd += "LD_PRELOAD=test_agent/print_test_agent.so ";
	cmd += "test_mutatee/cpp_prog.exe > /tmp/cpptest.log";
  system(cmd.c_str());
  test_result("/tmp/cpptest.log", "ns::foo1");
  test_result("/tmp/cpptest.log", "_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc");
  test_result("/tmp/cpptest.log", "printf");
}

}
