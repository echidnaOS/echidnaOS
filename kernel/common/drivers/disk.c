#include <stdint.h>
#include <kernel.h>

#define BYTES_PER_SECT 512

static uint8_t drive_tab[256];

static int cache_status = 0;
static char cached_drive;
static uint32_t cached_sector;
static uint8_t disk_cache[512];

void init_disk(uint8_t boot_drive) {
    uint8_t x = 0;

    kputs("\nBIOS drive "); kxtoa((uint32_t)boot_drive); kputs(" is the boot drive.");
    kputs("\nUsing boot drive as drive :"); kuitoa(x);
    drive_tab[x] = boot_drive;
    kputs("\nBIOS signature: "); kxtoa((uint32_t)disk_read_w(0, 510));
    x++;

    return;
}

uint8_t disk_read_b(uint8_t drive, uint64_t loc) {
    uint64_t sect = loc / BYTES_PER_SECT;
    uint16_t offset = loc % BYTES_PER_SECT;

    if ((sect == cached_sector) && (drive == cached_drive) && (cache_status))
        return disk_cache[offset];

    disk_load_sector(drive_tab[drive], disk_cache, sect);
    cached_drive = drive;
    cached_sector = sect;
    cache_status = 1;
    
    return disk_cache[offset];
}

uint16_t disk_read_w(uint8_t drive, uint64_t loc) {
    uint16_t x = 0;
    for (uint64_t i = 0; i < 2; i++)
        x += (uint16_t)disk_read_b(drive, loc++) * (uint16_t)(power(0x100, i));
    return x;
}

uint32_t disk_read_d(uint8_t drive, uint64_t loc) {
    uint32_t x = 0;
    for (uint64_t i = 0; i < 4; i++)
        x += (uint32_t)disk_read_b(drive, loc++) * (uint32_t)(power(0x100, i));
    return x;
}

uint64_t disk_read_q(uint8_t drive, uint64_t loc) {
    uint64_t x = 0;
    for (uint64_t i = 0; i < 8; i++)
        x += (uint64_t)disk_read_b(drive, loc++) * (uint64_t)(power(0x100, i));
    return x;
}

void disk_read_seq(uint8_t* buffer, uint8_t drive, uint32_t loc, uint32_t count) {
    uint32_t x;

    for (x = 0; x < count; x++)
        buffer[x] = disk_read_b(drive, loc++);

    return;
}
