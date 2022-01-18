#ifndef TRACELOG_H
#define TRACELOG_H

//#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
//#include <log4cplus/configurator.h>
//#include <log4cplus/initializer.h>

enum LOG_LEVEL
{
    LL_INFO = 0,
    LL_DEBUG,
    LL_WARN,
    LL_ERROR,
    LL_FATAL
};

class TraceLog
{
public:
    TraceLog();

    bool initTraceLog(const char *config);
    void printTrace(LOG_LEVEL level, const char *format, ...);

private:
    //    log4cplus::TraceLogger m_traceLogger;
    log4cplus::Logger m_traceLogger;
};

TraceLog *getTraceLog(void);

#endif // TRACELOG_H
