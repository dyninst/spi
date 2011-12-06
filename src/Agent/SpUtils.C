#include "SpUtils.h"

namespace sp {

/* Profiling tools */
typedef long i64;
typedef struct CPerfCounterRec {
  i64 _freq;
  i64 _clocks;
  i64 _start;
} CPerfCounter;

void SetupTimer(int);
void StartTimer(int);
void StopTimer(int);
void ResetTimer(int);
double GetTimer(int);
void PrintTime(char *, int);

CPerfCounter a[10];

static long instrumenter_timer = 0;
static long install_timer = 0;
static long propeller_timer = 0;
static long next_point_timer = 0;
static long callee_timer = 0;
static long findfunc_timer = 0;
static long payload_timer = 0;

void instrumenter_start() {
  SetupTimer(0);
  StartTimer(0);
}

void instrumenter_end() {
  StopTimer(0);
  instrumenter_timer += a[0]._clocks;
}

void install_start() {
  SetupTimer(1);
  StartTimer(1);
}

void install_end() {
  StopTimer(1);
  install_timer += a[1]._clocks;
}

void propeller_start() {
  SetupTimer(2);
  StartTimer(2);
}

void propeller_end() {
  StopTimer(2);
  propeller_timer += a[2]._clocks;
}

void next_point_start() {
  SetupTimer(3);
  StartTimer(3);
}

void next_point_end() {
  StopTimer(3);
  next_point_timer += a[3]._clocks;
}

void callee_start() {
  SetupTimer(4);
  StartTimer(4);
}

void callee_end() {
  StopTimer(4);
  callee_timer += a[4]._clocks;
}

void findfunc_start() {
  SetupTimer(5);
  StartTimer(5);
}

void findfunc_end() {
  StopTimer(5);
  findfunc_timer += a[5]._clocks;
}

void payload_start() {
  SetupTimer(6);
  StartTimer(6);
}

void payload_end() {
  StopTimer(6);
  payload_timer += a[6]._clocks;
}

void  report_timer() {
  sp_print("=== dump timer ===");
  //  sp_print("payload - %ld", payload_timer);
  sp_print("  Propeller::go - %ld", propeller_timer);
  //  sp_print("    instrumenter::go - %ld", instrumenter_timer);
  //  sp_print("      instrumenter::install - %ld", install_timer);
  //  sp_print("    propeller::next_points - %ld", next_point_timer);
  //  sp_print("    parser::find_func - %ld", findfunc_timer);
  //  sp_print("    parser::callee - %ld", callee_timer);
}

void SetupTimer(int i) {
  a[i]._clocks = 0;
  a[i]._start = 0;
  a[i]._freq = 1000;
  ResetTimer(i);
}

void StartTimer(int i) {
  struct timeval s;
  gettimeofday(&s, 0);
  a[i]._start = (i64)s.tv_sec * 1000*1000 + (i64)s.tv_usec;
}

void StopTimer(int i) {
  i64 n = 0;
  struct timeval s;
  gettimeofday(&s, 0);
  n = (i64)s.tv_sec * 1000 *1000 + (i64)s.tv_usec;
  n -= a[i]._start;
  a[i]._start = 0;
  a[i]._clocks += n;
}

void ResetTimer(int i) {
  a[i]._clocks = 0;
}

double GetTimer(int i) {
  return (double)a[i]._clocks;
}

void PrintTime(char *msg, int i) {
  if (msg != NULL)
    printf("%s: %f sec\n", msg, GetTimer(i));
  else
    printf("%f sec\n", GetTimer(i));
}

}
