#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpTcpWorker

namespace {


class TcpWorkerTest : public testing::Test {
  public:
  TcpWorkerTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(TcpWorkerTest, pid_inject) {
}

}
