#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpObject

namespace {


class ObjectTest : public testing::Test {
  public:
  ObjectTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(ObjectTest, pid_inject) {
}

}
