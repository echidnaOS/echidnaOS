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
    
    // avoid odd memory allocations, align at 4
    while (size % 4) size++;

again:
    if ((heap_chunk->free) && (heap_chunk->size > (size + sizeof(heap_chunk_t)))) {
        // split off a new heap_chunk
        new_chunk = (heap_chunk_t*)((uint32_t)heap_chunk + size + sizeof(heap_chunk_t));
        new_chunk->free = 1;
        new_chunk->size = heap_chunk->size - (size + sizeof(heap_chunk_t));
        new_chunk->prev_chunk = (uint32_t)heap_chunk;
        heap_chunk->free = !heap_chunk->free;
        heap_chunk->size = size;
        return (void*)((uint32_t)heap_chunk + sizeof(heap_chunk_t));
    } else {
        heap_chunk_ptr = (uint32_t)heap_chunk;
        heap_chunk_ptr += heap_chunk->size + sizeof(heap_chunk_t);
        if (heap_chunk_ptr >= memory_size)
            return (void*)0;
        heap_chunk = (heap_chunk_t*)heap_chunk_ptr;
        goto again;
    }
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
