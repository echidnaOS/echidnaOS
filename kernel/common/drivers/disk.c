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
    x++;

    return;
}

uint8_t disk_read_b(uint8_t drive, uint32_t loc) {
    uint32_t sect = loc / BYTES_PER_SECT;
    uint16_t offset = loc % BYTES_PER_SECT;

    if ((sect == cached_sector) && (drive == cached_drive) && (cache_status))
        return disk_cache[offset];

    disk_load_sector(drive_tab[drive], disk_cache, sect, 1);
    cached_drive = drive;
    cached_sector = sect;
    cache_status = 1;
    
    return disk_cache[offset];
}
