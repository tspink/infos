/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/uart.cpp
 *
 * by Stephen Kell <srk31@srcf.ucam.org>
 * borrowing heavily from MIT's xv6 uart.c code.
 */
#include <arch/x86/pio.h>
#include <infos/kernel/device-manager.h>
#include <infos/kernel/irq.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/log.h>
#include <infos/drivers/irq/ioapic.h>
#include <infos/drivers/irq/lapic.h>
#include <arch/x86/uart.h>
#include <infos/drivers/terminal/terminal.h>
#include <arch/x86/pio.h>

using namespace infos::kernel;
using namespace infos::drivers;
using namespace infos::drivers::irq;
using namespace infos::arch::x86;

const uint16_t UART::COM1 = 0x3f8;
const int UART::IRQ_COM1 = 4;

const DeviceClass UART::UARTDeviceClass(Device::RootDeviceClass, "uart");

int UART::read(void* buffer, size_t size)
{
	for (unsigned int i = 0; i < size; i++) {
		((char *)buffer)[i] = getc();
	}
	return size;
}

int UART::write(const void* buffer, size_t size)
{
	for (unsigned int i = 0; i < size; i++) {
		char c = ((char *)buffer)[i];
#if 0
		syslog.message(LogLevel::INFO, "UART putting a character: ");
		char cs[] = { c, 0 };
		if (c >= 32 && c < 128) syslog.message(LogLevel::INFO, cs);
		else if (c < 32) syslog.message(LogLevel::INFO, "(control code)");
		else syslog.message(LogLevel::INFO, "(high byte)");
#endif
		/* HACK: this translation doesn't belong in the UART code,
		 * but in a 'serial console' translation layer or possibly
		 * just in SerialTerminal. */
		if (c == '\n') { putc('\r'); putc('\n'); }
		else putc(c);
	}
	return size;
}

UART::UART() : _attached_terminal(nullptr)
{
	syslog.message(LogLevel::INFO, "created a UART");
	//init();
	assert(present);
	// Announce that we're here.
#define write_string(s) write((s), sizeof (s) - 1)
	write_string("Hello from the InfOS serial console\nLots and lots of very"
	"very long text to debug the information on possible error conditions that may"
	" exist within the UART, based on the data that has been received. Keep in mind"
	" that this is a \"read only\" register, and any data written to this register"
	" is likely to be ignored or worse, cause different behavior in the UART."
	" There are several uses for this information, and some information will"
	" be given below on how it can be useful for diagnosing problems with your"
	" serial data connection: "
	); // I get hex 1bf + ten  bytes of 02 82 after this message, regardless...
}

bool UART::init(kernel::DeviceManager& dm)
{
	syslog.message(LogLevel::INFO, "initing a UART");
	__outb(COM1 + FCR, 0);           // turn off the on-chip FIFO (we want a plain old 8250)
	__outb(COM1 + LCR, 0x80);        // unlock divisor
	__outb(COM1 + DLL, 115200/9600); // divide the clock by 12 to get 9600 baud
	__outb(COM1 + DLH, 0);           // high bit of 12 is 0
	__outb(COM1 + LCR, 0x03);        // lock divisor and set 8 data bits, 1 stop bit, no parity
	__outb(COM1 + MCR, 0);           // clear the modem control register
	__outb(COM1 + IER, 0x01);        // enable receive interrupts.

	// If status is 0xFF, no serial port.
	if (__inb(COM1 + LSR) == 0xFF) { this->present = false; return false; }
	this->present = true;

	// acknowledge any pre-existing interrupt and 'consume' data to re-enable.
	__inb(COM1 + IIR);
	__inb(COM1 + RBR);

	// Find the LAPIC.
	LAPIC *lapic;
	if (!dm.try_get_device_by_class(LAPIC::LAPICDeviceClass, lapic)) {
		return false;
	}

	// Find the IOAPIC.
	IOAPIC *ioapic;
	if (!dm.try_get_device_by_class(IOAPIC::IOAPICDeviceClass, ioapic)) {
		return false;
	}

	// Hook-up IRQ 4 on the IOAPIC to the IRQ handler object, and register
	// the IRQ callback function.
	_irq = ioapic->request_physical_irq(lapic, IRQ_COM1);
	_irq->attach(UART::irq_handler, this);

	// success
	return true;
}

void
UART::putc(int c)
{
	if (!present) return;
	// Test for THR empty (=> ready to send), but wait at most 1280 us
	int i;
	uint8_t lsr;
	for (i = 0; i < 128 && !((lsr = __inb(COM1 + LSR)) & 0x20); i++) sys.spin_delay(util::Microseconds(10));
	if (i == 128) syslog.message(LogLevel::WARNING, "outputting to a UART with non-empty THR");
	if (lsr & /* break interrupt, framing, parity, overrun error? */ 0x1e)
	{
		syslog.message(LogLevel::WARNING, "error on UART putc, but proceeding");
		// continue
	}
	__outb(COM1 + THR, c);
}

int UART::getc()
{
	if (!present) return -1;
	uint8_t lsr = __inb(COM1 + LSR);
	if (!(lsr & /* data ready? */ 0x1))
	{
		syslog.message(LogLevel::WARNING, "getc from a UART with no data ready");
		return -1; // not ready
	}
	if (lsr & /* framing error? */ 0x1<<3)
	{
		syslog.message(LogLevel::WARNING, "framing error on UART getc, but proceeding");
		// continue
	}
	return __inb(COM1 + RBR);
}

void UART::irq_handler(const kernel::IRQ *irq, void *priv)
{
// xv6 calls uartintr from trap()
// which is called from 'alltraps' in trapasm.S
// which is called with args (trapno, 0) from a trampoline in vectors.S (one for each of 256 trapnos)
// which also defines the 256-entry vector table, each pointing to its trampoline

	/* This asks the console subsystem to buffer our pending data if there is any. */
	/* XXX */ // consoleintr(uartgetc);

	UART *uart = (UART *)priv;
	int c = uart->getc();
#if 0 /* This is just for testing */
	if (c != -1) {
		syslog.message(LogLevel::INFO, "UART interrupt yielded a character"/*, (char) c*/);
	} else {
		syslog.message(LogLevel::INFO, "UART interrupt yielded -1");
	}
#endif
	// buffer this as pending data
	if (uart->_attached_terminal) uart->_attached_terminal->buffer_raw_character(c);
}
