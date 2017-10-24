cat <<EOF

ENTRY(_start)

SECTIONS
{
  .text 0x10000 : { *(.text) }
  .data : { *(.rodata) *(.data) *(.bss) *(COMMON) }
}

EOF
