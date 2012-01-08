// Google test/mock headers
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Self-propelled stuffs
#include "SpIpcMgr.h"
#include "SpChannel.h"
#include "SpUtils.h"

using namespace sp;
using namespace std;

namespace {

  // -----------------------------------------------------------------------------
  // To test right before connect
  //   - TcpWorker::set_start_tracing
  //
  // Setting:
  // - Server: tcp_server
  // - Client: this executable (tcp_test)
  // -----------------------------------------------------------------------------
  class TcpConnectTest2 : public testing::Test {
  public:
    TcpConnectTest2() {}

  protected:
    pid_t server_pid_;
		pid_t client_pid_; 
    FILE* server_;
		FILE* client_;

    virtual void SetUp() {
			// Server
      server_ = popen("./tcp_server", "r");
      setbuf(server_, NULL);
      if (server_ == NULL) {
        sp_perror("Failed to start tcp_server");
      }
      PidSet server_pid_set;
      get_pids_from_fd(fileno(server_), server_pid_set);
      ASSERT_TRUE(server_pid_set.size() > 0);
      server_pid_ = *(server_pid_set.begin());
    }

    virtual void TearDown() {
      kill(server_pid_, SIGKILL);
      int status;
      wait(&status);
      pclose(server_);
			pclose(client_);
    }
  };

  TEST_F(TcpConnectTest2, set_start_tracing) {
		// Client
		client_ = popen("LD_PRELOAD=./ipc_test_agent.so ./tcp_client localhost", "r");
		setbuf(client_, NULL);
		if (client_ == NULL) {
			sp_perror("Failed to start tcp_client");
		}
		PidSet client_pid_set;
		get_pids_from_fd(fileno(client_), client_pid_set);
		ASSERT_TRUE(client_pid_set.size() > 0);
		client_pid_ = *(client_pid_set.begin());

		char buf[1024];
		while (fgets(buf, 1024, client_) != NULL) {
			sp_print(buf);
		}

		while (fgets(buf, 1024, server_) != NULL) {
			sp_print(buf);
		}

		/*
		EXPECT_TRUE(fgets(buf, 1024, client_) != NULL);
		EXPECT_TRUE(fgets(buf, 1024, client_) != NULL);
		EXPECT_STREQ(buf, "client: received \'Hello, world!\'\n");
		*/
  }
}
