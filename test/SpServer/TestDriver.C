/* Test Driver for testing SpServer in self-propelled instrumentation
   - Each test case tests one IPC mechanism
 */

#include "TestDriver.h"
#include "string.h"

using sp::TestDriver;

char can_exit = 0;

TestDriver::TestDriver() {
  // Add test cases here:

  /* pipe1: preload agent.so before fork() */
  add_testcase("pipe1");

  /* pipe2: preload agent.so after fork() */
  add_testcase("pipe2");

  /* pipe3: popen */
  add_testcase("pipe3");

  /* pipe4: popen */
  add_testcase("pipe4");

  /* pipe4: popen */
  add_testcase("pipe5");

  add_testcase("tcp");
  add_testcase("udp");
}

void TestDriver::add_testcase(std::string name) {
  testcases_.insert(name);
}

bool TestDriver::run_testcase(std::string name) {
  fprintf(stderr, "Run test case for %s\n", name.c_str());
  // TODO
  // 1. Start server
  // 2. Run test case
  char cmd[1024];
  if (name.compare("pipe2") == 0 &&
      name.compare("pipe4") == 0) {
    sprintf(cmd, "%s", name.c_str());
  }
  else if (name.compare("pipe5") == 0) {
    sprintf(cmd, "LD_PRELOAD=./TestAgent.so %sclient&", name.c_str());
    system(cmd);
    sprintf(cmd, "LD_PRELOAD=./TestAgent.so %s", name.c_str());
  }
  else {
    sprintf(cmd, "LD_PRELOAD=./TestAgent.so %s", name.c_str());
  }
  std::cerr << cmd << "\n";
  system(cmd);

  // 3. Stop server
  return true;
}

void TestDriver::help() {
  std::cout << "usage: test IPC_MECHANISM\n";
  std::cout << "\tIPC_MECHANISM should be one of the following items:\n";
  for (TestCases::iterator i = testcases_.begin(); i != testcases_.end(); i++) {
    std::cout << "\t-" << *i << "\n";
  }
}

int main(int argc, char *argv[]) {
  TestDriver driver;
  if (argc < 2) {
    driver.help();
    return 1;
  }
  char* test_name = &argv[1][1];
  if (driver.testcases().find(test_name) == driver.testcases().end()) {
    driver.help();
    return 1;
  }
  int pid = fork();
  switch (pid) {
    // Child
    case 0: {
      driver.run_testcase(test_name);
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
