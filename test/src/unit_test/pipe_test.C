// Google test/mock headers
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Self-propelled stuffs
#include "SpInc.h"

using namespace sp;
using namespace std;

namespace {
  class PipeTest : public testing::Test {
	protected:
    virtual void SetUp() {
		}
    virtual void TearDown() {
		}
	};

  TEST_F(PipeTest, preload_agent_before_fork) {
		system("LD_PRELOAD=./ipc_test_agent.so ./pipe1");
		/*
    FILE* parent = popen("LD_PRELOAD=./ipc_test_agent.so ./pipe1", "r");
		char line[1024];
		while (fgets(line, 1024, parent) != NULL) {
			sp_print(line);
		}
		pclose(parent);
		*/
	}

  TEST_F(PipeTest, preload_agent_after_fork) {
		system("./pipe2");
	}

  TEST_F(PipeTest, preload_agent_before_popen) {
		system("LD_PRELOAD=./ipc_test_agent.so ./pipe3");
	}

  TEST_F(PipeTest, preload_agent_after_popen) {
		system("./pipe4");
	}

  TEST_F(PipeTest, fifo) {
		pid_t pid = fork();
		if (pid == 0) {
			system("LD_PRELOAD=./ipc_test_agent.so  pipe5");
		}

		else if (pid > 0) {
		  system("./pipe5server");
		}
		
	}

}
