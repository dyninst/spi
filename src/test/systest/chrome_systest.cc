#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing instrumentation on google chrome

namespace {


class ChromeTest : public testing::Test {
  public:
  ChromeTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(ChromeTest, preload) {
  string path_env = "PATH=/scratch/chromium/gold/bin:/scratch/chromium/bin:/scratch/chromium/depot_tools:/afs/cs.wisc.edu/s/gcc-4.6.1/@sys/bin:/afs/cs.wisc.edu/s/gperf-3.0.2/@sys/bin:/afs/cs.wisc.edu/unsup/git-1.7.6/@sys/bin:/afs/cs.wisc.edu/s/python-2.7.1/@sys/bin:/u/w/e/wenbin/bin:/s/std/bin:/usr/afsws/bin:/opt/SUNWspro/bin:/usr/ccs/bin:/usr/ucb:/bin:/usr/bin:/usr/stat/bin:/usr/X11R6/bin ";

  string lib_env = "LD_LIBRARY_PATH=/scratch/wenbin/software/lib:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/dyninst/dep/lib:/afs/cs.wisc.edu/s/gcc-4.6.1/amd64_rhel5/lib64:/scratch/chromium/lib64:/scratch/chromium/lib:./:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/dyninst/x86_64-unknown-linux2.4/:/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/dyninst/x86_64-unknown-linux2.4/lib:/p/paradyn/packages/tcl8.4.5/lib:/p/paradyn/packages/libdwarf/lib:/p/paradyn/packages/libelf/bin:/p/paradyn/packages/libxml2/lib ";

  string spi_env = "LD_PRELOAD=test_agent/chrome_test_agent.so ";
  
  string chrome_path = "/scratch/chromium/src/out/Debug/chrome";

  string cmd = path_env + lib_env + spi_env + chrome_path;

  system(cmd.c_str());
}

}
