#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpPayload

namespace {


class PayloadTest : public testing::Test {
  public:
  PayloadTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(PayloadTest, pid_inject) {
}

}
