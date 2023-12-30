/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/io/stream.h>
#include <infos/kernel/device-manager.h>
#include <infos/kernel/irq.h>
#include <arch/x86/uart.h>
#include <infos/drivers/device.h>
namespace infos
{
	namespace drivers {
		namespace terminal {
			class Terminal;
			class SerialTerminal;
		}
	}
	using drivers::terminal::SerialTerminal;
	namespace arch
	{
		namespace x86
		{
			class UART : public infos::drivers::Device
			{
				bool present;
			public:
				static const infos::drivers::DeviceClass UARTDeviceClass;
				UART();
				bool init(kernel::DeviceManager& dm) override;
				static const uint16_t COM1;
				static const int IRQ_COM1;
				enum UartRegister
				{
					/* W */  THR = 0,  /* Transmitter Holding Buffer -- only when DLAB == 0 */
					/* R */  RBR = 0,  /* Receiver Buffer -- only when DLAB == 0 */
					/* RW */ DLL = 0, /* Divisor Latch Low Byte -- only when DLAB == 1*/
					/* RW */ IER = 1, /* Interrupt Enable -- only when DLAB == 0 */
					/* RW */ DLH = 1, /* Divisor Latch High Byte -- only when DLAB == 1*/
					/* R */  IIR = 2,  /* Interrupt Identification */
					/* W */  FCR = 2,  /* FIFO Control */
					/* RW */ LCR = 3, /* Line Control */
					/* RW */ MCR = 4, /* Modem Control */
					/* R */  LSR = 5,  /* Line Status */
					/* R */  MSR = 6,  /* Modem Status */
					/* RW */ SR = 7,  /* Scratch */
				};
				int read(void* buffer, size_t size);
				int write(const void* buffer, size_t size);
				void putc(int c);
				int getc();
				static void irq_handler(const kernel::IRQ *irq, void *priv);
				void attach_terminal(SerialTerminal& term) { _attached_terminal = &term; }
				friend class SerialTerminal; // a bit nasty
			private:
				kernel::IRQ *_irq;
				SerialTerminal *_attached_terminal;
			};
		}
	}
}

