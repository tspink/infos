// --- 16 BITS --- //
.code16

.section .mpstart.text16, "ax"
__mpentry:
    jmp __mpstart16

.align 16
.type ___mpstart16,%function
__mpstart16:
    // Disable interrupts
    cli

    // If we've got this far, then the AP is online, so indicate this
    // back to the BSP, by setting the 'mpready' flag to one.
    mov $mpready, %eax
    sub $_MPSTARTUP_START, %eax
    movb $1, (%eax)

    // Prepare the AP temporary 32-bit GDT
    mov $mpgdt32p, %eax
    sub $_MPSTARTUP_START, %eax
    lgdt (%eax)

    // Enter protected mode
    mov %cr0, %eax
    or $1, %al
    mov %eax, %cr0

    // ljmp $0x08, $__mpstart32
    .byte   0x66, 0xea
    .long   __mpstart32
    .word   0x08
.size __mpstart16,.-__mpstart16

.section .mpstart.data, "aw"

.align 4
.type mpready, %object
.globl mpready
mpready:
    .byte 0
.size mpready,.-mpready

.align 4
.type mpgdt, %object
mpgdt32:
    .quad 0x0000000000000000		// NULL     0x00
    .quad 0x00cf9a000000ffff		// Code     0x08
    .quad 0x00cf92000000ffff		// Data     0x10
mpgdt32_end:
.size mpgdt32,.-mpgdt32

.align 4
.type mpgdt32p, %object
mpgdt32p:
    .word (mpgdt32_end - mpgdt32)
    .long mpgdt32

// --- 32 BITS --- //

.code32
.section .mpstart.text32, "ax"

.align 16
.globl __mpstart32
.type __mpstart32,%function
__mpstart32:
    // Reset segment registers
    mov $0x10,%eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss

    xor %eax, %eax
    mov %ax, %fs
    mov %ax, %gs

    // Reset flags
    mov $0x1000, %esp

    pushl $0
    popf

    // Reset IDT
    lidt mpidt32p

    // LOAD CR3
    mov $mpcr3, %eax
    sub $_MPSTARTUP_START, %eax
    mov (%eax), %eax
    mov %eax, %cr3

    // Disable PIC
    mov $0xff, %al
    out %al, $0xa1
    out %al, $0x21

    // CR4 := 011010110000
    mov $0x6b0, %eax
    mov %eax, %cr4

    // EFER :=
    mov $0xC0000080, %ecx
    rdmsr
    or $0x000000901, %eax
    wrmsr

    // CR0 := PG, PE, MP, EM, WP
    mov $0x80010007, %eax
    mov %eax, %cr0

    // Debugging code that prints an A
    // mov $0x41,%al
    // mov $0xe9,%dx
    // outb %al,%dx

    // Initialise the temporary GDT
    lgdt mpgdt64p

    // Long jump to 64-bit code
    ljmp $0x8, $__mpstart64

.size __mpstart32,.-__mpstart32

.section .mpstart.data, "aw"

.align 4
.type idtp, %object
mpidt32p:
	.word 0
	.long 0

.align 4
.type gdt, %object
mpgdt64:
    .quad 0x0000000000000000		// NULL
    .quad 0x00209A0000000000		// Code
    .quad 0x0000920000000000		// Data
mpgdt64_end:
.size mpgdt64,.-mpgdt64

.align 4
.type mpgdt64p, %object
mpgdt64p:
	.word (mpgdt64_end - mpgdt64)
	.long mpgdt64

.align 4
.globl mpcr3
.type mpcr3, %object
mpcr3:  .quad 0
.size mpcr3,.-mpcr3

// --- 64 BITS --- //

.code64
.section .mpstart.text64, "ax"

.align 16
.globl __mpstart64
.type __mpstart64,%function
__mpstart64:
    // Re-initialise segment registers
    mov $0x10, %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss

    xor %eax, %eax
    mov %ax, %fs
    mov %ax, %gs

    // Switch to the core stack
    mov $mpstack, %eax
    sub $_MPSTARTUP_START, %eax
    mov (%rax), %rax
    mov %rax, %rsp

    // Jump to the x86 initialisation entrypoint
    jmp x86_core_start
.size __mpstart64,.-__mpstart64

.section .mpstart.data, "aw"

.align 8
.globl mpstack
.type mpstack, %object
mpstack:  .quad 0
.size mpstack,.-mpstack