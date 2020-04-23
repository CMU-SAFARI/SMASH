#ifndef _TIMERS_H_
#define _TIMERS_H_

#include <stdlib.h>
#include <sys/time.h>

typedef struct timer_s {
  struct timeval t1;
  struct timeval t2;
  double duration;
} timer_tt;

static inline timer_tt *timer_alloc() {
  timer_tt *timer;
  timer = (timer_tt *)malloc(sizeof(timer_tt));
  timer->duration = 0;
  return timer;
}

static inline void timer_start(timer_tt *timer) { gettimeofday(&timer->t1, 0); }

static inline void timer_stop(timer_tt *timer) {
  gettimeofday(&timer->t2, 0);
  timer->duration += (double)((timer->t2.tv_sec - timer->t1.tv_sec) * 1000000 +
                              timer->t2.tv_usec - timer->t1.tv_usec) /
                     1000000;
}

static inline double timer_report_sec(timer_tt *timer) {
  return timer->duration;
}

static inline void timer_free(timer_tt *timer) { free(timer); }

#endif
