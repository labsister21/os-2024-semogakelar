#include "header/interrupt/idt.h"
#include "header/cpu/gdt.h"

void initialize_idt(void) {
    /* 
     * TODO: 
     * Iterate all isr_stub_table,
     * Set all IDT entry with set_interrupt_gate()
     * with following values:
     * Vector: i
     * Handler Address: isr_stub_table[i]
     * Segment: GDT_KERNEL_CODE_SEGMENT_SELECTOR
     * Privilege: 0
     */
    for (int i = 0; i < IDT_MAX_ENTRY_COUNT; i++) {
        set_interrupt_gate(i, isr_stub_table[i], GDT_KERNEL_CODE_SEGMENT_SELECTOR, 0);
    }
    __asm__ volatile("lidt %0" : : "m"(_idt_idtr));
    __asm__ volatile("sti");
}

struct InterruptDescriptorTable interrupt_descriptor_table = {
    .table = {
        {
            // TODO : Implement
            .offset_low = 0,
            .segment = 0,
            ._reserved = 0,
            ._r_bit_1 = 0,
            ._r_bit_2 = 0,
            ._r_bit_3 = 0,
            .gate_32 = 0,
            .dpl = 0,
            .valid_bit = 0,
            .offset_high = 0,
        }
    }
};

/**
 * _gdt_gdtr, predefined system GDTR. 
 * GDT pointed by this variable is already set to point interrupt_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */

struct IDTR _idt_idtr = {
    .size = sizeof(interrupt_descriptor_table) - 1,
    .address = &interrupt_descriptor_table
};

void set_interrupt_gate(
    uint8_t  int_vector, 
    void     *handler_address, 
    uint16_t gdt_seg_selector, 
    uint8_t  privilege
) {
    struct IDTGate *idt_int_gate = &interrupt_descriptor_table.table[int_vector];
    // TODO : Set handler offset, privilege & segment
    // Use &-bitmask, bitshift, and casting for offset
    // Target system 32-bit and flag this as valid interrupt gate
    uint32_t offset_handler = (uint32_t)handler_address;
    idt_int_gate->offset_low  = (uint32_t)(offset_handler & 0xFFFF);
    idt_int_gate->offset_high = (uint32_t)((offset_handler >> 16) & 0xFFFF);
    idt_int_gate->segment = gdt_seg_selector;
    idt_int_gate->dpl = privilege;

    // idt_int_gate->_reserved = 0;
    idt_int_gate->_r_bit_1    = INTERRUPT_GATE_R_BIT_1;
    idt_int_gate->_r_bit_2    = INTERRUPT_GATE_R_BIT_2;
    idt_int_gate->_r_bit_3    = INTERRUPT_GATE_R_BIT_3;
    idt_int_gate->gate_32     = 1;
    idt_int_gate->valid_bit   = 1;
}

