/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/io/stream.h>

namespace infos
{
	namespace arch
	{
		namespace x86
		{
			class QEMUStream : public io::Stream
			{
			public:
				int read(void* buffer, size_t size) override;
				int write(const void* buffer, size_t size) override;
			};
			extern QEMUStream qemu_stream;
		}
	}
}
