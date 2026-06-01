#include "pausa.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  void so_delay_ms(unsigned ms)
  {
      Sleep((DWORD)ms);
  }
#else
  #include <time.h>
  void p_ms(unsigned ms)
  {
      struct timespec ts;
      ts.tv_sec  = (time_t)(ms / 1000);
      ts.tv_nsec = (long)(ms % 1000) * 1000000L;
      nanosleep(&ts, 0);
  }
#endif
