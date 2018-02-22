OUTPUT_FORMAT(binary)
ENTRY(_start)

SECTIONS
{
    . = 1M;

    .text ALIGN(4K) :
    {
        sections_text = .;
        *(.multiboot)
        *(.startup)
        *(.text)
        sections_text_end = .;
    }

    .data ALIGN(4K) :
    {
        sections_data = .;
        *(.data)
        *(.rodata)
        initramfs = .;
        *(.initramfs)
        initramfs_end = .;
        initramfs_len = initramfs_end - initramfs;
        sections_data_end = .;
    }

    .bss ALIGN(4K) :
    {
        sections_bss = .;
        *(COMMON)
        *(.bss)
        sections_bss_end = .;
    }
}
