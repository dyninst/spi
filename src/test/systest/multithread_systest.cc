#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing instrumentation on multithreaded programs
// Multithread, single process

namespace {


class MultithreadTest : public testing::Test {
  public:
  MultithreadTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(MultithreadTest, pid_inject) {
}

}
