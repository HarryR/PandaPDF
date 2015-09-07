#pragma once
#ifndef LOGGER_H_
#define LOGGER_H_

#pragma interface

#include <stdio.h>
#include <stdarg.h>

enum LoggerLevel {
	LOG_PRINT = -1,
	LOG_DEBUG = 0,
	LOG_INFO = 1,
	LOG_WARN = 2,
	LOG_ERROR = 3
};

class Logger {
private:
	FILE *m_info_output;
	FILE *m_error_output;
	LoggerLevel m_level;

protected:
	void log(LoggerLevel level, const char* format, va_list arg);
	bool can_log(LoggerLevel level) const;
	const char* level_name(LoggerLevel level) const;
	FILE *output(LoggerLevel level) const;

public:
	Logger(const Logger &logger);
	Logger(FILE *info_output, FILE *error_output, LoggerLevel level);
	~Logger();
	void set_level(int level);
	void println(const char *format, ...);
	void debug(const char *format, ...);
	void info(const char *format, ...);
	void warn(const char *format, ...);
	void error(const char *format, ...);
};

#endif
