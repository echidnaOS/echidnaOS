#include <stdint.h>
#include <kernel.h>

#define BIOS_DRIVES_START 0x80
#define BIOS_DRIVES_MAX 4
#define BYTES_PER_SECT 512

static int cache_status = 0;
static char cached_drive;
static uint32_t cached_sector;
static uint8_t disk_cache[BYTES_PER_SECT];

char *bios_harddrive_names[] = {
    "bhda", "bhdb", "bhdc", "bhdd",
    "bhde", "bhdf", "bhdg", "bhdh",
    "bhdi", "bhdj", "bhdk", "bhdl",
    "bhdm", "bhdn", "bhdo", "bhdp",
    "bhdq", "bhdr", "bhds", "bhdt",
    "bhdu", "bhdv", "bhdw", "bhdx",
    "bhdy", "bhdz"
};

uint8_t bios_harddisk_read(uint8_t drive, uint64_t loc);
uint8_t bios_harddisks_io_wrapper(uint32_t disk, uint64_t loc, int type, uint8_t payload);

// kernel_add_device adds an io wrapper to the entries in the dev list
// arg 0 is a char* pointing to the name of the new device
// arg 1 is the general purpose value for the device (which gets passed to the wrapper when called)
// arg 2 is a pointer to the standard io wrapper function

void init_bios_harddisks(void) {    
    kputs("\nInitialising BIOS hard disks...");
    
    for (int i = BIOS_DRIVES_START; i < (BIOS_DRIVES_START + BIOS_DRIVES_MAX); i++) {
        kernel_add_device(bios_harddrive_names[i - BIOS_DRIVES_START], i, &bios_harddisks_io_wrapper);
        kputs("\nLoaded "); kputs(bios_harddrive_names[i - BIOS_DRIVES_START]);
    }

    return;
}

// standard kernel io wrapper expects
// arg 0 as a uint32_t being a general purpose value
// arg 1 as a uint64_t being the location for the i/o access
// arg 2 as an int, qualifing the type of access, being read (0) or write (1)
// arg 3 as a uint8_t being the byte to be written (only used when writing)
// the return value is a uint8_t, and returns the read byte, when reading
// when writing it should return 0

uint8_t bios_harddisks_io_wrapper(uint32_t disk, uint64_t loc, int type, uint8_t payload) {
    if (type == 0)
        return bios_harddisk_read((uint8_t)disk, loc);
    else if (type == 1) {
        //bios_harddisk_write((uint8_t)disk, loc, payload);
        return 0;
    }
}

uint8_t bios_harddisk_read(uint8_t drive, uint64_t loc) {
    uint64_t sect = loc / BYTES_PER_SECT;
    uint16_t offset = loc % BYTES_PER_SECT;

    if ((sect == cached_sector) && (drive == cached_drive) && (cache_status))
        return disk_cache[offset];

    disk_load_sector(drive, disk_cache, sect);
    cached_drive = drive;
    cached_sector = sect;
    cache_status = 1;
    
    return disk_cache[offset];
}
