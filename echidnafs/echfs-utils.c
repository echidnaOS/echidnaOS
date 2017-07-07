#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define SEARCH_FAILURE          0xffffffffffffffff
#define ROOT_ID                 0xffffffffffffffff
#define ENTRIES_PER_BLOCK       2
#define FILENAME_LEN            218
#define RESERVED_BLOCKS         16
#define BYTES_PER_BLOCK         512
#define FILE_TYPE               0
#define DIRECTORY_TYPE          1
#define DELETED_ENTRY           0xffffffffeeeeeeee
#define RESERVED_BLOCK          0xffffffff00000000
#define END_OF_CHAIN            0xffffffffffffffff

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

FILE* image;
uint64_t imgsize;
uint64_t blocks;
uint64_t fatsize;
uint64_t fatstart = RESERVED_BLOCKS;
uint64_t dirsize;
uint64_t dirstart;
uint64_t datastart;

uint64_t power(uint64_t x, uint64_t y) {
    uint64_t res;
    for (res = 1; y; y--)
        res *= x;
    return res;
}

uint8_t rd_byte(uint64_t loc) {
    fseek(image, (long)loc, SEEK_SET);
    return (uint8_t)fgetc(image);
}

void wr_byte(uint64_t loc, uint8_t x) {
    fseek(image, (long)loc, SEEK_SET);
    fputc((int)x, image);
    return;
}

uint16_t rd_word(uint64_t loc) {
    uint16_t x = 0;
    fseek(image, (long)loc, SEEK_SET);
    for (uint64_t i = 0; i < 2; i++)
        x += (uint16_t)fgetc(image) * (uint16_t)(power(0x100, i));
    return x;
}

void wr_word(uint64_t loc, uint16_t x) {
    fseek(image, (long)loc, SEEK_SET);
    for (uint64_t i = 0; i < 2; i++)
       fputc((int)(x / (power(0x100, i)) & 0xff), image);
    return;
}

uint32_t rd_dword(uint64_t loc) {
    uint32_t x = 0;
    fseek(image, (long)loc, SEEK_SET);
    for (uint64_t i = 0; i < 4; i++)
        x += (uint32_t)fgetc(image) * (uint32_t)(power(0x100, i));
    return x;
}

void wr_dword(uint64_t loc, uint32_t x) {
    fseek(image, (long)loc, SEEK_SET);
    for (uint64_t i = 0; i < 4; i++)
       fputc((int)(x / (power(0x100, i)) & 0xff), image);
    return;
}

uint64_t rd_qword(uint64_t loc) {
    uint64_t x = 0;
    fseek(image, (long)loc, SEEK_SET);
    for (uint64_t i = 0; i < 8; i++)
        x += (uint64_t)fgetc(image) * (uint64_t)(power(0x100, i));
    return x;
}

void wr_qword(uint64_t loc, uint64_t x) {
    fseek(image, (long)loc, SEEK_SET);
    for (uint64_t i = 0; i < 8; i++)
       fputc((int)(x / (power(0x100, i)) & 0xff), image);
    return;
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

void fstrcpy_in(char* str, uint64_t loc) {
    int i = 0;
    while (rd_byte(loc))
        str[i++] = (char)rd_byte(loc++);
    str[i] = 0;
    return;
}

void fstrcpy_out(uint64_t loc, const char* str) {
    int i = 0;
    while (str[i])
        wr_byte(loc++, (uint8_t)str[i++]);
    wr_byte(loc, 0);
    return;
}

void eputs(uint64_t loc) {
    int c;
    while (c = (int)rd_byte(loc++))
        fputc(c, stdout);
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

void wr_entry(uint64_t entry, entry_t entry_src) {
    uint64_t loc = (dirstart * BYTES_PER_BLOCK) + (entry * sizeof(entry_t));

    wr_qword(loc, entry_src.parent_id);
    loc += sizeof(uint64_t);
    wr_byte(loc++, entry_src.type);
    fstrcpy_out(loc, entry_src.name);
    loc += FILENAME_LEN;
    wr_byte(loc++, entry_src.perms);
    wr_word(loc, entry_src.owner);
    loc += sizeof(uint16_t);
    wr_word(loc, entry_src.group);
    loc += sizeof(uint16_t);
    wr_byte(loc++, entry_src.hundreths);
    wr_byte(loc++, entry_src.seconds);
    wr_byte(loc++, entry_src.minutes);
    wr_byte(loc++, entry_src.hours);
    wr_byte(loc++, entry_src.day);
    wr_byte(loc++, entry_src.month);
    wr_word(loc, entry_src.year);
    loc += sizeof(uint16_t);
    wr_qword(loc, entry_src.payload);
    loc += sizeof(uint64_t);
    wr_qword(loc, entry_src.size);
    
    return;
}

uint64_t import_chain(FILE* source) {
    uint64_t start_block;
    uint64_t block;
    uint64_t prev_block;
    uint64_t loc = (fatstart * BYTES_PER_BLOCK);
    
    fseek(source, 0L, SEEK_END);
    uint64_t source_size = (uint64_t)ftell(source);    
    rewind(source);
    
    uint64_t source_size_blocks = source_size / BYTES_PER_BLOCK;
    if (source_size % BYTES_PER_BLOCK) source_size_blocks++;
    
    fprintf(stdout, "file size: %" PRIu64 "\n", source_size);
    fprintf(stdout, "file size in blocks: %" PRIu64 "\n", source_size_blocks);
    
    // find first block
    for (block = 0; rd_qword(loc); block++) loc += 8;    
    start_block = block;
    fprintf(stdout, "first block of chain is #%" PRIu64 "\n", start_block);    
    
    for (uint64_t x = 0; ; x++) {
        prev_block = (fatstart * BYTES_PER_BLOCK) + (block * sizeof(uint64_t));
        wr_qword(loc, END_OF_CHAIN);
        
        fseek(image, (long)(block * 512), SEEK_SET);    
        // copy block
        for (int i = 0; i < 512; i++) {
            if (ftell(source) == source_size) goto out;
            fputc(fgetc(source), image);
        }
        
        if (x == source_size_blocks) goto out;
        
        // find next block
        loc = (fatstart * BYTES_PER_BLOCK);
        for (block = 0; rd_qword(loc); block++) loc += 8;
        
        wr_qword(prev_block, block);
    }
out:   
    
    return start_block;
}

uint64_t search(const char* name, uint64_t parent, uint8_t type) {
    // returns unique entry #, SEARCH_FAILURE upon failure/not found
    for (uint64_t i = 0; ; i++) {
        if (!rd_entry(i).parent_id) return SEARCH_FAILURE;              // check if past last entry
        if (i >= (dirsize * ENTRIES_PER_BLOCK)) return SEARCH_FAILURE;  // check if past directory table
        if ((rd_entry(i).parent_id == parent) && (rd_entry(i).type == type) && (!strcmp(rd_entry(i).name, name)))
            return i;
    }
}

path_result_t path_resolver(const char* path, uint8_t type) {
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
    
    if ((type == DIRECTORY_TYPE) && !strcmp(path, "/")) {
        result.target.payload = ROOT_ID;
        return result; // exception for root
    }
    if ((type == FILE_TYPE) && !strcmp(path, "/")) {
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
        strcpy(result.name, name);
        return result;
    }
    
    goto next;
}

uint64_t get_free_id(void) {
    uint64_t id = 1;
    uint64_t i;

    for (i = 0; rd_entry(i).parent_id; i++) {
        if ((rd_entry(i).type == 1) && (rd_entry(i).payload == id))
            id = (rd_entry(i).payload + 1);
    }
    
    return id;
}

void mkdir_cmd(int argc, char** argv) {
    uint64_t i;
    entry_t entry = {0};
    entry_t extra_entry = {0};
    
    if (argc < 4) {
        fprintf(stderr, "%s: %s: missing argument: directory name.\n", argv[0], argv[2]);
        return;
    }
    
    // check if it exists
    if (!path_resolver(argv[3], DIRECTORY_TYPE).not_found) {
        fprintf(stderr, "%s: %s: directory `%s` already exists.\n", argv[0], argv[2], argv[3]);
        return;
    }
    
    // find empty entry
    for (i = 0; ; i++) {
        if ((rd_entry(i).parent_id == 0) || (rd_entry(i).parent_id == DELETED_ENTRY))
            break;
    }
    
    entry.parent_id = path_resolver(argv[3], DIRECTORY_TYPE).parent.payload;
    fprintf(stdout, "new directory's parent ID: %" PRIu64 "\n", entry.parent_id);
    entry.type = DIRECTORY_TYPE;
    strcpy(entry.name, path_resolver(argv[3], DIRECTORY_TYPE).name);
    entry.payload = get_free_id();
    fprintf(stdout, "new directory's ID: %" PRIu64 "\n", entry.payload);
    fprintf(stdout, "writing to entry #%" PRIu64 "\n", i);
    
    wr_entry(i, entry);
    
    fprintf(stdout, "creating `.` entry\n");
    // find empty entry
    for (i = 0; ; i++) {
        if ((rd_entry(i).parent_id == 0) || (rd_entry(i).parent_id == DELETED_ENTRY))
            break;
    }
    extra_entry.parent_id = entry.payload;
    extra_entry.payload = entry.payload;
    extra_entry.type = DIRECTORY_TYPE;
    strcpy(extra_entry.name, ".");
    wr_entry(i, extra_entry);
    
    fprintf(stdout, "creating `..` entry\n");
    // find empty entry
    for (i = 0; ; i++) {
        if ((rd_entry(i).parent_id == 0) || (rd_entry(i).parent_id == DELETED_ENTRY))
            break;
    }
    extra_entry.parent_id = entry.payload;
    extra_entry.payload = entry.parent_id;
    extra_entry.type = DIRECTORY_TYPE;
    strcpy(extra_entry.name, "..");
    wr_entry(i, extra_entry);    
    
    fprintf(stdout, "created directory `%s`\n", argv[3]);

    return;
}

void import_cmd(int argc, char** argv) {
    FILE* source;

    if ((source = fopen(argv[3], "r")) == NULL) {
        fprintf(stderr, "%s: %s: error: couldn't access `%s`.\n", argv[0], argv[2], argv[3]);
        return;
    }

    import_chain(source);
    fclose(source);
    return;
}

void ls_cmd(int argc, char** argv) {
    uint64_t id;
    
    if (argc < 4)
        id = ROOT_ID;
    else {
        if (path_resolver(argv[3], DIRECTORY_TYPE).not_found) {
            fprintf(stdout, "%s: %s: error: invalid directory `%s`.\n", argv[0], argv[2], argv[3]);
            return;
        } else
            id = path_resolver(argv[3], DIRECTORY_TYPE).target.payload;
    }

    fprintf(stdout, "  ---- ls ----\n");
    
    for (uint64_t i = 0; rd_entry(i).parent_id; i++) {
        if (rd_entry(i).parent_id != id) continue;
        if (rd_entry(i).type == DIRECTORY_TYPE) fputs("[", stdout);
        fputs(rd_entry(i).name, stdout);
        if (rd_entry(i).type == DIRECTORY_TYPE) fputs("]", stdout);
        fputc('\n', stdout);
    }

    return;
}

void format_pass1(void) {

    fprintf(stdout, "formatting...\n");

    // write signature
    fstrcpy_out(4, "_ECH_FS_");
    // total blocks
    wr_qword(12, blocks);
    // directory size
    wr_qword(20, blocks / 20); // blocks / 20 (roughly 5% of the total)
    
    fseek(image, (16 * 512), SEEK_SET);
    fprintf(stdout, "zeroing");
    
    // zero out the rest of the image
    for (uint64_t i = (16 * 512); i < imgsize; i++) {
        fputc(0, image);
        if (!(i % 65536))
            fputc('.', stdout);
    }
    
    fputc('\n', stdout);
    
    return;

}

void format_pass2(void) {
    // mark reserved blocks
    uint64_t loc = fatstart * 512;
    
    for (uint64_t i = 0; i < (16 + fatsize + dirsize); i++) {
        wr_qword(loc, RESERVED_BLOCK);
        loc += 8;
    }
    
    fprintf(stdout, "format complete!\n");

    return;
}

int main(int argc, char** argv) {

    if (argc == 1) {
        fprintf(stderr, "Usage: %s [image] <action> <args...>\n", argv[0]);
        return EXIT_SUCCESS;
    }

    if ((image = fopen(argv[1], "r+")) == NULL) {
        fprintf(stderr, "%s: error: couldn't access `%s`.\n", argv[0], argv[1]);
        return EXIT_FAILURE;
    }
    
    fseek(image, 0L, SEEK_END);
    imgsize = (uint64_t)ftell(image);    
    rewind(image);
    
    fprintf(stdout, "image size: %" PRIu64 " bytes\n", imgsize);
    
    if (imgsize % 512) {
        fprintf(stderr, "%s: error: image is not block-aligned.\n", argv[0]);
        fclose(image);
        return EXIT_FAILURE;
    }
    
    blocks = imgsize / 512;
    
    fprintf(stdout, "block count: %" PRIu64 "\n", blocks);
    
    if ((argc > 2) && (!strcmp(argv[2], "format"))) format_pass1();
    
    if (!fstrncmp(4, "_ECH_FS_", 8)) {
        fprintf(stderr, "%s: error: echidnaFS signature missing.\n", argv[0]);
        fclose(image);
        return EXIT_FAILURE;
    }
    fprintf(stdout, "echidnaFS signature found\n");
    
    fprintf(stdout, "declared block count: %" PRIu64 "\n", rd_qword(12));
    if (rd_qword(12) != blocks) {
        fprintf(stderr, "%s: error: declared block count mismatch.\n", argv[0]);
        fclose(image);
        return EXIT_FAILURE;
    }
    
    fatsize = (blocks * 4) / 512;
    if ((blocks * 4) % 512) fatsize++;    
    fprintf(stdout, "expected allocation table size: %" PRIu64 " blocks\n", fatsize);
    
    fprintf(stdout, "expected allocation table start: block %" PRIu64 "\n", fatstart);
    
    dirsize = rd_qword(20);
    fprintf(stdout, "declared directory size: %" PRIu64 " blocks\n", dirsize);
    
    dirstart = fatstart + fatsize;
    fprintf(stdout, "expected directory start: block %" PRIu64 "\n", dirstart);
    
    datastart = 16 + fatsize + dirsize;
    fprintf(stdout, "expected reserved blocks: %" PRIu64 "\n", datastart);
    
    fprintf(stdout, "expected usable blocks: %" PRIu64 "\n", blocks - datastart);
    
    if (rd_word(510) == 0xaa55)
        fprintf(stdout, "the image is bootable\n");
    else
        fprintf(stdout, "the image is NOT bootable\n");
    
    if (argc > 2) {
        if (!strcmp(argv[2], "mkdir")) mkdir_cmd(argc, argv);
        else if (!strcmp(argv[2], "ls")) ls_cmd(argc, argv);
        else if (!strcmp(argv[2], "format")) format_pass2();
        else if (!strcmp(argv[2], "import")) import_cmd(argc, argv);
    
        else fprintf(stderr, "%s: error: invalid action: `%s`.\n", argv[0], argv[2]);
    }

    fclose(image);

    return 0;
}
