#include "test/common/common_unittest.h"
#include "agent/context.h"

// For testing SpParser

void foo(int a) {
  cout << "i'm foo(int a)\n";
}

void foo() {
  cout << "i'm foo()\n";
}

int foo(int a, int b) {
  cout << "i'm foo(int a, int b)\n";
  return 1;
}

namespace {

class ParserTest : public testing::Test {
  public:
  ParserTest() {
    sp::SpAgent::ptr agent = sp::SpAgent::Create();
    agent->EnableParseOnly(true);
    agent->Go();
    parser_ = agent->parser();
  }

  protected:
    sp::SpParser::ptr parser_;

  virtual void SetUp() {
    ASSERT_TRUE(parser_);
  }

  virtual void TearDown() {
  }
};


TEST_F(ParserTest, find_func_by_mangled_name) {
  string name = "_Z3fooii";
  sp::SpFunction* found = parser_->FindFunction(name);
  ASSERT_TRUE(found);
  EXPECT_STREQ(found->name().c_str(), "foo");
  EXPECT_STREQ(found->GetMangledName().c_str(), name.c_str());
}

TEST_F(ParserTest, find_func_by_demangled_name) {
  string name = "foo";
  std::set<sp::SpFunction*> func_set;
  bool ret = parser_->FindFunction(name, &func_set);
  EXPECT_TRUE(ret);
  EXPECT_EQ((int)func_set.size(), 3);

  dt::Address previous_addr = 0;
  for (std::set<sp::SpFunction*>::iterator i = func_set.begin();
       i != func_set.end(); i++) {
    sp::SpFunction* found = *i;
    ASSERT_TRUE(found);
    EXPECT_STREQ(found->name().c_str(), "foo");
    EXPECT_TRUE(found->addr() > 0);
    EXPECT_NE(previous_addr, found->addr());
    previous_addr = found->addr();
  }
}

TEST_F(ParserTest, find_func_by_addr) {
  string name = "_Z3fooii";
  sp::SpFunction* found = parser_->FindFunction(name);
  ASSERT_TRUE(found);
  EXPECT_STREQ(found->name().c_str(), "foo");
  EXPECT_STREQ(found->GetMangledName().c_str(), name.c_str());
  
  dt::Address addr = found->addr();
  found = parser_->FindFunction(addr);
  ASSERT_TRUE(found);
  EXPECT_STREQ(found->name().c_str(), "foo");
  EXPECT_STREQ(found->GetMangledName().c_str(), name.c_str());
}

}
