#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <vector>

class Timer {

  private:
     struct timespec m_start;
     struct timespec m_stop;
     std::vector<double> m_list;

  public:

    Timer() {
    }

    inline void start() {
       clock_gettime(CLOCK_MONOTONIC,&m_start);
    }

    inline void stop() {
      clock_gettime(CLOCK_MONOTONIC,&m_stop);
      double elapsed_us = (m_stop.tv_sec - m_start.tv_sec) * 1000000.0;
      elapsed_us += (m_stop.tv_nsec - m_start.tv_nsec) / 1000.0;
      m_list.push_back(elapsed_us);
    }

    inline int size()
    {
      return m_list.size();
    }

    inline double usAvg()
    {
      double size = m_list.size();
      if (size <= 0)
      {
        return 0;
      }
      else
      {
        double usAvg=0;
        while (!m_list.empty())
        {
          usAvg += m_list.back();
          m_list.pop_back();
        }
        return usAvg/size;
      }
    }

};

#endif /* TIMER_H */
