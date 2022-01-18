
#include "log/tracelog.h"

/*
   log4cplus macro
*/
#define W_PRINT(LEVEL, fmt, ...) \
    getTraceLog()->printTrace(LEVEL, fmt, ##__VA_ARGS__)

#define W_INFO(fmt, ...) getTraceLog()->printTrace(LL_INFO, fmt, ##__VA_ARGS__)

#define W_DEBUG(fmt, ...) \
    getTraceLog()->printTrace(LL_DEBUG, fmt, ##__VA_ARGS__)

#define W_WARN(fmt, ...) getTraceLog()->printTrace(LL_WARN, fmt, ##__VA_ARGS__)

#define W_ERROR(fmt, ...) \
    getTraceLog()->printTrace(LL_ERROR, fmt, ##__VA_ARGS__)

#define W_FATAL(fmt, ...) \
    getTraceLog()->printTrace(LL_FATAL, fmt, ##__VA_ARGS__)
