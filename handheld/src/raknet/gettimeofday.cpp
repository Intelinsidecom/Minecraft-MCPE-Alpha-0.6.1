



#if defined(_WIN32) && !defined(__GNUC__)  &&!defined(__GCCXML__)

#include "gettimeofday.h"

// From http://www.openasthra.com/c-tidbits/gettimeofday-function-for-windows/

#include "WindowsIncludes.h"

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;

  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    tmpres /= 10;  /*convert into microseconds*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }

  if (NULL != tz)
  {
#if !defined(WINAPI_FAMILY)
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
#else
    TIME_ZONE_INFORMATION tzi;
    DWORD result = GetTimeZoneInformation(&tzi);
    
    if (result != TIME_ZONE_ID_INVALID)
    {
      tz->tz_minuteswest = -tzi.Bias;
      tz->tz_dsttime = (result == TIME_ZONE_ID_DAYLIGHT) ? 1 : 0;
    }
    else
    {
      tz->tz_minuteswest = 0;
      tz->tz_dsttime = 0;
    }
#endif
  }

  return 0;
}

#endif

