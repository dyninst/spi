#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpInstrumenter

namespace {


class InstrumenterTest : public testing::Test {
  public:
  InstrumenterTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(InstrumenterTest, pid_inject) {
}

}
