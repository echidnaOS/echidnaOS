#include <stdint.h>
#include <kernel.h>
#include <cio.h>

#define DEVICE_COUNT 4
#define BYTES_PER_SECT 512

typedef struct {
    uint8_t master; // this is a boolean
    
    uint16_t data_port;
    uint16_t error_port;
    uint16_t sector_count_port;
    uint16_t lba_low_port;
    uint16_t lba_mid_port;
    uint16_t lba_hi_port;
    uint16_t device_port;
    uint16_t command_port;
    uint16_t control_port;
    
    uint8_t exists;
    
    uint16_t bytes_per_sector;
    
    uint8_t cache[BYTES_PER_SECT];
    uint32_t cached_sector;
    int cache_status;
} ata_device;

char* ata_names[] = {
    "hda", "hdb", "hdc", "hdd",
    "hde", "hdf", "hdg", "hdh",
    "hdi", "hdj", "hdk", "hdl",
    "hdm", "hdn", "hdo", "hdp",
    "hdq", "hdr", "hds", "hdt",
    "hdu", "hdv", "hdw", "hdx",
    "hdy", "hdz"
};

uint16_t ata_ports[] = { 0x1f0, 0x1f0, 0x170, 0x170 };
int max_ports = 4;

uint8_t ata_io_wrapper(uint32_t disk, uint64_t loc, int type, uint8_t payload);
uint8_t ata_read_byte(uint32_t drive, uint64_t loc);
ata_device init_ata_device(uint16_t port_base, uint8_t master);
void ata_identify(ata_device* dev);
void ata_read(uint32_t disk, uint32_t sector, uint8_t* buffer);
void ata_write(uint32_t disk, uint32_t sector, uint8_t* data);

ata_device devices[DEVICE_COUNT];

uint8_t ata_io_wrapper(uint32_t disk, uint64_t loc, int type, uint8_t payload) {
    if (type == 0)
        return ata_read_byte(disk, loc);
    else if (type == 1) {
        return 0;
    }
}

uint8_t ata_read_byte(uint32_t drive, uint64_t loc) {
    uint64_t sect = loc / BYTES_PER_SECT;
    uint16_t offset = loc % BYTES_PER_SECT;

    if (((uint32_t)sect == devices[drive].cached_sector) && (devices[drive].cache_status))
        return devices[drive].cache[offset];

    ata_read(drive, (uint32_t)sect, devices[drive].cache);
    devices[drive].cached_sector = (uint32_t)sect;
    devices[drive].cache_status = 1;
    
    return devices[drive].cache[offset];
}

void init_ata(void) {
    kputs("\nInitialising ATA device driver...");
    
    int ii = 0;
    int master = 1;
    for (int i = 0; i < DEVICE_COUNT; i++) {
        while (!(devices[i] = init_ata_device(ata_ports[ii], master)).exists) {
            ii++;
            if (ii >= max_ports) return;
            if (ii % 2) master = 0;
            else master = 1;
        }
        ii++;
        if (ii >= max_ports) return;
        if (ii % 2) master = 0;
        else master = 1;
        kernel_add_device(ata_names[i], i, &ata_io_wrapper);
        kputs("\nInitialised "); kputs(ata_names[i]);
    }

    return;
}

ata_device init_ata_device(uint16_t port_base, uint8_t master) {
    ata_device dev;
    
    dev.data_port = port_base;
    dev.error_port = port_base + 0x01;
    dev.sector_count_port = port_base + 0x02;
    dev.lba_low_port = port_base + 0x03;
    dev.lba_mid_port = port_base + 0x04;
    dev.lba_hi_port = port_base + 0x05;
    dev.device_port = port_base + 0x06;
    dev.command_port = port_base + 0x07;
    dev.control_port = port_base + 0x206;
    dev.exists = 0;
    dev.master = master;
    
    dev.bytes_per_sector = 512;
    
    dev.cache_status = 0;
    
    ata_identify(&dev);
    
    return dev;
}

void ata_identify(ata_device* dev) {

    port_out_b(dev->device_port, dev->master ? 0xA0 : 0xB0);
    //port_out_b(dev.control_port, 0);
    
    uint8_t status = port_in_b(dev->command_port);
    if (status == 0xFF) {
        dev->exists = 0;
        kputs("\nNo device found!");
        return;
    }
        
    port_out_b(dev->sector_count_port, 0);
    port_out_b(dev->lba_low_port, 0);
    port_out_b(dev->lba_mid_port, 0);
    port_out_b(dev->lba_hi_port, 0);
    
    port_out_b(dev->command_port, 0xEC); // identify command

    status = port_in_b(dev->command_port);
    
    if (status == 0x00) {
        dev->exists = 0;
        kputs("\nNo device found!");
        return;
    }
    
    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01)) 
        status = port_in_b(dev->command_port);
    
    if (status & 0x01) {
        dev->exists = 0;
        kputs("\nError occured!");
        return;
    }
    
    kputs("\nDevice successfully identified!");
    
    dev->exists = 1;
    
    return;
}

void ata_read(uint32_t disk, uint32_t sector, uint8_t* buffer) {
    ata_device dev = devices[disk];

    if (sector > 0x0FFFFFFF)
        return;

    port_out_b(dev.device_port, (dev.master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    port_out_b(dev.error_port, 0);
    
    port_out_b(dev.sector_count_port, 1);
    port_out_b(dev.lba_low_port, sector & 0x000000FF);
    port_out_b(dev.lba_mid_port, (sector & 0x0000FF00) >> 8);
    port_out_b(dev.lba_hi_port, (sector & 0x00FF0000) >> 16);
    
    port_out_b(dev.command_port, 0x20); // read command
 
    uint8_t status = port_in_b(dev.command_port);
    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01)) 
        status = port_in_b(dev.command_port);
    
    if (status & 0x01) {
        kputs("\nError occured!");
        return;
    }
    
    for(int i = 0; i < 256; i ++) {
        uint16_t wdata = port_in_w(dev.data_port);
        
        int c = i * 2;
        buffer[c] = wdata & 0xFF;
        buffer[c + 1] = (wdata >> 8) & 0xFF;  
    }
    
    return;
}

void ata_write(uint32_t disk, uint32_t sector, uint8_t* data) {
    ata_device dev = devices[disk];
    
    if (sector > 0x0FFFFFFF)
        return;

    port_out_b(dev.device_port, (dev.master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    port_out_b(dev.error_port, 0);
    
    port_out_b(dev.sector_count_port, 1);
    port_out_b(dev.lba_low_port, sector & 0x000000FF);
    port_out_b(dev.lba_mid_port, (sector & 0x0000FF00) >> 8);
    port_out_b(dev.lba_hi_port, (sector & 0x00FF0000) >> 16);
    
    port_out_b(dev.command_port, 0x30); // identify command
        
    for(int i = 0; i < 256; i ++) {
        int c = i * 2;
        
        uint16_t wdata = (data[c + 1] << 8)| data[c];
        
        port_out_w(dev.data_port, wdata);
    }
    
    return;    
}
