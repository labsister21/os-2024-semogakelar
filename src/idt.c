#include "header/cpu/idt.h"
#include "header/cpu/interrupt.h"
#include "header/cpu/gdt.h"

struct InterruptDescriptorTable interrupt_descriptor_table;

struct IDTR _idt_idtr = {
    .size = sizeof(interrupt_descriptor_table) - 1,
    .address = &interrupt_descriptor_table
};

void initialize_idt(void) {
    for (int i = 0; i < ISR_STUB_TABLE_LIMIT; i++) {
        if (i >= 0x30 && i <= 0x3F){
            set_interrupt_gate(i, isr_stub_table[i], GDT_KERNEL_CODE_SEGMENT_SELECTOR, 3);
        }else{
            set_interrupt_gate(i, isr_stub_table[i], GDT_KERNEL_CODE_SEGMENT_SELECTOR, 0);
        }
    }
    __asm__ volatile("lidt %0" : : "m"(_idt_idtr));
    __asm__ volatile("sti");
}

void set_interrupt_gate(
    uint8_t  int_vector, 
    void     *handler_address, 
    uint16_t gdt_seg_selector, 
    uint8_t  privilege
) {
    struct IDTGate *idt_int_gate = &interrupt_descriptor_table.table[int_vector];
    uint32_t offset_handler = (uint32_t)handler_address;
    idt_int_gate->offset_low = offset_handler & 0xFFFF;
    idt_int_gate->offset_high = offset_handler >> 16;
    idt_int_gate->desc_privilege_level = privilege;
    idt_int_gate->segment = gdt_seg_selector;
    idt_int_gate->_r_bit_1    = INTERRUPT_GATE_R_BIT_1;
    idt_int_gate->_r_bit_2    = INTERRUPT_GATE_R_BIT_2;
    idt_int_gate->_r_bit_3    = INTERRUPT_GATE_R_BIT_3;
    idt_int_gate->gate_32     = 1;
    idt_int_gate->valid_bit   = 1;
}