#ifndef _SPUTILS_H_
#define _SPUTILS_H_

#include <sys/time.h>

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
}

#endif /* _SPUTILS_H_ */
