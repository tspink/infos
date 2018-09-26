/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/kernel/subsystem.h>
#include <infos/kernel/object.h>
#include <infos/util/lock.h>
#include <infos/util/map.h>

namespace infos
{
	namespace kernel
	{
		class Thread;
		
		struct ObjectDescriptor
		{
			Thread *owner;
			void *object;
		};
		
		class ObjectManager : public Subsystem
		{
		public:
			ObjectManager(Kernel& owner);
			
			ObjectHandle register_object(Thread& owner, void *obj);
			void *get_object_secure(Thread& owner, ObjectHandle handle);
			
		private:
			util::Mutex _registry_lock;
			uint64_t _next_handle;
			util::Map<ObjectHandle, ObjectDescriptor> _objects;
		};
	}
}
