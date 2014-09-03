#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

class Timer {

  private:
     double m_start;
     double m_stop;
     struct timeval now;

  public:

    Timer() {
       m_start=0;
       m_stop = 0;
    }

    void start() {
       gettimeofday(&now, NULL);
       m_start = (now.tv_sec*1000.0)+(now.tv_usec/1000.0);
    }

    double stop() {
       gettimeofday(&now, NULL);
       m_stop = (now.tv_sec*1000.0)+(now.tv_usec/1000.0);
       return m_stop - m_start;
    }

};

#endif /* TIMER_H */
