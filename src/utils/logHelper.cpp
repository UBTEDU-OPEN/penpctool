#include "logHelper.h"
#include "config.h"

LogHelper::LogHelper(const char* argv)
{
    google::InitGoogleLogging(argv);
    auto logLevel = Config::getLogLevel();
    google::SetLogDestination(logLevel, Config::getLogPath().toStdString().c_str());
    FLAGS_logbufsecs = Config::getLogBufSecs();
    FLAGS_max_log_size = Config::getLogMaxSize();
    FLAGS_alsologtostderr = 1;
    FLAGS_stop_logging_if_full_disk = true;
}

LogHelper::~LogHelper()
{
    google::ShutdownGoogleLogging();
}

void LogHelper::init(const char* argv)
{
    static LogHelper inst(argv);
}


