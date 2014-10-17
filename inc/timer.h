#ifndef TIMER_H
#define TIMER_H

#include <time.h>

class Timer {

  private:
     struct timespec m_start;
     struct timespec m_stop;

  public:

    Timer() {
    }

    inline void start() {
       clock_gettime(CLOCK_MONOTONIC,&m_start);
    }

    inline double stop() {
	   clock_gettime(CLOCK_MONOTONIC,&m_stop);
	   return 1000*(m_stop.tv_sec - m_start.tv_sec) + (m_stop.tv_nsec - m_start.tv_nsec)/1000000.0;
    }

};

#endif /* TIMER_H */
