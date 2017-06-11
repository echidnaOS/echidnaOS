ENTRY(main)

SECTIONS
{
  .text 0x0 : { *(.text) }
  .data : { *(.data) }
}
