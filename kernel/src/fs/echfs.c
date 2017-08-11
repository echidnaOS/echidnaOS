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
#define SUCCESS 0

char* device;
uint64_t imgsize;
uint64_t blocks;
uint64_t fatsize;
uint64_t fatstart;
uint64_t dirsize;
uint64_t dirstart;
uint64_t datastart;

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
    // returns unique entry #, SEARCH_FAILURE upon failure/not found
    for (uint64_t i = 0; ; i++) {
        if (!rd_entry(i).parent_id) return SEARCH_FAILURE;              // check if past last entry
        if (i >= (dirsize * ENTRIES_PER_BLOCK)) return SEARCH_FAILURE;  // check if past directory table
        if ((rd_entry(i).parent_id == parent) && (rd_entry(i).type == type) && (!kstrcmp(rd_entry(i).name, name)))
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
    device = dev;
    blocks = rd_qword(12);
    fatsize = (blocks * sizeof(uint64_t)) / BYTES_PER_BLOCK;
    if ((blocks * sizeof(uint64_t)) % BYTES_PER_BLOCK) fatsize++;
    fatstart = RESERVED_BLOCKS;
    dirsize = rd_qword(20);
    dirstart = fatstart + fatsize;
    datastart = RESERVED_BLOCKS + fatsize + dirsize;
    
    uint64_t id;
    uint32_t ii = 0;
    
    if (!*path)
        id = ROOT_ID;
    else {
        if (path_resolver(path, DIRECTORY_TYPE).not_found)
            return FAILURE;
        else
            id = path_resolver(path, DIRECTORY_TYPE).target.payload;
    }
    
    for (uint32_t i = 0; i <= entry; i++) {
next:
        if ((rd_entry(ii).parent_id == id) && (i == entry)) break;
        else if (rd_entry(ii).parent_id == id) { ii++; continue; }
        else if (!rd_entry(ii).parent_id) return FAILURE;
        ii++;
        goto next;
    }
    
    kstrcpy(metadata->filename + task_table[current_task]->base, rd_entry(ii).name);
    return SUCCESS;
            
}

int echfs_write(char* path, uint8_t val, uint64_t loc, char* dev) { return 0; }
int echfs_read(char* path, uint64_t loc, char* dev) { return 0; }
int echfs_get_metadata(char* path, vfs_metadata_t* metadata, char* dev) { return 0; }

void install_echfs(void) {
    vfs_install_fs("echfs", &echfs_read, &echfs_write, &echfs_get_metadata, &echfs_list);
}
