OUTPUT_FORMAT(binary)
ENTRY(kernel_init)

SECTIONS
{
  .text 0x100000 : {
    *(.startup)
    *(.text)
  }

  .data : {
    *(.rodata)
    *(.data)
    *(.bss)
    *(COMMON)
  }

  initramfs : {
    initramfs = .;
    *(initramfs)
    initramfs_end = .;
    initramfs_len = initramfs_end - initramfs;
  }
}
