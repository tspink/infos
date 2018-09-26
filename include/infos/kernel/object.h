/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/define.h>

namespace infos
{
	namespace kernel
	{
		typedef uint64_t ObjectHandle;
		
		class KernelObject
		{
		public:
			ObjectHandle get_handle() const { return _handle; }
			
			static const ObjectHandle Error;
			
		private:
			ObjectHandle _handle;
		};
	}
}
