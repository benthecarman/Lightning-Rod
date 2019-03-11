#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/filesystem.hpp>

#include <stdio.h>

#include "config.h"

void initLogger()
{
    boost::filesystem::path path = boost::filesystem::path(getenv("HOME") + config.getLogDir());
    boost::filesystem::create_directories(path);

    boost::log::add_file_log(
        boost::log::keywords::file_name = path.string() + "%Y-%m-%d %H:%M:%S.log",
        boost::log::keywords::auto_flush = true,
        boost::log::keywords::format =
            (boost::log::expressions::stream
             << "<" << boost::log::trivial::severity
             << "> " << boost::log::expressions::smessage),
        boost::log::keywords::rotation_size = 10 * 1024 * 1024,
        boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0));
}

void logTrace(std::string lg)
{
    if (config.isDebug())
        printf("[Trace]\t\t%s\n", lg.c_str());
    BOOST_LOG_TRIVIAL(trace) << lg;
}

void logDebug(std::string lg)
{
    if (config.isDebug())
        printf("[Debug]\t\t%s\n", lg.c_str());
    BOOST_LOG_TRIVIAL(debug) << lg;
}

void logInfo(std::string lg)
{
    printf("[Info]\t\t%s\n", lg.c_str());
    BOOST_LOG_TRIVIAL(info) << lg;
}

void logWarning(std::string lg)
{
    printf("[Warning]\t%s\n", lg.c_str());
    BOOST_LOG_TRIVIAL(warning) << lg;
}

void logError(std::string lg)
{
    printf("[Error]\t\t%s\n", lg.c_str());
    BOOST_LOG_TRIVIAL(error) << lg;
}

void logFatal(std::string lg)
{
    printf("[Fatal]\t\t%s\n", lg.c_str());
    BOOST_LOG_TRIVIAL(fatal) << lg;
}