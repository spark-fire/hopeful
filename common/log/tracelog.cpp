#include "tracelog.h"
#include <stdio.h>
#include <iostream>

#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/initializer.h>

using namespace log4cplus;
using namespace log4cplus::helpers;

TraceLog::TraceLog()
{
    if (system("mkdir config")) {
        std::cout << "mkdir config failed !" << std::endl;
    }

    if (system("cp ../../hopeful/common/log/config/tracelog.properties "
               "./config")) {
        std::cout << "cp  tracelog.properties failed !" << std::endl;
    }

    initTraceLog("./config/tracelog.properties");
}

bool TraceLog::initTraceLog(const char *config)
{
    bool ret = true;
    try {
        log4cplus::initialize();
        LogLog::getLogLog()->setInternalDebugging(true);

        auto logger = Logger::getInstance(LOG4CPLUS_TEXT("mytrace"));
        m_traceLogger = logger;

        PropertyConfigurator::doConfigure(config);

        LOG4CPLUS_INFO(m_traceLogger, "log4cplus started.");
    } catch (...) {
        ret = false;
        LOG4CPLUS_FATAL(m_traceLogger, "Exception occured...");
    }

    return ret;
}

void TraceLog::printTrace(LOG_LEVEL level, const char *format, ...)
{
#define MAX_BUF 2048
    char buf[MAX_BUF + 1] = { 0 };
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, MAX_BUF, format, ap);
    va_end(ap);

    switch (level) {
    case LL_INFO:
        LOG4CPLUS_INFO(m_traceLogger, buf);
        break;
    case LL_DEBUG:
        LOG4CPLUS_DEBUG(m_traceLogger, buf);
        break;
    case LL_WARN:
        LOG4CPLUS_WARN(m_traceLogger, buf);
        break;
    case LL_ERROR:
        LOG4CPLUS_ERROR(m_traceLogger, buf);
        break;
    case LL_FATAL:
        LOG4CPLUS_FATAL(m_traceLogger, buf);
        break;
    }
}

TraceLog *getTraceLog()
{
    static TraceLog obj;

    return &obj;
}
