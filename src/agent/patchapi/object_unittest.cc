#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpObject --
// mainly for allocating a close buffer near the object

namespace {


class ObjectTest : public testing::Test {
  public:
  ObjectTest() {
    // Load a bunch of shared libraries

    // Parse the all binrary objects
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};

// The simplest allocation and deallocation
TEST_F(ObjectTest, simple_alloc_free) {
}

// See what happens when we use up all free buffers
TEST_F(ObjectTest, extreme_test) {
}

}
