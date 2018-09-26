/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/drivers/device.h>

namespace infos
{
	namespace kernel
	{
		class IRQ;
	}
	
	namespace drivers
	{
		namespace irq
		{
			class LAPIC;
		}
		
		namespace terminal
		{
			class Terminal;
		}
		
		namespace input
		{
			namespace Keys
			{
				enum Keys
				{
					NO_KEY = 0,
					UNKNOWN_KEY,
					
					KEY_ESCAPE,
					
					KEY_A,
					KEY_B,
					KEY_C,
					KEY_D,
					KEY_E,
					KEY_F,
					KEY_G,
					KEY_H,
					KEY_I,
					KEY_J,
					KEY_K,
					KEY_L,
					KEY_M,
					KEY_N,
					KEY_O,
					KEY_P,
					KEY_Q,
					KEY_R,
					KEY_S,
					KEY_T,
					KEY_U,
					KEY_V,
					KEY_W,
					KEY_X,
					KEY_Y,
					KEY_Z,
					
					KEY_1,
					KEY_2,
					KEY_3,
					KEY_4,
					KEY_5,
					KEY_6,
					KEY_7,
					KEY_8,
					KEY_9,
					KEY_0,
					
					KEY_F1,
					KEY_F2,
					KEY_F3,
					KEY_F4,
					KEY_F5,
					KEY_F6,
					KEY_F7,
					KEY_F8,
					KEY_F9,
					KEY_F10,
					KEY_F11,
					KEY_F12,
					
					KEY_ASTERISK,
					
					KEY_LSQBRACKET,
					KEY_RSQBRACKET,
					
					KEY_DOT,
					KEY_COMMA,
					KEY_SEMICOLON,
					KEY_APOSTROPHE,
					KEY_HASH,
					KEY_BACKSLASH,
					KEY_FORSLASH,
					KEY_BACKTICK,
					KEY_HYPHEN,
					KEY_EQUALS,
					
					KEY_TAB,
					KEY_RETURN,
					KEY_SPACE,
					KEY_BACKSPACE,
					
					KEY_CAPSLOCK,
					KEY_LSHIFT,
					KEY_RSHIFT,
					KEY_LCTRL,
					KEY_RCTRL,
					KEY_LALT,
					KEY_RALT,
				};
			}
			
			class KeyboardSink
			{
			public:
				virtual void key_up(Keys::Keys key) = 0;
				virtual void key_down(Keys::Keys key) = 0;
			};
			
			class Keyboard : public Device
			{
			public:
				static const DeviceClass KeyboardDeviceClass;
				const DeviceClass& device_class() const override { return KeyboardDeviceClass; }
				
				Keyboard();

				bool init(kernel::DeviceManager& dm) override;
				
				void attach_sink(KeyboardSink& sink) { _sink = &sink; }
				
			private:
				static void keyboard_irq_handler(const kernel::IRQ *irq, void *priv);
				void handle_key_event(int8_t scancode);
				Keys::Keys scancode_to_key(int8_t scancode);
				
				kernel::IRQ *_irq;
				KeyboardSink *_sink;
			};
		}
	}
}
