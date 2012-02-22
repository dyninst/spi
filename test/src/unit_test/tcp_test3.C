// Google test/mock headers
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Headers for TCP Client/Server
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

// Self-propelled stuffs
#include "SpInc.h"

using namespace sp;
using namespace std;

namespace {
  // -----------------------------------------------------------------------------
  // To test right before connect
  // -----------------------------------------------------------------------------
  class TcpTest3 : public testing::Test {
  public:
    TcpTest3() {}

  protected:
    pid_t server_pid_;

    // Start a server on remote machine, then fork a client on local machine
    virtual void SetUp() {
			server_pid_ = fork();
			if (server_pid_ == 0) {
				system("ssh wasabi sh /afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/test/x86_64-unknown-linux2.4/tcp_server.sh");
				// system("ssh wasabi /afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/test/x86_64-unknown-linux2.4/tcp_server4");
			}
    }

    // Kill the server
    virtual void TearDown() {
			if (server_pid_ > 0) {
				system("ssh wasabi killall tcp_server4");
				kill(server_pid_, SIGKILL);
				int status;
				wait(&status);
			}
    }
  };

  TEST_F(TcpTest3, inst_server_not_inst_client) {
		if (server_pid_ > 0) {
			sleep(10);
			// Trigger server to output things
      FILE* fp = popen("./tcp_client wasabi", "r");
			char buf[1024];
			while (fgets(buf, 1024, fp) != NULL) {
				fprintf(stderr, "From client: %s\n", buf);
			}
			pclose(fp);
		}
  }
}
