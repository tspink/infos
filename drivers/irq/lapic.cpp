/* SPDX-License-Identifier: MIT */

/*
 * drivers/irq/lapic.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/irq/lapic.h>
#include <arch/x86/x86-arch.h>

#define MASKED     0x00010000   // Interrupt masked

#define INIT       0x00000500   // INIT/RESET
#define STARTUP    0x00000600   // Startup IPI
#define DELIVS     0x00001000   // Delivery status
#define ASSERT     0x00004000   // Assert interrupt (vs deassert)
#define DEASSERT   0x00000000
#define LEVEL      0x00008000   // Level triggered
#define BCAST      0x00080000   // Send to all APICs, including self.
#define BUSY       0x00001000
#define FIXED      0x00000000

#define X1         0x0000000B   // divide counts by 1
#define PERIODIC   0x00020000   // Periodic

using namespace infos::drivers;
using namespace infos::drivers::irq;
using namespace infos::arch::x86;

const DeviceClass infos::drivers::irq::LAPIC::LAPICDeviceClass(RootDeviceClass, "lapic");

LAPIC::LAPIC(virt_addr_t base_address) : _apic_base((volatile uint32_t *)base_address)
{

}

bool LAPIC::init(kernel::DeviceManager& dm)
{
	// Specify the spurious interrupt vector, and enable the device.
	write(LAPICRegisters::SVR, 0x1ff);

	// Mask all interrupt sources.
	write(LAPICRegisters::TIMER, MASKED);
	write(LAPICRegisters::LINT0, MASKED);
	write(LAPICRegisters::LINT1, MASKED);
	write(LAPICRegisters::ERROR, MASKED);

	// If the APIC version is >= 4, mask the PCINT interrupt source
	if (((read(LAPICRegisters::VER) >> 16) & 0xFF) >= 4) {
		write(LAPICRegisters::PCINT, MASKED);
	}

	// Clear-out the ESR
	write(LAPICRegisters::ESR, 0);
	write(LAPICRegisters::ESR, 0);

	// Acknowledge any pending interrupts
	write(LAPICRegisters::EOI, 0);

	// Set-up the interrupt control register
	write(LAPICRegisters::ICRHI, 0);
	write(LAPICRegisters::ICRLO, BCAST | INIT | LEVEL);

	// Wait for pending deliveries to complete
	while (read(LAPICRegisters::ICRLO) & DELIVS);
	
	write(LAPICRegisters::TPR, 0);
	
	// Initialise the timer IRQ
	_timer_irq = new LAPICIRQ(*this, Timer);
	if (!x86arch.irq_manager().attach_irq(_timer_irq)) {
		return false;
	}
	
	set_timer_irq(_timer_irq->nr());
	return true;
}

void LAPIC::eoi()
{
	write(LAPICRegisters::EOI, 0);
}

void LAPIC::mask_interrupts(LVTs lvt)
{
	switch (lvt) {
	case Timer:
		set_flag(LAPICRegisters::TIMER, MASKED);
		break;
	case PMU:
		set_flag(LAPICRegisters::PCINT, MASKED);
		break;
	case LINT0:
		set_flag(LAPICRegisters::LINT0, MASKED);
		break;
	case LINT1:
		set_flag(LAPICRegisters::LINT1, MASKED);
		break;
	case Error:
		set_flag(LAPICRegisters::ERROR, MASKED);
		break;
	}
}

void LAPIC::unmask_interrupts(LVTs lvt)
{
	switch (lvt) {
	case Timer:
		clear_flag(LAPICRegisters::TIMER, MASKED);
		break;
	case PMU:
		clear_flag(LAPICRegisters::PCINT, MASKED);
		break;
	case LINT0:
		clear_flag(LAPICRegisters::LINT0, MASKED);
		break;
	case LINT1:
		clear_flag(LAPICRegisters::LINT1, MASKED);
		break;
	case Error:
		clear_flag(LAPICRegisters::ERROR, MASKED);
		break;
	}
}

void LAPIC::set_timer_irq(uint8_t irq)
{
	uint32_t v = read(LAPICRegisters::TIMER);
	v = (v & ~0xff) | irq;
	
	write(LAPICRegisters::TIMER, v);
}

void LAPIC::set_timer_divide(uint8_t v)
{
	write(LAPICRegisters::TDCR, v);
}

void LAPIC::set_timer_initial_count(uint32_t v)
{
	write(LAPICRegisters::TICR, v);
}

uint32_t LAPIC::get_timer_current_count()
{
	return read(LAPICRegisters::TCCR);
}

void LAPIC::set_timer_one_shot()
{
	clear_flag(LAPICRegisters::TIMER, PERIODIC);
}

void LAPIC::set_timer_periodic()
{
	set_flag(LAPICRegisters::TIMER, PERIODIC);
}

void LAPIC::LAPICIRQ::enable()
{
	_lapic.unmask_interrupts(_lvt);
}

void LAPIC::LAPICIRQ::disable()
{
	_lapic.mask_interrupts(_lvt);
}

void LAPIC::LAPICIRQ::handle() const
{
	if (!invoke()) {
		arch_abort();
	} else {
		_lapic.eoi();
	}
}
