#ifndef _SPUTILS_H_
#define _SPUTILS_H_

#include <sys/time.h>
#include "SpCommon.h"

namespace sp {
  void SetupTimer(int);
  void StartTimer(int);
  void StopTimer(int);
  void ResetTimer(int);
  double GetTimer(int);
  void PrintTime(char *, int);


  void instrumenter_start();
  void instrumenter_end();

  void install_start();
  void install_end();

  void propeller_start();
  void propeller_end();

  void next_point_start();
  void next_point_end();

  void callee_start();
  void callee_end();

  void findfunc_start();
  void findfunc_end();

  void payload_start();
  void payload_end();

  void report_timer();
  inline bool is_disp32(long d) {
    const long max_int32 = 2147483647;
    const long min_int32 = (-2147483647-1);
    return ((d <= max_int32) && (d >= min_int32));
  }
  inline bool is_disp8(long d) {
    const long max_int8 = 128;
    const long min_int8 = -128;
    return ((d <= max_int8) && (d >= min_int8));
  }
}

#endif /* _SPUTILS_H_ */
