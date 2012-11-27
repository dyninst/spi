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
    TcpConnectTest2() {
      mutatee_prefix_ = " LD_LIBRARY_PATH=test_mutatee::tmp/lib:$LD_LIBRARY_PATH ";
      mutatee_prefix_ += " SP_AGENT_DIR=$SP_DIR/$PLATFORM/test_agent ";
      preload_prefix_ = " LD_PRELOAD=$SP_DIR/$PLATFORM/test_agent/ipc_test_agent.so ";
    }

  protected:
    pid_t server_pid_;
		pid_t client_pid_; 
    FILE* server_;
		FILE* client_;
    string mutatee_prefix_;
    string preload_prefix_;

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    //
    // Simple IPC test
    //

    void run_server1() {
			// Server
      string cmd = mutatee_prefix_ + "test_mutatee/tcp_server4.exe";
      server_ = popen(cmd.c_str(), "r");
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

    void shutdown_server1() {
      system("killall tcp_server4.exe");
      // sleep(2);
      // kill(server_pid_, SIGKILL);
      int status;
      wait(&status);
      pclose(server_);
    }
    
		void run_client1() {
			// Client
      string cmd = mutatee_prefix_ + preload_prefix_ +
          "test_mutatee/tcp_client.exe localhost";
      // system(cmd.c_str());

			client_ = popen(cmd.c_str(), "r");
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

		void run_client_no_inst1() {
			// Client
      string cmd = mutatee_prefix_ +
          "test_mutatee/tcp_client.exe localhost";
			client_ = popen(cmd.c_str(), "r");
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

    //
    // Complex IPC test
    //
    void run_server2() {
			// Server
      string cmd = mutatee_prefix_ + "test_mutatee/lighttpd.exe -f tmp/conf -D"
          " >& /tmp/serverlighttpd.log";
      server_ = popen(cmd.c_str(), "r");
      setbuf(server_, NULL);
      if (server_ == NULL) {
        sp_perror("Failed to start lighttpd");
      }
			sleep(5);
      PidSet server_pid_set;
      char ip[256];
      const char* port = "8001";
      GetPidsFromAddrs(ip, port, server_pid_set);
      ASSERT_TRUE(server_pid_set.size() > 0);
      server_pid_ = *(server_pid_set.begin());
    }

		void run_client2() {
			// Client
      string cmd = mutatee_prefix_ + preload_prefix_ +
          "test_mutatee/wget.exe localhost:8001/tmp.txt >& /tmp/clientwget.log";
      // system(cmd.c_str());

			client_ = popen(cmd.c_str(), "r");
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

		void run_client_no_inst2() {
			// Client
      string cmd = mutatee_prefix_ + 
          "test_mutatee/wget.exe localhost:8001/tmp.txt >& /tmp/clientnowget.log";
      // system(cmd.c_str());

			client_ = popen(cmd.c_str(), "r");
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
    
    void shutdown_server2() {
      system("killall lighttpd.exe");
      // sleep(2);
      // kill(server_pid_, SIGKILL);
      int status;
      wait(&status);
      pclose(server_);
    }

    void test_result(const char* filename) {
      bool inst = false;
      char line[1024];
      FILE* fp = fopen(filename, "r");
      while (fgets(line, 1024, fp)) {
        if (strstr(line, "Read size: 12 @ pid=")) {
          inst = true;
          break;
        }
        if (strstr(line, "Write size: 12 @ pid=")) {
          inst = true;
          break;
        }
      }
      ASSERT_TRUE(inst);
      fclose(fp);
    }
  };


/*
  TEST_F(TcpConnectTest2, simple_ipc) {
    run_server1();
		run_client1();
		run_client_no_inst1();
		run_client1();
    shutdown_server1();
  }
*/

  TEST_F(TcpConnectTest2, complex_ipc) {
    system("rm -f *.log");
    run_server2();
		run_client2();
		run_client_no_inst2();
    shutdown_server2();
    system("rm -f tmp.txt*");
    test_result("/tmp/serverlighttpd.log");
    test_result("/tmp/clientnowget.log");
    test_result("/tmp/clientwget.log");
  }
}
