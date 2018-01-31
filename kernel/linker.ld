OUTPUT_FORMAT(binary)
ENTRY(kernel_init)

SECTIONS
{
  .text 0x100000 : { *(.text) }
  .data : { *(.rodata) *(.data) *(.bss) *(COMMON) }
}
