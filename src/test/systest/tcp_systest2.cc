// Google test/mock headers
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Self-propelled stuffs
#include "SpInc.h"

using namespace sp;
using namespace std;

namespace {

  // -----------------------------------------------------------------------------
  // To test right before connect (localhost to localhost)
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
      server_ = popen("./tcp_server4", "r");
      setbuf(server_, NULL);
      if (server_ == NULL) {
        sp_perror("Failed to start tcp_server");
      }
			sleep(3);

      PidSet server_pid_set;
      GetPidsFromFileDesc(fileno(server_), server_pid_set);
      ASSERT_TRUE(server_pid_set.size() > 0);
      server_pid_ = *(server_pid_set.begin());
    }

    virtual void TearDown() {
      kill(server_pid_, SIGKILL);
      int status;
      wait(&status);
      pclose(server_);
    }

		void run_client() {
			// Client
			client_ = popen("LD_PRELOAD=./ipc_test_agent.so ./tcp_client localhost", "r");
			setbuf(client_, NULL);
			if (client_ == NULL) {
				sp_perror("Failed to start tcp_client");
			}


			// Two possibilities:
			// 1. Client finishes too soon, before we get pid
      //    -- need to test pid_set.size() > 0
			// 2. Client hasn't started to connect
      //    -- need to wait()
			PidSet client_pid_set;
			GetPidsFromFileDesc(fileno(client_), client_pid_set);
			if (client_pid_set.size() > 0) {
				client_pid_ = *(client_pid_set.begin());
				int status;
				wait(&status);
			}
			pclose(client_);
		}

		void run_client_no_inst() {
			// Client
			client_ = popen("./tcp_client localhost", "r");
			setbuf(client_, NULL);
			if (client_ == NULL) {
				sp_perror("Failed to start tcp_client");
			}

			// Two possibilities:
			// 1. Client finishes too soon, before we get pid
      //    -- need to test pid_set.size() > 0
			// 2. Client hasn't started to connect
      //    -- need to wait()
			PidSet client_pid_set;
			GetPidsFromFileDesc(fileno(client_), client_pid_set);
			if (client_pid_set.size() > 0) {
				client_pid_ = *(client_pid_set.begin());
				int status;
				wait(&status);
			}
			pclose(client_);
		}

  };

  TEST_F(TcpConnectTest2, set_start_tracing) {
		run_client();
		//run_client_no_inst();
		// run_client();
  }
}
