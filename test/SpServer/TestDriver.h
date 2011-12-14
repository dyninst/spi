#ifndef SP_TEST_DRIVER_H_
#define SP_TEST_DRIVER_H_

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <dlfcn.h>

#include <set>
#include <iostream>

namespace sp {
class TestDriver {
  public:
    TestDriver();

    void add_testcase(std::string name);
    bool run_testcase(std::string name);
    typedef std::set<std::string> TestCases;
    TestCases& testcases() { return testcases_; }
    void help();
  private:

    TestCases testcases_;
};
}

#endif /* SP_TEST_DRIVER_H_ */
