.text

// TIMER = 0x0320
// TICR = 0x0380, // Timer Initial Count
// TCCR = 0x0390, // Timer Current Count
// TDCR = 0x03E0, // Timer Divide Configuration

.align 16

.global lapic_fast_calibrate
.type lapic_fast_calibrate,%function
lapic_fast_calibrate:
    // RDI contains APIC base

    // Clear periodic, set IRQ=32
    movl 0x320(%rdi), %esi
    or $0x10000, %esi
    movl $0x20, 0x320(%rdi)

    // Set TICR 1
    movl $1, 0x380(%rdi)

    // Set TDCR 3
    movl $3, 0x3e0(%rdi)

    // Init PIT
    xor %eax, %eax

    in $0x61, %al
    and $0x0c, %eax
    or $1, %eax
    out %al, $0x61

    mov $0xb0, %eax
    out %al, $0x43

    // Timeout 0x2e9b
    mov $0x2e9b, %eax
    out %al, $0x42
    rol $8, %ax
    out %al, $0x42

    // Enable PIT
    in $0x61, %al
    and $0x0c, %eax
    out %al, $0x61             // OFF
    or $1, %eax
    out %al, $0x61             // ON

    // Set TICR = 0xffffffff, and start LAPIC
    movl $0xffffffff, 0x380(%rdi)

    // Wait for PIT
1:
    in $0x61, %al
    test $0x20, %al
    jz 1b

    // Stop LAPIC
    mov 0x390(%rdi), %eax
    mov %esi, 0x320(%rdi)

    ret
.size lapic_fast_calibrate,.-lapic_fast_calibrate
