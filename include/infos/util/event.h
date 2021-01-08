/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/define.h>
#include <infos/util/wakequeue.h>

namespace infos
{
	namespace util
	{
		class Event
		{
		public:
			void trigger();
			void wait();

		private:
			WakeQueue _wakequeue;
		};
	}
}
