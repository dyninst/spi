#ifndef _COMMON_UNITTEST_H_
#define _COMMON_UNITTEST_H_

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "SpInc.h"

// Use Mock object to skip real instrumentation
class MockEvent: public sp::SpEvent {
 public:
   MOCK_METHOD0(RegisterEvent, void());
};

// Alarm to exist unit test
void
alarm_handler(int signum) {
  FAIL() << "Failed due to timeout\n";
}

inline void
SetTimeout() {
  signal(SIGALRM, alarm_handler);
  alarm(360);
}

#endif  // _COMMON_UNITTEST_H_
