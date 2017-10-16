#ifndef __TIME_H__
#define __TIME_H__

#include <stdint.h>
#include <stddef.h>

typedef int64_t time_t;

struct tm {

    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;

};

time_t time(time_t* secs);
struct tm* gmtime(const time_t* timer);
size_t strftime(char* str, size_t maxsize, const char* format, const struct tm* timeptr);
struct tm* localtime(const time_t* timer);

#endif
