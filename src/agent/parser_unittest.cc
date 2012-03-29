#include "test/common/common_unittest.h"
#include "agent/context.h"

// For testing SpParser
// We load up to 100 shared libraries from /lib

namespace {


class ParserTest : public testing::Test {
  public:
  ParserTest() {
  }

  protected:

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

TEST_F(ParserTest, update_memory_mappings) {
}

TEST_F(ParserTest, update_free_intervals) {
}

TEST_F(ParserTest, get_closest_interval) {
}

TEST_F(ParserTest, parse_objects) {
}

}
