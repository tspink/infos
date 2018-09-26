/* SPDX-License-Identifier: MIT */

/*
 * kernel/log.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/kernel/log.h>
#include <infos/io/stream.h>
#include <infos/util/string.h>
#include <infos/util/printf.h>

using namespace infos::kernel;
using namespace infos::util;

__init_priority(101) SysLog infos::kernel::syslog;

void Log::messagef(LogLevel::LogLevel level, const char* format, ...)
{
	if (!enabled()) return;
	
	char buffer[0x200];
	va_list args;

	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	message(level, buffer);
}


void SysLog::message(LogLevel::LogLevel level, const char* message)
{
	if (!enabled()) return;
	
	if (_stream) {
		UniqueLock<Mutex> l(_mtx);

		switch (level) {
		case LogLevel::DEBUG:
			if (_colour) {
				_stream->write("\x1b[34;1m", 7);
			}
			_stream->write("  debug: ", 9);
			break;
		case LogLevel::INFO:
			if (_colour) {
				_stream->write("\x1b[32;1m", 7);
			}
			_stream->write("   info: ", 9);
			break;
		case LogLevel::WARNING:
			if (_colour) {
				_stream->write("\x1b[32;1m", 7);
			}
			_stream->write("warning: ", 9);
			break;
		case LogLevel::ERROR:
			if (_colour) {
				_stream->write("\x1b[31;1m", 7);
			}
			_stream->write("  error: ", 9);
			break;
		case LogLevel::FATAL:
			if (_colour) {
				_stream->write("\x1b[31;1m", 7);
			}
			_stream->write("  fatal: ", 9);
			break;
		case LogLevel::IMPORTANT:
			if (_colour) {
				_stream->write("\x1b[37;1;42m", 10);
			}
			_stream->write(" notice: ", 9);
			break;
		}

		if (_colour && level != LogLevel::IMPORTANT) {
			_stream->write("\x1b[37;0m", 7);
		}
		
		_stream->write(message, strlen(message));
		if (_colour && level == LogLevel::IMPORTANT) {
			_stream->write("\x1b[37;0m", 7);
		}

		_stream->write("\n", 1);
	}
}

ComponentLog::ComponentLog(Log& parent, const char *component_name) : _parent(parent), _component_name(component_name)
{

}

void ComponentLog::message(LogLevel::LogLevel level, const char* message)
{
	if (!enabled()) return;
	
	char message_buffer[0x200];
	snprintf(message_buffer, sizeof(message_buffer), "%s: %s", _component_name, message);
	
	_parent.message(level, message_buffer);
}
