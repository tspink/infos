ENTRY(multiboot_start)

SECTIONS
{
	. = 1M;
	_IMAGE_START = .;

	_STARTUP_START = .;
	.startup.header : AT(_STARTUP_START) { *(.multiboot.header) }

	. = ALIGN(16);

	.startup.text :
	{
		*(.multiboot.text)
		*(.startup.text32)
		*(.startup.text64)
	}

	.startup.data :
	{
		*(.startup.data)
	}

	_STARTUP_END = .;

	. = ALIGN(4096);
	_KERNEL_LMA_START = .;

	. = 0xFFFFFFFF80000000 + _KERNEL_LMA_START;
	_KERNEL_VMA_START = .;

	_TEXT_START = .;
	.text : AT(_KERNEL_LMA_START)
	{
		*(.text)
		*(.text.*)
	}
	_TEXT_END = .;

	. = ALIGN(4096);

	_RODATA_START = .;
	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
	}

	.roinitdata :
	{
		. = ALIGN(16);
		_CMDLINE_ARGS_START = .;
		KEEP(*(.cmdlineargs))
		_CMDLINE_ARGS_END = .;
	
		. = ALIGN(16);
		_VFS_REG_START = .;
		KEEP(*(.fsreg))
		_VFS_REG_END = .;

		. = ALIGN(16);
		_PGALLOC_PTR_START = .;
		KEEP(*(.pgallocptr))
		_PGALLOC_PTR_END = .;

		. = ALIGN(16);
		_SCHED_ALG_PTR_START = .;
		KEEP(*(.schedalg))
		_SCHED_ALG_PTR_END = .;

		. = ALIGN(16);
		_DEVICE_PTR_START = .;
		KEEP(*(.devctor))
		_DEVICE_PTR_END = .;
	}

	_RODATA_END = .;
	
	. = ALIGN(4096);

	_DATA_START = .;
	.data :
	{
		*(.data)

		. = ALIGN(16);

		__init_array_start = .;
		KEEP(*(.init_array*))
		__init_array_end = .;
	}
	_DATA_END = .;

	. = ALIGN(4096);

	_BSS_START = .;
	.bss : { *(.bss) }
	_BSS_END = .;

	. = ALIGN(4096);
	
	_STACK_START = .;
	. += 8192;
	_STACK_END = .;

	_STACK2_START = .;
	. += 8192;
	_STACK2_END = .;

	. = ALIGN(4096);
	_HEAP_START = .;

	_IMAGE_END = . - 0xFFFFFFFF80000000;

	/DISCARD/ :
	{
		*(.comment)
	}
}
