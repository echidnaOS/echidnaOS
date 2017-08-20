ENTRY(kernel_init)

SECTIONS
{
  .text 0x100000 : { *(.text) }
  .data : { *(.data) *(.bss) *(COMMON) }
}
