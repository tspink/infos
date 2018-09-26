/* SPDX-License-Identifier: MIT */

/*
 * include/arch/x86/early-screen.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/io/stream.h>

namespace infos
{
	namespace arch
	{
		namespace x86
		{
			class EarlyScreen : public io::Stream
			{
			public:
				EarlyScreen();
				
				void init();
				
				int read(void *buffer, size_t size) override;
				int write(const void *buffer, size_t size) override;
				
			private:
				const unsigned int _width = 80;
				const unsigned int _height = 25;
				
				void putch(int attr, char c);
				void putstr(int attr, const char *str);
				void scroll();
				void clear();
				void update_cursor();
				
				uint8_t _current_x, _current_y;
				volatile uint16_t *_video_ram;
			};
		}
	}
}
