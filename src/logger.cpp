#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/filesystem.hpp>

#include <iostream>

#include "config.h"

void initLogger()
{
    boost::filesystem::path path = boost::filesystem::path(config.getLogDir());
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

void findAndReplaceAll(std::string &data, std::string toSearch, std::string replaceStr)
{
    // Get the first occurrence
    size_t pos = data.find(toSearch);

    // Repeat till end is reached
    while (pos != std::string::npos)
    {
        // Replace this occurrence of Sub String
        data.replace(pos, toSearch.size(), replaceStr);
        // Get the next occurrence from the current position
        pos = data.find(toSearch, pos + replaceStr.size());
    }
}

void logTrace(std::string lg)
{
    BOOST_LOG_TRIVIAL(trace) << lg;
    if (config.getDebugLevel() <= DebugLevel::trace && !config.isDaemon())
    {
        findAndReplaceAll(lg, "\n", "\n\t\t");
        std::cout << "[Trace]\t\t" << lg << std::endl;
    }
}

void logDebug(std::string lg)
{
    BOOST_LOG_TRIVIAL(debug) << lg;
    if (config.getDebugLevel() <= DebugLevel::debug && !config.isDaemon())
    {
        findAndReplaceAll(lg, "\n", "\n\t\t");
        std::cout << "[Debug]\t\t" << lg << std::endl;
    }
}

void logInfo(std::string lg)
{
    BOOST_LOG_TRIVIAL(info) << lg;
    if (config.getDebugLevel() <= DebugLevel::info && !config.isDaemon())
    {
        findAndReplaceAll(lg, "\n", "\n\t\t");
        std::cout << "[Info]\t\t" << lg << std::endl;
    }
}

void logWarning(std::string lg)
{
    BOOST_LOG_TRIVIAL(warning) << lg;
    if (config.getDebugLevel() <= DebugLevel::warning && !config.isDaemon())
    {
        findAndReplaceAll(lg, "\n", "\n\t");
        std::cout << "[Warning]\t" << lg << std::endl;
    }
}

void logError(std::string lg)
{
    BOOST_LOG_TRIVIAL(error) << lg;
    if (config.getDebugLevel() <= DebugLevel::error && !config.isDaemon())
    {
        findAndReplaceAll(lg, "\n", "\n\t\t");
        std::cerr << "[Error]\t\t" << lg << std::endl;
    }
}

void logFatal(std::string lg)
{
    BOOST_LOG_TRIVIAL(fatal) << lg;
    if (config.getDebugLevel() <= DebugLevel::fatal && !config.isDaemon())
    {
        findAndReplaceAll(lg, "\n", "\n\t\t");
        std::cerr << "[Fatal]\t\t" << lg << std::endl;
    }
}