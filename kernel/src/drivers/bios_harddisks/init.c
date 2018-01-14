#include <stdint.h>
#include <kernel.h>

typedef struct {
    uint8_t drive;
    uint32_t cyl_count;
    uint32_t head_count;
    uint32_t sect_per_track;
    uint64_t sect_count;
} drive_parameters_t;

void disk_load_sector(uint8_t, uint8_t*, uint64_t);
void disk_write_sector(uint8_t, uint8_t*, uint64_t);
void read_drive_parameters(drive_parameters_t* drive_parameters);

#define BIOS_DRIVES_START 0x80
#define BIOS_DRIVES_MAX 26
#define BIOS_DRIVES_LIMIT 0xfe
#define BYTES_PER_SECT 512
#define CACHE_SECTS 64
#define CACHE_SIZE (BYTES_PER_SECT * CACHE_SECTS)

#define CACHE_NOT_READY 0
#define CACHE_READY 1
#define CACHE_DIRTY 2

static int cache_status = 0;
static uint8_t cached_drive;
static uint64_t cached_block;
static uint8_t disk_cache[CACHE_SIZE];

static char* bios_harddrive_names[] = {
    "bda", "bdb", "bdc", "bdd",
    "bde", "bdf", "bdg", "bdh",
    "bdi", "bdj", "bdk", "bdl",
    "bdm", "bdn", "bdo", "bdp",
    "bdq", "bdr", "bds", "bdt",
    "bdu", "bdv", "bdw", "bdx",
    "bdy", "bdz"
};

uint8_t bios_harddisk_read(uint8_t drive, uint64_t loc);
int bios_harddisk_write(uint8_t drive, uint64_t loc, uint8_t payload);
int bios_harddisks_io_wrapper(uint32_t disk, uint64_t loc, int type, uint8_t payload);

// kernel_add_device adds an io wrapper to the entries in the dev list
// arg 0 is a char* pointing to the name of the new device
// arg 1 is the general purpose value for the device (which gets passed to the wrapper when called)
// arg 2 is a pointer to the standard io wrapper function

void init_bios_harddisks(void) {    
    kputs("\nInitialising BIOS hard disks...");

    drive_parameters_t drive_parameters;

    int j = BIOS_DRIVES_START;
    for (int i = 0; i < BIOS_DRIVES_MAX; i++) {
        for ( ; j < BIOS_DRIVES_LIMIT; j++) {
            if (j == 0xe0)      // ignore BIOS CD
                continue;
            drive_parameters.drive = j;
            read_drive_parameters(&drive_parameters);
            if (!drive_parameters.sect_count)
                continue;
            else
                goto found;
        }
        // limit exceeded, return
        return;
found:
        kputs("\nBIOS drive:         "); kxtoa(drive_parameters.drive);
        kputs("\nCylinder count:     "); kuitoa(drive_parameters.cyl_count);
        kputs("\nHead count:         "); kuitoa(drive_parameters.head_count);
        kputs("\nSect per track:     "); kuitoa(drive_parameters.sect_per_track);
        kputs("\nSector count:       "); kuitoa(drive_parameters.sect_count);
        kernel_add_device(bios_harddrive_names[i], j, drive_parameters.sect_count * BYTES_PER_SECT, &bios_harddisks_io_wrapper);
        kputs("\nLoaded "); kputs(bios_harddrive_names[i]);

        j++;
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

int bios_harddisks_io_wrapper(uint32_t disk, uint64_t loc, int type, uint8_t payload) {
    if (type == 0) {
        return bios_harddisk_read((uint8_t)disk, loc);
    } else if (type == 1) {
        return bios_harddisk_write((uint8_t)disk, loc, payload);
    }
}

uint8_t bios_harddisk_read(uint8_t drive, uint64_t loc) {
    uint64_t block = loc / CACHE_SIZE;
    uint32_t offset = loc % CACHE_SIZE;

    if ((block == cached_block) && (drive == cached_drive) && (cache_status))
        return disk_cache[offset];

    // cache miss
    // flush cache if dirty
    if (cache_status == CACHE_DIRTY) {
        DISABLE_INTERRUPTS;
        disk_write_sector(cached_drive, disk_cache, cached_block * CACHE_SECTS);
        ENABLE_INTERRUPTS;
    }

    DISABLE_INTERRUPTS;
    disk_load_sector(drive, disk_cache, block * CACHE_SECTS);
    ENABLE_INTERRUPTS;
    cached_drive = drive;
    cached_block = block;
    cache_status = 1;
    
    return disk_cache[offset];
}

int bios_harddisk_write(uint8_t drive, uint64_t loc, uint8_t payload) {
    uint64_t block = loc / CACHE_SIZE;
    uint32_t offset = loc % CACHE_SIZE;

    if ((block == cached_block) && (drive == cached_drive) && (cache_status)) {
        disk_cache[offset] = payload;
        cache_status = CACHE_DIRTY;
        return 0;
    }

    // cache miss
    // flush cache if dirty
    if (cache_status == CACHE_DIRTY) {
        DISABLE_INTERRUPTS;
        disk_write_sector(cached_drive, disk_cache, cached_block * CACHE_SECTS);
        ENABLE_INTERRUPTS;
    }

    DISABLE_INTERRUPTS;
    disk_load_sector(drive, disk_cache, block * CACHE_SECTS);
    ENABLE_INTERRUPTS;
    cached_drive = drive;
    cached_block = block;
    
    disk_cache[offset] = payload;
    cache_status = CACHE_DIRTY;
    return 0;
}
