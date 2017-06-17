#include <stdint.h>
#include <kernel.h>

void* alloc(uint32_t size) {
    // search of a big enough, free, heap chunk
    heap_chunk_t* heap_chunk = (heap_chunk_t*)current_task->heap_begin;
    heap_chunk_t* new_chunk;
    uint32_t heap_chunk_ptr;

again:
    if ((heap_chunk->free) && (heap_chunk->size > (size + sizeof(heap_chunk_t)))) {
        // split off a new heap_chunk
        new_chunk = (heap_chunk_t*)((uint32_t)heap_chunk + size + sizeof(heap_chunk_t));
        new_chunk->free = 1;
        new_chunk->size = heap_chunk->size - (size + sizeof(heap_chunk_t));
        heap_chunk->free = !heap_chunk->free;
        heap_chunk->size = size;
        return (void*)(((uint32_t)heap_chunk + sizeof(heap_chunk_t)) - current_task->base);
    } else {
        heap_chunk_ptr = (uint32_t)heap_chunk;
        heap_chunk_ptr += heap_chunk->size + sizeof(heap_chunk_t);
        if (heap_chunk_ptr >= (current_task->base + (current_task->pages * 4096)))
            return (void*)0;        // alloc fail
        heap_chunk = (heap_chunk_t*)heap_chunk_ptr;
        goto again;
    }
}

void freemem(void* addr) {
    return;
}
