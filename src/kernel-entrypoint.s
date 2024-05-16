global loader                        ; the entry symbol for ELF
global load_gdt                      ; load GDT table
global set_tss_register              ; set tss register to GDT entry
global kernel_execute_user_program
global process_context_switch
extern kernel_setup                  ; kernel C entrypoint
extern _paging_kernel_page_directory ; kernel page directory

; Define offsets for CPURegister and Context structures
%define CPU_INDEX_EDI_OFFSET 0
%define CPU_INDEX_ESI_OFFSET 4
%define CPU_STACK_EBP_OFFSET 8
%define CPU_STACK_ESP_OFFSET 12
%define CPU_GENERAL_EBX_OFFSET 16
%define CPU_GENERAL_EDX_OFFSET 20
%define CPU_GENERAL_ECX_OFFSET 24
%define CPU_GENERAL_EAX_OFFSET 28
%define CPU_SEGMENT_GS_OFFSET 32
%define CPU_SEGMENT_FS_OFFSET 36
%define CPU_SEGMENT_ES_OFFSET 40
%define CPU_SEGMENT_DS_OFFSET 44

%define CONTEXT_EIP_OFFSET 48
%define CONTEXT_EFLAGS_OFFSET 52
%define CONTEXT_PAGE_DIR_OFFSET 56

KERNEL_VIRTUAL_BASE equ 0xC0000000    ; kernel virtual memory
KERNEL_STACK_SIZE   equ 2097152       ; size of stack in bytes
MAGIC_NUMBER        equ 0x1BADB002    ; define the magic number constant
FLAGS               equ 0x0           ; multiboot flags
CHECKSUM            equ -MAGIC_NUMBER ; calculate the checksum (magic number + checksum + flags == 0)


section .bss
align 4                    ; align at 4 bytes
kernel_stack:              ; label points to beginning of memory
    resb KERNEL_STACK_SIZE ; reserve stack for the kernel


section .multiboot  ; GRUB multiboot header
align 4             ; the code must be 4 byte aligned
    dd MAGIC_NUMBER ; write the magic number to the machine code,
    dd FLAGS        ; the flags,
    dd CHECKSUM     ; and the checksum

; start of the text (code) section
section .setup.text 
loader equ (loader_entrypoint - KERNEL_VIRTUAL_BASE)
loader_entrypoint:         ; the loader label (defined as entry point in linker script)
    ; Set CR3 (CPU page register)
    mov eax, _paging_kernel_page_directory - KERNEL_VIRTUAL_BASE
    mov cr3, eax

    ; Use 4 MB paging
    mov eax, cr4
    or  eax, 0x00000010    ; PSE (4 MB paging)
    mov cr4, eax

    ; Enable paging
    mov eax, cr0
    or  eax, 0x80000000    ; PG flag
    mov cr0, eax

    ; Jump into higher half first, cannot use C because call stack is still not working
    lea eax, [loader_virtual]
    jmp eax

loader_virtual:
    mov dword [_paging_kernel_page_directory], 0
    invlpg [0]                                ; Delete identity mapping and invalidate TLB cache for first page
    mov esp, kernel_stack + KERNEL_STACK_SIZE ; Setup stack register to proper location
    call kernel_setup
.loop:
    jmp .loop                                 ; loop forever


section .text
; More details: https://en.wikibooks.org/wiki/X86_Assembly/Protected_Mode
load_gdt:
    cli
    mov  eax, [esp+4]
    lgdt [eax] ; Load GDT from GDTDescriptor, eax at this line will point GDTR location
    
    ; Set bit-0 (Protection Enable bit-flag) in Control Register 0 (CR0)
    ; This is optional, as usually GRUB already start with protected mode flag enabled
    mov  eax, cr0
    or   eax, 1
    mov  cr0, eax

    ; Far jump to update cs register
    ; Warning: Invalid GDT will raise exception in any instruction below
    jmp 0x8:flush_cs
flush_cs:
    ; Update all segment register
    mov ax, 10h
    mov ss, ax
    mov ds, ax
    mov es, ax
    ret
kernel_execute_user_program:
    mov  eax, 0x20 | 0x3
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax
    
    ; Using iret (return instruction for interrupt) technique for privilege change
    ; Stack values will be loaded into these register:
    ; [esp] -> eip, [esp+4] -> cs, [esp+8] -> eflags, [] -> user esp, [] -> user ss
    mov  ecx, [esp+4] ; Save first (before pushing anything to stack) for last push
    push eax ; Stack segment selector (GDT_USER_DATA_SELECTOR), user privilege
    mov  eax, ecx
    add  eax, 0x400000 - 4
    push eax ; User space stack pointer (esp), move it into last 4 MiB
    pushf    ; eflags register state, when jump inside user program
    mov  eax, 0x18 | 0x3
    push eax ; Code segment selector (GDT_USER_CODE_SELECTOR), user privilege
    mov  eax, ecx
    push eax ; eip register to jump back

    mov ebp, 0xBFFFFFFC
    mov esp, 0xBFFFFFFC
    
    iret
set_tss_register:
    mov ax, 0x28 | 0 ; GDT TSS Selector, ring 0
    ltr ax
    ret

process_context_switch:
    ; Save base address of function argument (ctx)
    mov eax, [esp + 4]         ; ctx is the first argument, contains pointer to current context

    ; Save all CPU registers to the current context
    mov [eax + CPU_INDEX_EDI_OFFSET], edi    ; Save edi
    mov [eax + CPU_INDEX_ESI_OFFSET], esi    ; Save esi
    mov [eax + CPU_STACK_EBP_OFFSET], ebp    ; Save ebp
    mov [eax + CPU_STACK_ESP_OFFSET], esp    ; Save esp
    mov [eax + CPU_GENERAL_EBX_OFFSET], ebx  ; Save ebx
    mov [eax + CPU_GENERAL_EDX_OFFSET], edx  ; Save edx
    mov [eax + CPU_GENERAL_ECX_OFFSET], ecx  ; Save ecx
    mov [eax + CPU_GENERAL_EAX_OFFSET], eax  ; Save eax
    mov [eax + CPU_SEGMENT_GS_OFFSET], gs    ; Save gs
    mov [eax + CPU_SEGMENT_FS_OFFSET], fs    ; Save fs
    mov [eax + CPU_SEGMENT_ES_OFFSET], es    ; Save es
    mov [eax + CPU_SEGMENT_DS_OFFSET], ds    ; Save ds

    ; Save eflags
    pushfd                           ; Push eflags
    pop dword [eax + CONTEXT_EFLAGS_OFFSET] ; Save eflags to context

    ; Get eip (instruction pointer)
    call next_instruction            ; Get the address of next instruction
next_instruction:
    pop dword [eax + CONTEXT_EIP_OFFSET]    ; Save eip to context

    ; Save page directory virtual address
    mov [eax + CONTEXT_PAGE_DIR_OFFSET], ecx  ; Assuming ecx holds the page directory virtual address

    ; Load the new process context
    mov eax, [esp + 8]         ; ctx is the second argument, contains pointer to new context

    ; Restore CPU registers from the new context
    mov edi, [eax + CPU_INDEX_EDI_OFFSET]    ; Restore edi
    mov esi, [eax + CPU_INDEX_ESI_OFFSET]    ; Restore esi
    mov ebp, [eax + CPU_STACK_EBP_OFFSET]    ; Restore ebp
    mov esp, [eax + CPU_STACK_ESP_OFFSET]    ; Restore esp
    mov ebx, [eax + CPU_GENERAL_EBX_OFFSET]  ; Restore ebx
    mov edx, [eax + CPU_GENERAL_EDX_OFFSET]  ; Restore edx
    mov ecx, [eax + CPU_GENERAL_ECX_OFFSET]  ; Restore ecx
    mov eax, [eax + CPU_GENERAL_EAX_OFFSET]  ; Restore eax
    mov gs, [eax + CPU_SEGMENT_GS_OFFSET]    ; Restore gs
    mov fs, [eax + CPU_SEGMENT_FS_OFFSET]    ; Restore fs
    mov es, [eax + CPU_SEGMENT_ES_OFFSET]    ; Restore es
    mov ds, [eax + CPU_SEGMENT_DS_OFFSET]    ; Restore ds

    ; Restore eflags
    pop eax                            ; Pop eflags
    push eax                           ; Push eflags back to stack
    popfd                              ; Restore eflags from stack

    ; Restore eip
    mov eax, [eax + CONTEXT_EIP_OFFSET]  ; Get eip from context
    jmp eax                              ; Jump to the new eip

    ret