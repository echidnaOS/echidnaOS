ENTRY(main)

SECTIONS
{
  .text 0x10000 : { *(.text) }
  .data : { *(.data) *(.bss) *(COMMON) }
}
