/* SPDX-License-Identifier: MIT */

/*
 * drivers/irq/ioapic.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/irq/ioapic.h>
#include <infos/drivers/irq/lapic.h>
#include <infos/kernel/log.h>
#include <infos/util/string.h>
#include <arch/x86/x86-arch.h>

using namespace infos::drivers;
using namespace infos::drivers::irq;
using namespace infos::kernel;
using namespace infos::util;
using namespace infos::arch::x86;

const DeviceClass infos::drivers::irq::IOAPIC::IOAPICDeviceClass(RootDeviceClass, "ioapic");

#define IOAPIC_ID	0
#define IOAPIC_VER	1

IOAPIC::IOAPIC(virt_addr_t base_address) : _base_address((volatile uint32_t *)base_address), _nr_irqs(0)
{

}

bool IOAPIC::init(kernel::DeviceManager& dm)
{
	uint32_t ver = read(IOAPIC_VER);
	
	_nr_irqs = (uint8_t)(ver >> 16);
	
	syslog.messagef(LogLevel::DEBUG, "IOAPIC ID=%u, VER=%u, NR-IRQS=%u", read(IOAPIC_ID), ver & 0xff, _nr_irqs);
	
	RedirectionEntry blank;
	bzero(&blank, sizeof(blank));
	
	for (unsigned int i = 0; i < _nr_irqs; i++) {
		store_redir_entry(i, blank);
	}
	
	return true;
}

void IOAPIC::store_redir_entry(uint8_t irq_index, RedirectionEntry& e)
{
	assert(irq_index < _nr_irqs);
	
	uint8_t configuration_register = 0x10 + (irq_index * 2);
	
	write(configuration_register, e.low);
	write(configuration_register + 1, e.high);
}

IRQ *IOAPIC::request_physical_irq(LAPIC *lapic, uint32_t phys_irq_nr)
{
	if (!lapic) {
		return NULL;
	}
	
	if (phys_irq_nr >= _nr_irqs) {
		return NULL;
	}
	
	IRQ *irq = new IOAPICIRQ(*lapic);
	if (!x86arch.irq_manager().attach_irq(irq)) {
		return NULL;
	}
	
	RedirectionEntry re;
	bzero(&re, sizeof(re));
	
	re.delivery_mode = 0;
	re.delivery_status = 0;
	re.destination = 0;					// TODO: USE LAPIC ID
	re.destination_mode = 0;
	re.mask = 0;
	re.pin_polarity = 0;
	re.remote_irr = 0;
	re.trigger_mode = 0;
	re.vector = irq->nr();
	
	store_redir_entry(phys_irq_nr, re);
	
	return irq;
}

void IOAPIC::IOAPICIRQ::handle() const
{
	invoke();
	_lapic.eoi();
}

void IOAPIC::IOAPICIRQ::enable()
{

}

void IOAPIC::IOAPICIRQ::disable()
{

}
