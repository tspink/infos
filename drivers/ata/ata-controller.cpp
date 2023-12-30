/* SPDX-License-Identifier: MIT */

/*
 * drivers/ata/ata-controller.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/ata/ata-controller.h>
#include <infos/drivers/ata/ata-device.h>
#include <infos/util/lock.h>
#include <infos/kernel/kernel.h>
#include <arch/x86/pio.h>

using namespace infos::drivers;
using namespace infos::drivers::ata;
using namespace infos::kernel;
using namespace infos::arch::x86;
using namespace infos::util;

ComponentLog infos::drivers::ata::ata_log(syslog, "ata");

DeviceClass ATAController::ATAControllerDeviceClass(Device::RootDeviceClass, "atactl");

#define PORT_OR_BASE_ADDRESS(__v, __p) ((__v == 0) ? (__p) : (__v & ~3))

ATAController::ATAController(IRQ& irq, const ATAControllerConfiguration& cfg) : _irq(irq)
{
	channels[ATA_PRIMARY].base = PORT_OR_BASE_ADDRESS(cfg.BAR[0], 0x1F0);
	channels[ATA_PRIMARY].ctrl = PORT_OR_BASE_ADDRESS(cfg.BAR[1], 0x3F6);
	channels[ATA_SECONDARY].base = PORT_OR_BASE_ADDRESS(cfg.BAR[2], 0x170);
	channels[ATA_SECONDARY].ctrl = PORT_OR_BASE_ADDRESS(cfg.BAR[3], 0x376);

	channels[ATA_PRIMARY].bmide = (cfg.BAR[4] & ~3);
	channels[ATA_SECONDARY].bmide = (cfg.BAR[4] & ~3) + 8;

	channels[ATA_PRIMARY].nIEN = 2;
	channels[ATA_SECONDARY].nIEN = 2;
}

bool ATAController::init(kernel::DeviceManager& dm)
{
	ata_log.messagef(LogLevel::INFO, "Initialising ATA storage device Status=%u", ata_read(0, ATA_REG_STATUS));

	// Disable interrupts
	ata_write(ATA_PRIMARY, ATA_REG_CONTROL, 2);
	ata_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

	//UniqueLock<IRQLock> l(IRQLock::Instance);

	bool success = true;
	for (int channel = 0; channel < 2; channel++) {
		success &= probe_channel(dm, channel);
	}

	return success;
}

bool ATAController::probe_channel(kernel::DeviceManager& dm, int channel)
{
	bool success = true;
	for (int slave = 0; slave < 2; slave++) {
		success &= probe_device(dm, channel, slave);
	}

	return true;
}

bool ATAController::probe_device(kernel::DeviceManager& dm, int channel, int device)
{
	ata_log.messagef(LogLevel::DEBUG, "Probing device %d:%d", channel, device);

	ata_write(channel, ATA_REG_HDDEVSEL, 0xa0 | (device << 4));
	sys.spin_delay(Milliseconds(1));

	ata_write(channel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	sys.spin_delay(Milliseconds(1));

	if (ata_read(channel, ATA_REG_STATUS) == 0) return true;

	while (true) {
		uint8_t status = ata_read(channel, ATA_REG_STATUS);
		if (status & ATA_SR_ERR) {
			ata_log.message(LogLevel::ERROR, "ATA device error");
			return false;
		}

		if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) {
			break;
		}
	}

	ata_log.messagef(LogLevel::INFO, "Found ATA device");

	ATADevice *dev = new ATADevice(*this, channel, device);
	if (!dm.register_device(*dev)) {
		delete dev;
		return false;
	}
	
	return true;
}

uint8_t ATAController::ata_read(int channel, int reg)
{
	uint8_t result = 0;
	if (reg > 0x07 && reg < 0x0C) {
		ata_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
	}

	if (reg < 0x08) {
		result = __inb(channels[channel].base + reg - 0x00);
	} else if (reg < 0x0C) {
		result = __inb(channels[channel].base + reg - 0x06);
	} else if (reg < 0x0E) {
		result = __inb(channels[channel].ctrl + reg - 0x0C);
	} else if (reg < 0x16) {
		result = __inb(channels[channel].bmide + reg - 0x0E);
	}

	if (reg > 0x07 && reg < 0x0C) {
		ata_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
	}

	return result;
}

void ATAController::ata_read_buffer(int channel, int reg, void* buffer, size_t size)
{
	if (reg > 0x07 && reg < 0x0C) {
		ata_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
	}
	
	if (reg < 0x08) {
		__insl(channels[channel].base + reg - 0x00, (uintptr_t)buffer, size >> 2);
	} else if (reg < 0x0C) {
		__insl(channels[channel].base + reg - 0x06, (uintptr_t)buffer, size >> 2);
	} else if (reg < 0x0E) {
		__insl(channels[channel].ctrl + reg - 0x0C, (uintptr_t)buffer, size >> 2);
	} else if (reg < 0x16) {
		__insl(channels[channel].bmide + reg - 0x0E, (uintptr_t)buffer, size >> 2);
	}
	
	if (reg > 0x07 && reg < 0x0C) {
		ata_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
	}
}

void ATAController::ata_write(int channel, int reg, uint8_t data)
{
	if (reg > 0x07 && reg < 0x0C) {
		ata_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
	}

	if (reg < 0x08) {
		__outb(channels[channel].base + reg - 0x00, data);
	} else if (reg < 0x0C) {
		__outb(channels[channel].base + reg - 0x06, data);
	} else if (reg < 0x0E) {
		__outb(channels[channel].ctrl + reg - 0x0C, data);
	} else if (reg < 0x16) {
		__outb(channels[channel].bmide + reg - 0x0E, data);
	}

	if (reg > 0x07 && reg < 0x0C) {
		ata_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
	}
}

int ATAController::ata_poll(int channel, bool error_check)
{
	sys.spin_delay(Nanoseconds(400));
	
	while (ata_read(channel, ATA_REG_STATUS) & ATA_SR_BSY) asm volatile("pause");

	if (error_check) {
		uint8_t status = ata_read(channel, ATA_REG_STATUS);

		if (status & ATA_SR_ERR)
		   return 2;

		if (status & ATA_SR_DF)
		   return 1; // Device Fault.

		if ((status & ATA_SR_DRQ) == 0)
		   return 3;
	}

	return 0; // No Error.
}
