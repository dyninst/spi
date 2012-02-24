#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpUdpWorker

namespace {


class UdpWorkerTest : public testing::Test {
  public:
  UdpWorkerTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(UdpWorkerTest, pid_inject) {
}

}
