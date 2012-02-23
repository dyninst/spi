#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "SpInc.h"

using namespace sp;
using namespace std;

// For testing Injector 
// 1. pid_inject: inject the known process
// 2. network_inject: inject a ip/port pair

namespace {


class InjectorTest : public testing::Test {
  public:
  InjectorTest() {
	}

  protected:

  virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};


TEST_F(InjectorTest, pid_inject) {
}

TEST_F(InjectorTest, pid_complex_agent_inject) {
}

TEST_F(InjectorTest, network_inject) {
}

TEST_F(InjectorTest, dup_inject) {
}

}
