#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "PatchMgr.h"
#include "test/common/common_unittest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpAddrSpace
// Mainly for testing memory allocation stuffs
// 1. In constructor, load a 100 shared libraries from /lib
// 2. In constructor, parse all binary objects in address space

namespace {

class AddrSpaceTest : public testing::Test {
  public:
  AddrSpaceTest() {
    // Load 100 shared libraries from /lib
    bool ret = SpTestUtils::LoadManyLibs("/lib", 100);
    if (!ret) {
      sp_perror("Failed to load libraries from /lib");
    }

    // Parse binary objects
    agent_ = SpTestUtils::GetAgent();
    if (!agent_) {
      sp_perror("Failed to get sp::agent");
    }
    as_ = static_cast<sp::SpAddrSpace*>(agent_->parser()->mgr()->as());
	}
  
  ~AddrSpaceTest() {
  }
  
  protected:
  sp::SpAgent::ptr agent_;
  sp::SpAddrSpace* as_;
  
  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};

// Allocate small, medium, large, and very large buffers.
TEST_F(AddrSpaceTest, simple_malloc_free) {
  SpObject* exe = agent_->parser()->exe();
  dt::Address buf = as_->malloc(exe, 100, 0);
  EXPECT_TRUE(IsDisp32(buf - exe->load_addr()));
  sp_debug("SMALL BUF - %lx", buf);

  buf = as_->malloc(exe, 500, 0);
  EXPECT_TRUE(IsDisp32(buf - exe->load_addr()));
  sp_debug("MID BUF - %lx", buf);

  buf = as_->malloc(exe, 4000, 0);
  EXPECT_TRUE(IsDisp32(buf - exe->load_addr()));
  sp_debug("LARGE BUF - %lx", buf);

  // Should use mmap to allocate
  buf = as_->malloc(exe, 40000, 0);
  EXPECT_TRUE(buf != 0);
  sp_debug("MMAPED - %lx", buf);
}

}
