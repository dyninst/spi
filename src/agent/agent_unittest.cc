#include "common/common_unittest.h"

// For testing SpAgent
// Mainly test setters and getters

namespace {


class AgentTest : public testing::Test {
  public:
  AgentTest() {
  }

  protected:

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};


TEST_F(AgentTest, default_setting) {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  ASSERT_TRUE(agent);
  agent->Go();

  EXPECT_TRUE(agent->parser());
  EXPECT_TRUE(agent->init_event());
  EXPECT_TRUE(agent->fini_event());
  EXPECT_TRUE(agent->init_propeller());
  EXPECT_STREQ(agent->init_entry().c_str(), "default_entry");
  EXPECT_TRUE(agent->init_exit().size() == 0);
  EXPECT_TRUE(agent->libraries_to_instrument().size() == 0);
  EXPECT_FALSE(agent->IsParseOnlyEnabled());
  EXPECT_FALSE(agent->IsDirectcallOnlyEnabled());
  EXPECT_FALSE(agent->IsTrapOnlyEnabled());
  EXPECT_FALSE(agent->IsIpcEnabled());
  EXPECT_FALSE(agent->IsMultithreadEnabled());
}

TEST_F(AgentTest, customized_setting) {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  ASSERT_TRUE(agent);

  sp::SpParser::ptr parser = sp::SpParser::Create();
  ASSERT_TRUE(parser);

  sp::SpEvent::ptr init_event = sp::AsyncEvent::Create();
  ASSERT_TRUE(init_event);

  sp::SpEvent::ptr fini_event = sp::SyncEvent::Create();
  ASSERT_TRUE(fini_event);

  sp::SpPropeller::ptr propeller = sp::SpPropeller::Create();
  ASSERT_TRUE(propeller);

  agent->SetParser(parser);
  agent->SetInitEvent(init_event);
  agent->SetFiniEvent(fini_event);
  agent->SetInitEntry("wrapper_entry");
  agent->SetInitExit("wrapper_exit");
  agent->SetInitPropeller(propeller);

  agent->EnableParseOnly(true);
  agent->EnableDirectcallOnly(true);
  agent->EnableTrapOnly(true);
  agent->EnableIpc(true);
  agent->EnableMultithread(true);

  sp::StringSet libs_to_inst;
  libs_to_inst.insert("libtest1.so");
  agent->SetLibrariesToInstrument(libs_to_inst);

  sp::StringSet funcs_not_to_inst;
  funcs_not_to_inst.insert("std::");
  funcs_not_to_inst.insert("gcc::");
  agent->SetFuncsNotToInstrument(funcs_not_to_inst);
  
  agent->Go();

  EXPECT_TRUE(agent->parser() == parser);
  EXPECT_TRUE(agent->init_event() == init_event);
  EXPECT_TRUE(agent->fini_event() == fini_event);
  EXPECT_STREQ(agent->init_entry().c_str(), "wrapper_entry");
  EXPECT_STREQ(agent->init_exit().c_str(), "wrapper_exit");
  EXPECT_TRUE(agent->init_propeller() == propeller);

  EXPECT_TRUE(agent->IsParseOnlyEnabled());
  EXPECT_TRUE(agent->IsDirectcallOnlyEnabled());
  EXPECT_TRUE(agent->IsTrapOnlyEnabled());
  EXPECT_TRUE(agent->IsIpcEnabled());
  EXPECT_FALSE(agent->IsMultithreadEnabled());

  EXPECT_TRUE(parser->CanInstrumentLib("libtest1.so"));
  EXPECT_FALSE(parser->CanInstrumentLib("libtest2.so"));
  EXPECT_FALSE(parser->CanInstrumentFunc("std::string"));
  EXPECT_FALSE(parser->CanInstrumentFunc("gcc::abc"));
  EXPECT_TRUE(parser->CanInstrumentFunc("strlen"));
}


}
