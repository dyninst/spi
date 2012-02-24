#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing SpSnippet

namespace {


class SnippetTest : public testing::Test {
  public:
  SnippetTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(SnippetTest, pid_inject) {
}

}
