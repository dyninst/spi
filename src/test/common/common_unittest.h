#ifndef _COMMON_UNITTEST_H_
#define _COMMON_UNITTEST_H_

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "SpInc.h"

// ------------------------------------------------------------------- 
// Mock objects for SPI classes
// -------------------------------------------------------------------

// Use Mock object to skip real instrumentation
class MockEvent: public sp::SpEvent {
 public:
   MOCK_METHOD0(RegisterEvent, void());
};

// ------------------------------------------------------------------- 
// Utilities for testing
// -------------------------------------------------------------------
namespace sp {
  class SpAddrSpace;
};

class SpTestUtils {
  public:
  static bool LoadManyLibs(const char* libdir,
                           const int maxlibs);

  // Get 1st party address space
  static sp::SpAgent::ptr GetAgent();
};

#endif  // _COMMON_UNITTEST_H_
