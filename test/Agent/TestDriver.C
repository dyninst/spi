/* Test Driver for testing Agent in self-propelled instrumentation
   Two pieces of components should be provided for each test case:
   1. *_mutatee.c, which is a shared library for mutatee,
      providing a mutatee() method.
   2. *_agent.C, which is a shared library for agent,
      providing an init() method that will be called when this library is loaded
 */

#include "TestDriver.h"

using sp::TestDriver;

TestDriver::TestDriver() {
  struct rlimit core_limit;
  core_limit.rlim_cur = RLIM_INFINITY;
  core_limit.rlim_max = RLIM_INFINITY;
  if (setrlimit(RLIMIT_CORE, &core_limit) < 0) {
    std::cerr << "ERROR: failed to setup core dump ability\n";
    exit(0);
  }

  // Add test cases here:
  add_testcase("parser");
  add_testcase("event");
}

void TestDriver::add_testcase(std::string name) {
  testcases_.insert(name);
}

bool TestDriver::run_testcase(std::string name) {
  // 1. Check if this test case is in testcases_
  // name = name.substr(1);
  if (testcases_.find(name) == testcases_.end()) {
    std::cerr << "ERROR: there's not a test case called " << name << "\n";
    exit(0);
  }

  // 2. Load name_mutatee.so and name_agent.so
  std::string mutatee = name + "_mutatee.so";
  void* m_handle = dlopen(mutatee.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if (!m_handle) {
    std::cerr << "ERROR: cannot load " << mutatee << "\n";
    std::cerr << dlerror() << "\n";
    exit(0);
  }
  std::string agent = name + "_agent.so";
  void* a_handle = dlopen(agent.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if (!a_handle) {
    std::cerr << "ERROR: cannot load " << agent << "\n";
    std::cerr << dlerror() << "\n";
    exit(0);
  }

  // 3. Run mutatee()
  typedef void (*run_mutatee_t)();
  run_mutatee_t run = (run_mutatee_t)dlsym(m_handle, "run_mutatee");
  run();

  // 4. Unload name_mutatee.so and name_agent.so
  dlclose(m_handle);
  dlclose(a_handle);

  return true;
}

void TestDriver::help() {
  std::cout << "usage: test TEST_CASES\n";
  std::cout << "\tTEST_CASES should be one of the following items:\n";
  for (TestCases::iterator i = testcases_.begin(); i != testcases_.end(); i++) {
    std::cout << "\t-" << *i << "\n";
  }
}

int main(int argc, char *argv[]) {
  TestDriver driver;
  if (argc != 2) {
    driver.help();
    return 1;
  }
  char* test_name = &argv[1][1];
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
      std::cout << test_name;
      if (WIFEXITED(child_status))
	std::cout << " PASSED\n";
      else
	std::cout << " FAILED\n";
      break;
    }
  }
  return 0;
}
