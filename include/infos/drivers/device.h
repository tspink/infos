/* SPDX-License-Identifier: MIT */

/*
 * include/drivers/device.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/define.h>
#include <infos/util/string.h>

namespace infos {
	namespace kernel {
		class DeviceManager;
	}

	namespace fs {
		class File;
	}

	namespace drivers {
		class Device;

		struct DeviceClass {
			friend class Device;
			friend class kernel::DeviceManager;

			explicit DeviceClass(const DeviceClass& _parent, const char *_name) : parent(&_parent), name(_name), instance(0)
			{
			}

			const DeviceClass *parent;
			const char *name;

			inline bool is(const DeviceClass& type) const
			{
				if (this == &type) return true;
				if (parent != NULL) return parent->is(type);
				return false;
			}

			inline bool operator==(const DeviceClass& other) const
			{
				return &other == this;
			}

			inline bool operator!=(const DeviceClass& other) const
			{
				return &other != this;
			}

		private:

			inline uint64_t acquire_instance() const
			{
				return instance++;
			}

			DeviceClass() : parent(NULL), name(""), instance(0)
			{
			}
			mutable uint64_t instance;
		};

		class Device {
		public:
			static const DeviceClass RootDeviceClass;

			Device();
			virtual ~Device();

			virtual bool init(kernel::DeviceManager& dm);

			virtual const DeviceClass& device_class() const
			{
				return RootDeviceClass;
			}

			void assign_name(const util::String& name)
			{
				_name = name;
			}

			const util::String& name() const
			{
				return _name;
			}

			virtual fs::File *open_as_file();

		private:
			util::String _name;
		};

		// static _class __device_##_class; __section(".device") infos::drivers::Device *__device_ptr_##_class = &__device_##_class

		typedef Device *(*device_ctor_fn)(void);
		
		#define RegisterDevice(_class) static infos::drivers::Device *__construct_device_##_class() { \
			return new _class(); \
		} \
		__section(".devctor") device_ctor_fn __construct_device_ptr_##_class = __construct_device_##_class
	}
}
