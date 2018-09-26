/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/define.h>
#include <infos/util/list.h>

namespace infos
{
	namespace kernel
	{
		class Thread;
	}
	
	namespace util
	{
		class Event
		{
		public:
			Event() : _triggered(0) { }
			
			void trigger();
			void wait();
			void reset();
			
		private:
			util::List<kernel::Thread *> _waiters;
			
			volatile uint32_t _triggered;
		};
	}
}
