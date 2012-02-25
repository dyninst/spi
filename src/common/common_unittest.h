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

#endif  // _COMMON_UNITTEST_H_
