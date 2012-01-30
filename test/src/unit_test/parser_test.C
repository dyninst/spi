// Google test/mock headers
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Self-propelled stuffs
#include "SpIpcMgr.h"
#include "SpChannel.h"
#include "SpUtils.h"

using namespace sp;
using namespace std;

// Test Parser
namespace {

  class ParserTest : public testing::Test {
	protected:
    virtual void SetUp() {
		}
    virtual void TearDown() {
		}
	};
	/*
  TEST_F(ParserTest, parse_non_stripped) {
		std::string cmd = "LD_PRELOAD=./parser_test_agent.so ./indcall";
		// system(cmd.c_str());
		FILE* fp = popen(cmd.c_str(), "r");
		char buf[1024];
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "OKAY\n");
		pclose(fp);
	}

  TEST_F(ParserTest, func_to_object) {
		std::string cmd = "LD_PRELOAD=./parser_func_to_obj_agent.so ./indcall";
		// system(cmd.c_str());
		FILE* fp = popen(cmd.c_str(), "r");
		char buf[1024];
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "indcall\n");
		pclose(fp);
	}


  TEST_F(ParserTest, get_agent_name) {
		std::string cmd = "LD_PRELOAD=./parser_agent_name.so ./indcall";
		// system(cmd.c_str());
		FILE* fp = popen(cmd.c_str(), "r");
		char buf[1024];
		EXPECT_TRUE(fgets(buf, 1024, fp) != NULL);
		EXPECT_STREQ(buf, "parser_agent_name.so\n");
		pclose(fp);
	}
	*/

  TEST_F(ParserTest, parser_stripped) {
		std::string cmd = "LD_PRELOAD=./parser_test_agent.so ./stripped";
		system(cmd.c_str());
	}

}
