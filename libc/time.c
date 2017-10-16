#include <time.h>

static struct tm _gmtime_tm = {0};
static struct tm _localtime_tm = {0};

struct tm* gmtime(const time_t* timer) {

    return &_gmtime_tm;
}

struct tm* localtime(const time_t* timer) {

    return &_localtime_tm;
}

time_t time(time_t* secs) {

    if (secs)
        *secs = (time_t)0;
    
    return (time_t)0;

}

size_t strftime(char* str, size_t maxsize, const char* format, const struct tm* timeptr) {

    str[0] = 0;
    return 1;

}
