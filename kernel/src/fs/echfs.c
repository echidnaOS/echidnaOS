#include <stdint.h>
#include <kernel.h>

#define SEARCH_FAILURE          0xffffffffffffffff
#define ROOT_ID                 0xffffffffffffffff
#define ENTRIES_PER_BLOCK       2
#define FILENAME_LEN            218
#define RESERVED_BLOCKS         16
#define BYTES_PER_BLOCK         512
#define FILE_TYPE               0
#define DIRECTORY_TYPE          1
#define DELETED_ENTRY           0xfffffffffffffffe
#define RESERVED_BLOCK          0xfffffffffffffff0
#define END_OF_CHAIN            0xffffffffffffffff

#define FAILURE -2
#define EOF -1
#define SUCCESS 0

char* device;
uint64_t blocks;
uint64_t fatsize;
uint64_t fatstart;
uint64_t dirsize;
uint64_t dirstart;
uint64_t datastart;

typedef struct {
    char name[128];
    uint64_t blocks;
    uint64_t fatsize;
    uint64_t fatstart;
    uint64_t dirsize;
    uint64_t dirstart;
    uint64_t datastart;
} mount_t;

mount_t* mounts;
int mounts_ptr = 0;

typedef struct {
    uint64_t parent_id;
    uint8_t type;
    char name[FILENAME_LEN];
    uint8_t perms;
    uint16_t owner;
    uint16_t group;
    uint8_t hundreths;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint64_t payload;
    uint64_t size;
} __attribute__((packed)) entry_t;

typedef struct {
    uint64_t target_entry;
    entry_t target;
    entry_t parent;
    char name[FILENAME_LEN];
    int failure;
    int not_found;
} path_result_t;

typedef struct {
    char path[2048];
    path_result_t path_result;
    uint64_t cached_block;
    uint8_t cache[BYTES_PER_BLOCK];
    int cache_status;
    uint64_t* alloc_map;
} cached_file_t;

cached_file_t* cached_files;
int cached_files_ptr = 0;

int find_device(char* dev) {
    int i;

    for (i = 0; kstrcmp(mounts[i].name, dev); i++);
    
    return i;
}

uint8_t rd_byte(uint64_t loc) {
    return (uint8_t)vfs_kread(device, loc);
}

uint16_t rd_word(uint64_t loc) {
    uint16_t x = 0;
    for (uint64_t i = 0; i < 2; i++)
        x += (uint16_t)vfs_kread(device, loc++) * (uint16_t)(power(0x100, i));
    return x;
}

uint32_t rd_dword(uint64_t loc) {
    uint32_t x = 0;
    for (uint64_t i = 0; i < 4; i++)
        x += (uint32_t)vfs_kread(device, loc++) * (uint32_t)(power(0x100, i));
    return x;
}

uint64_t rd_qword(uint64_t loc) {
    uint64_t x = 0;
    for (uint64_t i = 0; i < 8; i++)
        x += (uint64_t)vfs_kread(device, loc++) * (uint64_t)(power(0x100, i));
    return x;
}

void fstrcpy_in(char* str, uint64_t loc) {
    int i = 0;
    while (rd_byte(loc))
        str[i++] = (char)rd_byte(loc++);
    str[i] = 0;
    return;
}

entry_t rd_entry(uint64_t entry) {
    entry_t res;
    uint64_t loc = (dirstart * BYTES_PER_BLOCK) + (entry * sizeof(entry_t));

    res.parent_id = rd_qword(loc);
    loc += sizeof(uint64_t);
    res.type = rd_byte(loc++);
    fstrcpy_in(res.name, loc);
    loc += FILENAME_LEN;
    res.perms = rd_byte(loc++);
    res.owner = rd_word(loc);
    loc += sizeof(uint16_t);
    res.group = rd_word(loc);
    loc += sizeof(uint16_t);
    res.hundreths = rd_byte(loc++);
    res.seconds = rd_byte(loc++);
    res.minutes = rd_byte(loc++);
    res.hours = rd_byte(loc++);
    res.day = rd_byte(loc++);
    res.month = rd_byte(loc++);
    res.year = rd_word(loc);
    loc += sizeof(uint16_t);
    res.payload = rd_qword(loc);
    loc += sizeof(uint64_t);
    res.size = rd_qword(loc);
    
    return res;
}

int fstrcmp(uint64_t loc, const char* str) {
    for (int i = 0; (rd_byte(loc) || str[i]); i++)
        if ((char)rd_byte(loc++) != str[i]) return 0;
    return 1;
}

int fstrncmp(uint64_t loc, const char* str, int len) {
    for (int i = 0; i < len; i++)
        if ((char)rd_byte(loc++) != str[i]) return 0;
    return 1;
}

uint64_t search(char* name, uint64_t parent, uint8_t type) {
    entry_t entry;
    // returns unique entry #, SEARCH_FAILURE upon failure/not found
    for (uint64_t i = 0; ; i++) {
        entry = rd_entry(i);
        if (!entry.parent_id) return SEARCH_FAILURE;              // check if past last entry
        if (i >= (dirsize * ENTRIES_PER_BLOCK)) return SEARCH_FAILURE;  // check if past directory table
        if ((entry.parent_id == parent) && (entry.type == type) && (!kstrcmp(entry.name, name)))
            return i;
    }
}

path_result_t path_resolver(char* path, uint8_t type) {
    // returns a struct of useful info
    // failure flag set upon failure
    // not_found flag set upon not found
    // even if the file is not found, info about the "parent"
    // directory and name are still returned
    char name[FILENAME_LEN];
    entry_t parent = {0};
    int last = 0;
    int i;
    path_result_t result;
    entry_t empty_entry = {0};
    
    result.name[0] = 0;
    result.target_entry = 0;
    result.parent = empty_entry;
    result.target = empty_entry;
    result.failure = 0;
    result.not_found = 0;
    
    parent.payload = ROOT_ID;
    
    if ((type == DIRECTORY_TYPE) && !kstrcmp(path, "/")) {
        result.target.payload = ROOT_ID;
        return result; // exception for root
    }
    if ((type == FILE_TYPE) && !kstrcmp(path, "/")) {
        result.failure = 1;
        return result; // fail if looking for a file named "/"
    }
    
    if (*path == '/') path++;

next:    
    for (i = 0; *path != '/'; path++) {
        if (!*path) {
            last = 1;
            break;
        }
        name[i++] = *path;
    }
    name[i] = 0;
    path++;
    
    if (!last) {
        if (search(name, parent.payload, DIRECTORY_TYPE) == SEARCH_FAILURE) {
            result.failure = 1; // fail if search fails
            return result;
        }
        parent = rd_entry(search(name, parent.payload, DIRECTORY_TYPE));
    } else {
        if (search(name, parent.payload, type) == SEARCH_FAILURE)
            result.not_found = 1;
        else {
            result.target = rd_entry(search(name, parent.payload, type));
            result.target_entry = search(name, parent.payload, type);
        }
        result.parent = parent;
        kstrcpy(result.name, name);
        return result;
    }
    
    goto next;
}

int echfs_list(char* path, vfs_metadata_t* metadata, uint32_t entry, char* dev) {
    int dev_n = find_device(dev);

    device = dev;
    blocks = mounts[dev_n].blocks;
    fatsize = mounts[dev_n].fatsize;
    fatstart = mounts[dev_n].fatstart;
    dirsize = mounts[dev_n].dirsize;
    dirstart = mounts[dev_n].dirstart;
    datastart = mounts[dev_n].datastart;
    
    entry_t read_entry;
    path_result_t path_result;
    
    uint64_t id;
    uint32_t ii = 0;
    
    if (!*path)
        id = ROOT_ID;
    else {
        path_result = path_resolver(path, DIRECTORY_TYPE);
        if (path_result.not_found)
            return FAILURE;
        else
            id = path_result.target.payload;
    }
    
    for (uint32_t i = 0; i <= entry; i++) {
next:
        read_entry = rd_entry(ii);
        if ((!kstrcmp(read_entry.name, ".")) || (!kstrcmp(read_entry.name, ".."))) { ii++; goto next; }
        if ((read_entry.parent_id == id) && (i == entry)) break;
        else if (read_entry.parent_id == id) { ii++; continue; }
        else if (!read_entry.parent_id) return FAILURE;
        ii++;
        goto next;
    }
    
    kstrcpy(metadata->filename, read_entry.name);
    metadata->filetype = read_entry.type;
    return SUCCESS;
            
}

int echfs_write(char* path, uint8_t val, uint64_t loc, char* dev) { return 0; }

int echfs_mount(char* dev) {
    device = dev;
    
    // verify signature
    if (!fstrncmp(4, "_ECH_FS_", 8)) {
        kputs("\nechidnaFS signature invalid, mount failed!");
        return FAILURE;
    }
    
    mounts = krealloc(mounts, sizeof(mount_t) * (mounts_ptr+1));

    kstrcpy(mounts[mounts_ptr].name, dev);
    mounts[mounts_ptr].blocks = rd_qword(12);
    mounts[mounts_ptr].fatsize = (mounts[mounts_ptr].blocks * sizeof(uint64_t)) / BYTES_PER_BLOCK;
    if ((mounts[mounts_ptr].blocks * sizeof(uint64_t)) % BYTES_PER_BLOCK) mounts[mounts_ptr].fatsize++;
    mounts[mounts_ptr].fatstart = RESERVED_BLOCKS;
    mounts[mounts_ptr].dirsize = rd_qword(20);
    mounts[mounts_ptr].dirstart = mounts[mounts_ptr].fatstart + mounts[mounts_ptr].fatsize;
    mounts[mounts_ptr].datastart = RESERVED_BLOCKS + mounts[mounts_ptr].fatsize + mounts[mounts_ptr].dirsize;
    /*
    kputs("\nmounted with:");
    kputs("\nblocks:        "); kxtoa(mounts[mounts_ptr].blocks);
    kputs("\nfatsize:       "); kxtoa(mounts[mounts_ptr].fatsize);
    kputs("\nfatstart:      "); kxtoa(mounts[mounts_ptr].fatstart);
    kputs("\ndirsize:       "); kxtoa(mounts[mounts_ptr].dirsize);
    kputs("\ndirstart:      "); kxtoa(mounts[mounts_ptr].dirstart);
    kputs("\ndatastart:     "); kxtoa(mounts[mounts_ptr].datastart);
    */
    mounts_ptr++;

    return SUCCESS;
}

int echfs_read(char* path, uint64_t loc, char* dev) {
    uint64_t i;
    
    int dev_n = find_device(dev);

    device = dev;
    blocks = mounts[dev_n].blocks;
    fatsize = mounts[dev_n].fatsize;
    fatstart = mounts[dev_n].fatstart;
    dirsize = mounts[dev_n].dirsize;
    dirstart = mounts[dev_n].dirstart;
    datastart = mounts[dev_n].datastart;
    
    int cached_file;
    
    path_result_t path_result;

    if (!cached_files_ptr) goto skip_search;

    for (cached_file = 0; kstrcmp(cached_files[cached_file].path, path); cached_file++)
        if (cached_file == (cached_files_ptr - 1)) goto skip_search;
        
    path_result = cached_files[cached_file].path_result;
    goto search_out;

skip_search:

    cached_files = krealloc(cached_files, sizeof(cached_file_t) * (cached_files_ptr+1));

    kstrcpy(cached_files[cached_files_ptr].path, path);
    cached_files[cached_files_ptr].path_result = path_resolver(path, FILE_TYPE);

    path_result = cached_files[cached_files_ptr].path_result;
    
    cached_file = cached_files_ptr;
    
    // cache the allocation map
    cached_files[cached_file].alloc_map = kalloc(sizeof(uint64_t));
    cached_files[cached_file].alloc_map[0] = path_result.target.payload;
    for (i = 1; cached_files[cached_file].alloc_map[i-1] != END_OF_CHAIN; i++) {
        cached_files[cached_file].alloc_map = krealloc(cached_files[cached_file].alloc_map, sizeof(uint64_t) * (i+1));
        cached_files[cached_file].alloc_map[i] = rd_qword((fatstart * BYTES_PER_BLOCK) + (cached_files[cached_file].alloc_map[i-1] * sizeof(uint64_t)));
    }
    
    cached_files_ptr++;

search_out:
    if (path_result.not_found) return FAILURE;
    
    if (loc >= path_result.target.size) return EOF;
    
    uint64_t block = loc / BYTES_PER_BLOCK;
    uint64_t offset = loc % BYTES_PER_BLOCK;
    
    if (cached_files[cached_file].cache_status &&
       (cached_files[cached_file].cached_block == block))
        return cached_files[cached_file].cache[offset];
    
    // copy block in cache
    cached_files[cached_file].cache_status = 1;
    cached_files[cached_file].cached_block = block;
    for (i = 0; i < BYTES_PER_BLOCK; i++)
        cached_files[cached_file].cache[i] = rd_byte((cached_files[cached_file].alloc_map[block] * BYTES_PER_BLOCK) + i);
    
    return cached_files[cached_file].cache[offset];
}

int echfs_get_metadata(char* path, vfs_metadata_t* metadata, int type, char* dev) {
    int dev_n = find_device(dev);

    device = dev;
    blocks = mounts[dev_n].blocks;
    fatsize = mounts[dev_n].fatsize;
    fatstart = mounts[dev_n].fatstart;
    dirsize = mounts[dev_n].dirsize;
    dirstart = mounts[dev_n].dirstart;
    datastart = mounts[dev_n].datastart;
    
    path_result_t path_result = path_resolver(path, type);

    if (path_result.not_found) return FAILURE;
    
    metadata->filetype = type;
    metadata->size = path_result.target.size;
    kstrcpy(metadata->filename, path_result.target.name);
    
    return SUCCESS;
}

void install_echfs(void) {
    vfs_install_fs("echfs", &echfs_read, &echfs_write, &echfs_get_metadata, &echfs_list, &echfs_mount);
}
