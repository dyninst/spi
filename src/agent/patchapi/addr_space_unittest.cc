#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpAddrSpace

namespace {


class AddrSpaceTest : public testing::Test {
  public:
  AddrSpaceTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(AddrSpaceTest, pid_inject) {
}

}
