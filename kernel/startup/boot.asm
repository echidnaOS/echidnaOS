extern startup
extern sections_text
extern sections_data_end
extern sections_bss_end
global _start

section .multiboot
legacy_skip_header:
    jmp _start

align 4
multiboot_header:
    .magic dd 0x1BADB002
    .flags dd 0x00010000
    .checksum dd -(0x1BADB002 + 0x00010000)
    .header_addr dd multiboot_header
    .load_addr dd sections_text
    .load_end_addr dd sections_data_end
    .bss_end_addr dd sections_bss_end
    .entry_addr dd _start

section .text
_start:
    mov esp, 0xeffff0

    jmp startup

    cli
    hlt
