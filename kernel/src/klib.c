#include <stdint.h>
#include <kernel.h>

void kmemcpy(char* dest, char* source, uint32_t count) {
    uint32_t i;

    for (i = 0; i < count; i++)
        dest[i] = source[i];

    return;
}

void kstrcpy(char* dest, char* source) {
    uint32_t i = 0;

    for ( ; source[i]; i++)
        dest[i] = source[i];
    
    dest[i] = 0;

    return;
}

int kstrcmp(char* dest, char* source) {
    uint32_t i = 0;

    for ( ; dest[i] == source[i]; i++)
        if ((!dest[i]) && (!source[i])) return 0;

    return 1;
}

int kstrncmp(char* dest, char* source, uint32_t len) {
    uint32_t i = 0;

    for ( ; i < len; i++)
        if (dest[i] != source[i]) return 1;

    return 0;
}

uint32_t kstrlen(char* str) {
    uint32_t len;

    for (len = 0; str[len]; len++);

    return len;
}

void init_kalloc(void) {
    // creates the first memory chunk
    heap_chunk_t* root_chunk = (heap_chunk_t*)KRNL_MEMORY_BASE;
    
    root_chunk->free = 1;
    root_chunk->size = memory_size - KRNL_MEMORY_BASE - sizeof(heap_chunk_t);
    root_chunk->prev_chunk = 0;

    return;
}

void* kalloc(uint32_t size) {
    // search of a big enough, free, heap chunk
    heap_chunk_t* heap_chunk = (heap_chunk_t*)KRNL_MEMORY_BASE;
    heap_chunk_t* new_chunk;
    uint32_t heap_chunk_ptr;
    char* area;
    
    // avoid odd memory allocations, align at 4
    while (size % 4) size++;

    for (;;) {
        if ((heap_chunk->free) && (heap_chunk->size > (size + sizeof(heap_chunk_t)))) {
            // split off a new heap_chunk
            new_chunk = (heap_chunk_t*)((uint32_t)heap_chunk + size + sizeof(heap_chunk_t));
            new_chunk->free = 1;
            new_chunk->size = heap_chunk->size - (size + sizeof(heap_chunk_t));
            new_chunk->prev_chunk = (uint32_t)heap_chunk;
            heap_chunk->free = !heap_chunk->free;
            heap_chunk->size = size;
            area = (char*)((uint32_t)heap_chunk + sizeof(heap_chunk_t));
            break;
        } else {
            heap_chunk_ptr = (uint32_t)heap_chunk;
            heap_chunk_ptr += heap_chunk->size + sizeof(heap_chunk_t);
            if (heap_chunk_ptr >= memory_size)
                return (void*)0;
            heap_chunk = (heap_chunk_t*)heap_chunk_ptr;
            continue;
        }
    }
    
    // zero the memory
    for (int i = 0; i < size; i++)
        area[i] = 0;
    return (void*)area;
}

void* krealloc(void* addr, uint32_t new_size) {
    if (!addr) return kalloc(new_size);
    if (!new_size) {
        kfree(addr);
        return (void*)0;
    }

    uint32_t heap_chunk_ptr = (uint32_t)addr;
    
    heap_chunk_ptr -= sizeof(heap_chunk_t);
    heap_chunk_t* heap_chunk = (heap_chunk_t*)heap_chunk_ptr;
    
    char* new_ptr;
    if ((new_ptr = kalloc(new_size)) == 0)
        return (void*)0;
    
    if (heap_chunk->size > new_size)
        kmemcpy(new_ptr, (char*)addr, new_size);
    else
        kmemcpy(new_ptr, (char*)addr, heap_chunk->size);
    
    kfree(addr);
    
    return new_ptr;
}

void kfree(void* addr) {
    uint32_t heap_chunk_ptr = (uint32_t)addr;
    
    heap_chunk_ptr -= sizeof(heap_chunk_t);
    heap_chunk_t* heap_chunk = (heap_chunk_t*)heap_chunk_ptr;
    
    heap_chunk_ptr += heap_chunk->size + sizeof(heap_chunk_t);
    heap_chunk_t* next_chunk = (heap_chunk_t*)heap_chunk_ptr;
    
    heap_chunk_t* prev_chunk = (heap_chunk_t*)heap_chunk->prev_chunk;
    
    // flag chunk as free
    heap_chunk->free = 1;
    
    // if the next chunk is free as well, fuse the chunks into a single one
    if (next_chunk->free)
        heap_chunk->size += next_chunk->size + sizeof(heap_chunk_t);
    
    // if the previous chunk is free as well, fuse the chunks into a single one
    if (prev_chunk) {       // if its not the first chunk
        if (prev_chunk->free)
            prev_chunk->size += heap_chunk->size + sizeof(heap_chunk_t);
    }
    
    return;
}

uint64_t power(uint64_t x, uint64_t y) {
    uint64_t res;
    for (res = 1; y; y--)
        res *= x;
    return res;
}

void kputs(const char* string) {
    tty_kputs(string, 0);
    
    return;
}

void tty_kputs(const char* string, uint8_t which_tty) {
    uint32_t i;
    for (i = 0; string[i]; i++)
        text_putchar(string[i], which_tty);
    return;
}

void kuitoa(uint64_t x) {
    tty_kuitoa(x, 0);

    return;
}

void tty_kuitoa(uint64_t x, uint8_t which_tty) {
    uint8_t i;
    char buf[21] = {0};

    if (!x) {
        tty_kputs("0", which_tty);
        return;
    }

    for (i = 19; x; i--) {
        buf[i] = (x % 10) + 0x30;
        x = x / 10;
    }

    i++;
    tty_kputs(buf + i, which_tty);

    return;
}

static const char hex_to_ascii_tab[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

void kxtoa(uint64_t x) {
    tty_kxtoa(x, 0);

    return;
}

void tty_kxtoa(uint64_t x, uint8_t which_tty) {
    uint8_t i;
    char buf[17] = {0};

    if (!x) {
        tty_kputs("0x0", which_tty);
        return;
    }

    for (i = 15; x; i--) {
        buf[i] = hex_to_ascii_tab[(x % 16)];
        x = x / 16;
    }

    i++;
    tty_kputs("0x", which_tty);
    tty_kputs(buf + i, which_tty);

    return;
}
