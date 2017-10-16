#ifndef __ASSERT_H__
#define __ASSERT_H__

#ifndef NDEBUG

    #include <stdio.h>
    #include <stdlib.h>

    #define assert(EXPR) ({ \
        if (!(EXPR)) { \
            fprintf(stderr, "%s:%d: assertion failed.\n", \
                __FILE__ \
                __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    })

#else

    #define assert(EXPR) ({ })

#endif

#endif
