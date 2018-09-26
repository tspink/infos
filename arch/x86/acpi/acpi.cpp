/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/acpi/acpi.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/acpi/acpi.h>
#include <infos/util/string.h>

using namespace infos::arch::x86::acpi;
using namespace infos::util;

// ACPI Logging Component
infos::kernel::ComponentLog infos::arch::x86::acpi::acpi_log(infos::kernel::syslog, "acpi");

// Describes an entry in the RSDP
struct RSDPDescriptor {
	uint64_t signature;
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_address;
} __packed;

// Describes an entry in the RSDT
struct SDTHeader {
  uint32_t signature;
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oem_id[6];
  char oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __packed;

// Describes the RSDT
struct RSDT {
	SDTHeader header;
	uint32_t sdt_pointers[];
} __packed;

#define SIG32(__d, __c, __b, __a) ((uint32_t)(__d) | ((uint32_t)__c << 8) | ((uint32_t)__b << 16) | ((uint32_t)__a << 24))
#define RSDP_SIGNATURE	0x2052545020445352

#define MADT_SIGNATURE	SIG32('A', 'P', 'I', 'C')

struct MADTRecordHeader {
	uint8_t type, length;
} __packed;

struct MADTRecordLAPIC {
	MADTRecordHeader header;
	uint8_t acpi_processor_id;
	uint8_t apic_id;
	uint32_t flags;
} __packed;

struct MADTRecordIOAPIC {
	MADTRecordHeader header;
	uint8_t ioapic_id;
	uint8_t reserved;
	uint32_t ioapic_address;
	uint32_t gsi_base;
} __packed;

struct MADTRecordInterruptSourceOverride {
	MADTRecordHeader header;
	uint8_t bus_source;
	uint8_t irq_source;
	uint32_t gsi;
	uint16_t flags;
} __packed;

struct MADT {
	SDTHeader header;
	uint32_t lca;
	uint32_t flags;
	MADTRecordHeader records;		// VARIABLE LENGTH
} __packed;

static RSDPDescriptor *__rsdp;
static uint32_t __ioapic_base;

/**
 * Scans memory for the RSDP by looking for the RSDP signature.  Returns a pointer to the RSDP descriptor, if it's
 * found.
 */
static RSDPDescriptor *scan_for_rsdp(uintptr_t start, uintptr_t end)
{
	// Align the starting address.
	uintptr_t cur = start & ~0xf;

	// Loop until we run out of memory, or the RSDP signature was detected.
	while (cur < end) {
		RSDPDescriptor *candidate = (RSDPDescriptor *)cur;
		
		// Check the signature, and if it matches, return the candidate.
		if (candidate->signature == RSDP_SIGNATURE) return candidate;
		
		cur += 16;
	}
	
	// The RSDP was not found.
	return NULL;
}

/**
 * Attempts to locate the RSDP in memory.
 */
static RSDPDescriptor *locate_rsdp()
{
	acpi_log.messagef(infos::kernel::LogLevel::DEBUG, "finding ebda...");
	
	virt_addr_t ebda = pa_to_kva(*(uint16_t *)(pa_to_kva(0x40E)) << 4);
	acpi_log.messagef(infos::kernel::LogLevel::DEBUG, "ebda=%p, locating rsdp descriptor...", ebda);
	
	RSDPDescriptor *ret = scan_for_rsdp((uintptr_t)ebda, (uintptr_t)ebda + 0x400);
	
	if (!ret) {
		ret = scan_for_rsdp((uintptr_t)pa_to_kva(0x000e0000), (uintptr_t)pa_to_kva(0x00100000));
	}
	
	acpi_log.messagef(infos::kernel::LogLevel::DEBUG, "rsdp=%p", ret);
	
	return ret;
}

/**
 * Performs an ACPI checksum on an arbitrary structure.
 */
static bool is_structure_valid(const void *structure_base, size_t structure_size)
{
	uint8_t sum = 0;
	for (unsigned int i = 0; i < structure_size; i++) {
		sum += ((uint8_t *)structure_base)[i];
	}
	
	return sum == 0;
}

/**
 * Parses an MADT LAPIC entry.
 */
static bool parse_madt_lapic(const MADTRecordLAPIC *lapic)
{
	acpi_log.messagef(infos::kernel::LogLevel::DEBUG, "madt: lapic: id=%u, procid=%u, flags=%x", lapic->apic_id, lapic->acpi_processor_id, lapic->flags);
	return true;
}

/**
 * Parses an MADT IOAPIC entry.
 */
static bool parse_madt_ioapic(const MADTRecordIOAPIC *ioapic)
{
	assert(!__ioapic_base);
	acpi_log.messagef(infos::kernel::LogLevel::DEBUG,
		"madt: ioapic: id=%u, addr=%p, gsi-base=%u",
		ioapic->ioapic_id,
		ioapic->ioapic_address,
		ioapic->gsi_base);
	__ioapic_base = ioapic->ioapic_address;

	return true;
}

/**
 * Parses an MADT ISO entry.
 */
static bool parse_madt_iso(const MADTRecordInterruptSourceOverride *iso)
{
	acpi_log.messagef(infos::kernel::LogLevel::DEBUG, "madt: iso: bus=%u, irq=%u, gsi=%u, flags=%x", iso->bus_source, iso->irq_source, iso->gsi, iso->flags);
	return true;
}

/**
 * Parses the MADT.
 */
static bool parse_madt(const MADT *madt)
{
	acpi_log.messagef(infos::kernel::LogLevel::DEBUG, "madt: lca=%08x, flags=%x", madt->lca, madt->flags);
	
	const MADTRecordHeader *rhs = &madt->records;
	const MADTRecordHeader *rhe = (const MADTRecordHeader *)((uintptr_t)madt + madt->header.length);
	
	while (rhs < rhe) {
		//acpi_log.messagef(infos::kernel::LogLevel::DEBUG, "madt: record: type=%u length=%u", rhs->type, rhs->length);
		
		switch (rhs->type) {
		case 0:
			if (!parse_madt_lapic((const MADTRecordLAPIC *)rhs)) {
				return false;
			}
			break;
			
		case 1:
			if (!parse_madt_ioapic((const MADTRecordIOAPIC *)rhs)) {
				return false;
			}
			break;

		case 2:
			if (!parse_madt_iso((const MADTRecordInterruptSourceOverride *)rhs)) {
				return false;
			}
			break;
			
		default:
			acpi_log.messagef(infos::kernel::LogLevel::WARNING, "madt: unsupported record type=%u, length=%u", rhs->type, rhs->length);
			break;
		}
		
		rhs = (const MADTRecordHeader *)((uintptr_t)rhs + rhs->length);
	}
	
	return true;
}

/**
 * Parses the ACPI tables.
 */
static bool acpi_parse()
{
	RSDT *rsdt = (RSDT *)pa_to_vpa(__rsdp->rsdt_address);
	acpi_log.messagef(infos::kernel::LogLevel::DEBUG, "parsing acpi tables rsdt=%p", rsdt);

	if (!rsdt) {
		acpi_log.messagef(infos::kernel::LogLevel::ERROR, "rsdt missing");
		return false;
	}
	
	if (!is_structure_valid(rsdt, rsdt->header.length)) {
		acpi_log.messagef(infos::kernel::LogLevel::ERROR, "acpi rsdt checksum invalid");
		return false;
	}
	
	const SDTHeader *hdr = NULL;
	for (unsigned int i = 0; i < (rsdt->header.length - sizeof(rsdt->header)) / sizeof(uint32_t); i++) {
		hdr = (const SDTHeader *)pa_to_vpa(rsdt->sdt_pointers[i]);
		
		if (!is_structure_valid(hdr, hdr->length)) {
			acpi_log.messagef(infos::kernel::LogLevel::ERROR, "skipping acpi table (invalid checksum)");
			continue;
		}
		
		switch (hdr->signature) {
		case MADT_SIGNATURE:
			if (!parse_madt((const MADT *)hdr)) {
				return false;
			}
			
			break;
		default:
			acpi_log.messagef(infos::kernel::LogLevel::WARNING, "unsupported acpi table: %08x", hdr->signature);
			break;
		}
	}
	
	return true;
}

/**
 * Initialises the ACPI subsystem
 */
bool infos::arch::x86::acpi::acpi_init()
{
	__rsdp = locate_rsdp();
	if (!__rsdp) {
		return false;
	}
		
	if (!is_structure_valid(__rsdp, sizeof(*__rsdp))) {
		acpi_log.messagef(infos::kernel::LogLevel::ERROR, "acpi rsdp checksum invalid");
		return false;
	}
	
	char oemid[7] = {0};
	strncpy(oemid, __rsdp->oem_id, 6);
	
	acpi_log.messagef(infos::kernel::LogLevel::INFO, "acpi version=%u oemid=%s rsdt=%p", __rsdp->revision, oemid, __rsdp->rsdt_address);
	
	if (__rsdp->revision > 2) {
		acpi_log.messagef(infos::kernel::LogLevel::ERROR, "unsupported acpi revision");
		return false;
	}
	
	return acpi_parse();
}

/**
 * Returns the base address of the IOAPIC.
 */
uint32_t infos::arch::x86::acpi::acpi_get_ioapic_base()
{
	return __ioapic_base;
}
