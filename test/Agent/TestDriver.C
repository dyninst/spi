/* Test Driver for testing Agent in self-propelled instrumentation
   Two pieces of components should be provided for each test case:
   1. *_mutatee.c, which is a shared library for mutatee,
      providing a mutatee() method.
   2. *_agent.C, which is a shared library for agent,
      providing an init() method that will be called when this library is loaded
 */

#include "TestDriver.h"

using sp::TestDriver;

char can_exit = 0;

TestDriver::TestDriver() {
  // Add test cases here:
  add_testcase("parser");
  add_testcase("event");
  add_testcase("propel");
  add_testcase("indcall");
}

void TestDriver::add_testcase(std::string name) {
  testcases_.insert(name);
}

bool TestDriver::run_preloaded_testcase(std::string name) {
  // 1. Check if this test case is in testcases_
  if (testcases_.find(name) == testcases_.end()) {
    std::cerr << "ERROR: there's not a test case called " << name << "\n";
    exit(-1);
  }

  // 2. Make name_mutatee and name_agent.so
  std::string mutatee = name + "_mutatee";
  std::string agent = name + "_agent.so";

  // 3. Exec
  char cmd[1024];
  sprintf(cmd, "LD_PRELOAD=./%s %s", agent.c_str(), mutatee.c_str());
  system(cmd);

  return true;
}

bool TestDriver::run_injected_testcase(std::string name) {
  // 1. Check if this test case is in testcases_
  if (testcases_.find(name) == testcases_.end()) {
    std::cerr << "ERROR: there's not a test case called " << name << "\n";
    exit(-1);
  }

  // 2. Make name_mutatee and name_agent.so
  std::string mutatee = name + "_mutatee";
  std::string agent = name + "_agent.so";

  // 3. Exec
  char cmd[1024];
  sprintf(cmd, "sh run.sh %s %s", agent.c_str(), mutatee.c_str());
  return (system(cmd) != -1);
}

void TestDriver::help() {
  std::cout << "usage: test TEST_CASES LOAD_METHOD\n";
  std::cout << "\tTEST_CASES should be one of the following items:\n";
  for (TestCases::iterator i = testcases_.begin(); i != testcases_.end(); i++) {
    std::cout << "\t-" << *i << "\n";
  }
  std::cout << "\tLOAD_METHOD should be one of the following items:\n";
  std::cout << "\t-" << "i  (Inject)" << "\n";
  std::cout << "\t-" << "l  (Preload)" << "\n";
}

int main(int argc, char *argv[]) {
  TestDriver driver;
  if (argc < 3) {
    driver.help();
    return 1;
  }

  char* test_name = &argv[1][1];
  bool injected = false;
  if (strncmp(&argv[2][1], "i", 1) == 0) {
    injected = true;
  }

  int pid = fork();
  switch (pid) {
    // Child
    case 0: {
      // driver.run_testcase(test_name);
      if (injected) {
        driver.run_injected_testcase(test_name);
      } else {
        driver.run_preloaded_testcase(test_name);
      }
      break;
    }
    case -1: {
      std::cerr << "ERROR: failed to fork a process to execute test case.\n";
      break;
    }
    default: {
      int child_status;
      pid_t wpid = wait(&child_status);
      std::cout << test_name << " returns " << child_status << " -- ";
      if (child_status == 0 && WIFEXITED(child_status))
        std::cout << " PASSED\n";
      else
        std::cout << " FAILED\n";
      break;
    }
  }
  return 0;
}
