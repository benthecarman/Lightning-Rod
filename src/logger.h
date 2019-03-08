#ifndef LOGGER_H
#define LOGGER_H


void initLogger();
void logTrace(std::string lg);
void logDebug(std::string lg);
void logInfo(std::string lg);
void logWarning(std::string lg);
void logError(std::string lg);
void logFatal(std::string lg);

#endif