/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/define.h>

namespace infos
{
	namespace kernel
	{
		class DeviceManager;
	}
	
	namespace drivers
	{
		class Controller
		{
		public:
			Controller(kernel::DeviceManager& dm);
			virtual ~Controller();
			
			kernel::DeviceManager& device_manager() const { return _dm; }
			
		private:
			kernel::DeviceManager& _dm;
		};
	}
}
