/* SPDX-License-Identifier: MIT */

/*
 * include/kernel/log.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/define.h>
#include <infos/util/lock.h>

namespace infos
{
	namespace io
	{
		class Stream;
	}
	
	namespace kernel
	{
		namespace LogLevel
		{
			enum LogLevel
			{
				DEBUG,
				INFO,
				WARNING,
				ERROR,
				FATAL,
				IMPORTANT
			};
		}
		
		class Log
		{
		public:
			Log() : _enabled(true) { }
			
			virtual void message(LogLevel::LogLevel level, const char *message) = 0;
			void messagef(LogLevel::LogLevel level, const char *format, ...);
			
			void enable() { _enabled = true; }
			void disable() { _enabled = false; }
			bool enabled() const { return _enabled; }
			
		private:
			bool _enabled;
		};
		
		class SysLog : public Log
		{
		public:
			SysLog() : _colour(false), _stream(NULL) { }
			
			void colour(bool colour) { _colour = colour; }
			bool colour() const { return _colour; }
			void set_stream(io::Stream& stream) { _stream = &stream; }
			void message(LogLevel::LogLevel level, const char *message) override;
			
		private:
			bool _colour;
			io::Stream *_stream;
			util::Mutex _mtx;
		};
		
		class ComponentLog : public Log
		{
		public:
			ComponentLog(Log& parent, const char *component_name);
			void message(LogLevel::LogLevel level, const char *message) override;
						
		private:
			Log& _parent;
			const char *_component_name;
		};
		
		extern SysLog syslog;
	}
}
