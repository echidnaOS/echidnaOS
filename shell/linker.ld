ENTRY(main)

SECTIONS
{
  .text 0x1000000 : { *(.text) }
  .data : { *(.data) }
}
