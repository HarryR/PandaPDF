#pragma implementation "logger.hh"
#include "logger.hh"

// Copyright (c) 2010-2013 PixelMags Inc. All Rights Reserved

// -----------------------------------------------------------------------------

void
Logger::log(LoggerLevel level, const char* format, va_list arg) {
	if( can_log(level) ) {
		FILE *out_fh = output(level);

		if( level != LOG_PRINT ) {
			::fprintf(out_fh, "[%s] ", level_name(level));
			::vfprintf(out_fh, format, arg);
		}
		else {
			::vfprintf(out_fh, format, arg);
		}
		::fprintf(out_fh, "\n");			
		::fflush(out_fh);
	}
}

FILE *
Logger::output(LoggerLevel level) const {
	switch(level) {
	case LOG_PRINT:
	case LOG_DEBUG:
	case LOG_INFO:
		return m_info_output;
	default:
		return m_error_output;
	}
}

void
Logger::set_level(int level) {
	m_level = static_cast<LoggerLevel>(level);
}

const char*
Logger::level_name(LoggerLevel level) const {
	switch(level) {
	case LOG_DEBUG:
		return "DEBUG";
	case LOG_INFO:
		return "INFO";
	case LOG_WARN:
		return "WARN";
	case LOG_ERROR:
		return "ERROR";
	default:
		return "UNKNOWN!";
	}
}

bool
Logger::can_log(LoggerLevel level) const {
	return level == LOG_PRINT || level >= m_level;
}

Logger::Logger(FILE *info_output, FILE *error_output, LoggerLevel level)
: m_info_output(info_output), m_error_output(error_output), m_level(level)
{

}

Logger::Logger(const Logger &logger)
: m_info_output(logger.m_info_output)
, m_error_output(logger.m_error_output)
, m_level(logger.m_level)
{}

Logger::~Logger() {

}

void
Logger::debug(const char *format, ...) {
	va_list args;
	::va_start(args, format);
	log(LOG_DEBUG, format, args);
	::va_end(args);
}

void
Logger::println(const char *format, ...) {
	va_list args;
	::va_start(args, format);
	log(LOG_PRINT, format, args);
	::va_end(args);
}

void
Logger::info(const char *format, ...) {
	va_list args;
	::va_start(args, format);
	log(LOG_INFO, format, args);
	::va_end(args);
}

void
Logger::warn(const char *format, ...) {
	va_list args;
	::va_start(args, format);
	log(LOG_WARN, format, args);
	::va_end(args);
}

void
Logger::error(const char *format, ...) {
	va_list args;
	::va_start(args, format);
	log(LOG_ERROR, format, args);
	::va_end(args);
}
