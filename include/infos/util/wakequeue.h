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
		class WakeQueue
		{
		public:
            void sleep(kernel::Thread& thread);
            void wake();

		private:
			util::List<kernel::Thread *> _waiters;
		};
	}
}
