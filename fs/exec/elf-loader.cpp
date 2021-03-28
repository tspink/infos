/*
 * fs/exec/elf-loader.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include "infos/define.h"
#include <infos/fs/exec/elf-loader.h>
#include <infos/fs/file.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/process.h>
#include <infos/mm/mm.h>
#include <infos/mm/page-allocator.h>

using namespace infos::kernel;
using namespace infos::fs;
using namespace infos::fs::exec;
using namespace infos::util;

ComponentLog infos::fs::exec::elf_log(syslog, "elf");

#define MAGIC_NUMBER	0x464c457f

#define ECLASS_32BIT	1
#define ECLASS_64BIT	2
#define EDATA_LITTLE	1
#define EDATA_BIG		2

struct ELF64Header
{
	struct {
		union {
			char magic_bytes[4];
			uint32_t magic_number;
		};
		
		uint8_t eclass, data, version, osabi, abiversion;
		uint8_t pad[7];
	} ident __packed;
	
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint64_t entry_point;
	uint64_t phoff;
	uint64_t shoff;
	uint32_t flags;
	uint16_t ehsize;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
} __packed;

struct ELF64ProgramHeaderEntry
{
	uint32_t type, flags;
	uint64_t offset, vaddr, paddr;
	uint64_t filesz, memsz, align;
};

struct ELF64SectionHeaderEntry
{
	uint32_t sh_name;
	uint32_t sh_type;
	uint64_t sh_flags;
	uint64_t sh_addr;
	uint64_t sh_offset;
	uint64_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint64_t sh_addralign;
	uint64_t sh_entsize;
};

ElfLoader::ElfLoader(File& f) : _file(f)
{

}

Process* ElfLoader::load(const String& cmdline)
{
	ELF64Header hdr;
	
	_file.seek(0, File::SeekAbsolute);
	int bytes = _file.read(&hdr, sizeof(hdr));
	
	if (bytes != sizeof(hdr)) {
		elf_log.messagef(LogLevel::DEBUG, "Unable to read ELF header");
		return NULL;
	}
	
	if (hdr.ident.magic_number != MAGIC_NUMBER) {
		elf_log.messagef(LogLevel::DEBUG, "Invalid ELF magic number %x", hdr.ident.magic_number);
		return NULL;
	}
	
	if (hdr.ident.eclass != ECLASS_64BIT) {
		elf_log.message(LogLevel::DEBUG, "Only 64-bit ELF programs are supported");
		return NULL;
	}
	
	if (hdr.ident.data != EDATA_LITTLE) {
		elf_log.message(LogLevel::DEBUG, "Only little-endian ELF programs are supported");
		return NULL;
	}
	
	if (hdr.type != 2) {
		elf_log.messagef(LogLevel::DEBUG, "Only executables can be loaded (%d)", hdr.type);
		return NULL;
	}

	if (hdr.machine != 0x3e) {
		elf_log.messagef(LogLevel::DEBUG, "Unsupported instruction set architecture (%d)", hdr.machine);
		return NULL;
	}
	
	if (hdr.version != 1) {
		elf_log.message(LogLevel::DEBUG, "Invalid ELF version");
		return NULL;
	}
	
	Process *np = new Process("unknown", false, (Thread::thread_proc_t)hdr.entry_point);
	for (unsigned int i = 0; i < hdr.phnum; i++) {
		ELF64ProgramHeaderEntry ent;
		
		off_t off = hdr.phoff + (i * hdr.phentsize);
		if (_file.pread(&ent, sizeof(ent), off) != sizeof(ent)) {
			delete np;
			
			elf_log.message(LogLevel::DEBUG, "Unable to read PH entry");
			return NULL;
		}
		
		if (ent.type == 1) {
			if (!np->vma().is_mapped(ent.vaddr)) {
				virt_addr_t base = __page_base(ent.vaddr);
				size_t size = __page_offset(ent.vaddr) + ent.memsz;
				np->vma().allocate_virt(base, __align_up_page(size) >> 12);
			}
			
			char *buffer = new char[ent.filesz];
			_file.pread(buffer, ent.filesz, ent.offset);
			np->vma().copy_to(ent.vaddr, buffer, ent.filesz);
			delete buffer;
		}
	}

	// Iterate over section headers
	for(unsigned int i = 0; i < hdr.shnum; i++) {
		ELF64SectionHeaderEntry section;
		off_t off = hdr.shoff + (i * hdr.shentsize);
		if (_file.pread(&section, sizeof(section), off) != sizeof(section)) {
			delete np;
			
			elf_log.message(LogLevel::DEBUG, "Unable to read SH entry");
			return NULL;
		}
 
		// SHT_NOBITS
		if(section.sh_type == 8) {
			// Skip if it the section is empty
			if(!section.sh_size) continue;
			// SHF_ALLOC
			if(section.sh_flags & 2) {
				// Allocate and zero some memory
				if (!np->vma().is_mapped(section.sh_addr)) {
					virt_addr_t base = __page_base(section.sh_addr);
					size_t size = __page_offset(section.sh_addr) + section.sh_size;
					np->vma().allocate_virt(base, __align_up_page(size) >> 12);
				}
			}
		}
	}
	
	np->main_thread().allocate_user_stack(0xffffd000, 0x2000);

	if (cmdline.length() > 0) {
		virt_addr_t cmdline_start = 0xfffff000;
		np->vma().allocate_virt(cmdline_start, 1);
		
		if (!np->vma().copy_to(cmdline_start, cmdline.c_str(), cmdline.length())) {
			return NULL;
		}
		
		np->main_thread().add_entry_argument((void *)cmdline_start);
	} else {
		np->main_thread().add_entry_argument(NULL);
	}
		
	return np;
}
