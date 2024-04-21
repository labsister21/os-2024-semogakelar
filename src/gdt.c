#include "header/cpu/gdt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, ...].
 */
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {
            // TODO : Implement
            .segment_low = 0,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0,
            .non_system = 0,
            .desc_privilege_lvl = 0,
            .present_bit = 0,
            .segment_mid = 0,
            .available = 0,
            .code_segment = 0,
            .default_op_size = 0,
            .granularity = 0,
            .base_high = 0
        },
        {
            // TODO : Implement
            .segment_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0xA,
            .non_system = 1,
            .desc_privilege_lvl = 0,
            .present_bit = 1,
            .segment_mid = 0xF,
            .available = 0,
            .code_segment = 0,
            .default_op_size = 1,
            .granularity = 1,
            .base_high = 0
        },
        {
            // TODO : Implement
            .segment_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0x2,
            .non_system = 1,
            .desc_privilege_lvl = 0,
            .present_bit = 1,
            .segment_mid = 0xF,
            .available = 0,
            .code_segment = 0,
            .default_op_size = 1,
            .granularity = 1,
            .base_high = 0
        },
        {
            // TODO : Implement
            .segment_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0x2,
            .non_system = 1,
            .desc_privilege_lvl = 0x3,
            .present_bit = 1,
            .segment_mid = 0xF,
            .available = 0,
            .code_segment = 0,
            .default_op_size = 1,
            .granularity = 1,
            .base_high = 0
        },
        {
            // TODO : Implement
            .segment_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0x2,
            .non_system = 1,
            .desc_privilege_lvl = 0x3,
            .present_bit = 1,
            .segment_mid = 0xF,
            .available = 0,
            .code_segment = 0,
            .default_op_size = 1,
            .granularity = 1,
            .base_high = 0
        },
        {
            .segment_mid       = (sizeof(struct TSSEntry) & (0xF << 16)) >> 16,
            .segment_low       = sizeof(struct TSSEntry),
            .base_high         = 0,
            .base_mid          = 0,
            .base_low          = 0,
            .non_system        = 0,    // S bit
            .type_bit          = 0x9,
            .desc_privilege_lvl        = 0,    // DPL
            .present_bit       = 1,    // P bit
            .default_op_size   = 1,    // D/B bit
            .code_segment      = 0,    // L bit
            .granularity       = 0,    // G bit
        },
        {0}
    }
};

/**
 * _gdt_gdtr, predefined system GDTR. 
 * GDT pointed by this variable is already set to point global_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */
struct GDTR _gdt_gdtr = {
    // TODO : Implement, this GDTR will point to global_descriptor_table. 
    //        Use sizeof operator
    .size = sizeof(global_descriptor_table) - 1,
    .address = &global_descriptor_table
};

void gdt_install_tss(void) {
    struct SegmentDescriptor* tss_descriptor = &(_gdt_gdtr.address->table[GDT_TSS_SELECTOR / sizeof(struct SegmentDescriptor)]);

    // Populate the TSS descriptor
    tss_descriptor->segment_low = sizeof(struct TSSEntry) & 0xFFFF;          // Limit (low 16 bits)
    tss_descriptor->base_low = (uint32_t)(&_interrupt_tss_entry) & 0xFFFF;    // Base address (low 16 bits)
    tss_descriptor->base_mid = ((uint32_t)(&_interrupt_tss_entry) >> 16) & 0xFF; // Base address (next 8 bits)
    tss_descriptor->type_bit = 0x9;                                           // Type (TSS descriptor)
    tss_descriptor->non_system = 0;                                           // System flag (0 for system descriptor)
    tss_descriptor->desc_privilege_lvl = 0;                                    // Descriptor Privilege Level (Ring 0)
    tss_descriptor->present_bit = 1;                                           // Present (1 for valid descriptor)
    tss_descriptor->segment_mid = ((sizeof(struct TSSEntry) >> 16) & 0x0F);     // Limit (next 4 bits)
    tss_descriptor->available = 0;                                             // Available for system use (set to 0)
    tss_descriptor->code_segment = 0;                                          // Not a code segment (set to 0)
    tss_descriptor->default_op_size = 0;                                       // 16-bit segment (set to 0)
    tss_descriptor->granularity = 0;                                           // Limit is in bytes (set to 0)
    tss_descriptor->base_high = ((uint32_t)(&_interrupt_tss_entry) >> 24) & 0xFF; // Base address (high 8 bits)

    // Load Task Register (TR) with TSS selector
    asm volatile ("ltr %w0" :: "r"(GDT_TSS_SELECTOR));
}