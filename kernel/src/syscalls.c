#include <stdint.h>
#include <kernel.h>

void ipc_send_packet(int pid, char* payload, int len) {
    payload += task_table[current_task]->base;
    task_table[pid]->ipc_queue = krealloc(task_table[pid]->ipc_queue, (task_table[pid]->ipc_queue_ptr + 1) * sizeof(ipc_packet_t));
    
    task_table[pid]->ipc_queue[task_table[pid]->ipc_queue_ptr].payload = kalloc(len);
    kmemcpy(task_table[pid]->ipc_queue[task_table[pid]->ipc_queue_ptr].payload, payload, len);

    task_table[pid]->ipc_queue[task_table[pid]->ipc_queue_ptr].length = len;
    task_table[pid]->ipc_queue[task_table[pid]->ipc_queue_ptr++].sender = current_task;

    if (task_table[pid]->status == KRN_STAT_IPCWAIT_TASK)
        task_table[pid]->status = KRN_STAT_ACTIVE_TASK;

    return;
}

int ipc_payload_length(void) {
    if (task_table[current_task]->ipc_queue_ptr)
        return task_table[current_task]->ipc_queue[task_table[current_task]->ipc_queue_ptr-1].length;
    else
        return 0;
}

int ipc_payload_sender(void) {
    if (task_table[current_task]->ipc_queue_ptr)
        return task_table[current_task]->ipc_queue[task_table[current_task]->ipc_queue_ptr-1].sender;
    else
        return 0;
}

int ipc_resolve_name(char* server_name) {
    server_name += task_table[current_task]->base;
    // find the server name's PID
    int pid;
    for (pid = 0; task_table[pid]; pid++)
        if (!kstrcmp(server_name, task_table[pid]->server_name)) return pid;
    return 0;
}

int ipc_read_packet(char* payload) {
    if (!task_table[current_task]->ipc_queue_ptr) return 0;

    payload += task_table[current_task]->base;

    kmemcpy(payload,
            task_table[current_task]->ipc_queue[task_table[current_task]->ipc_queue_ptr-1].payload,
            task_table[current_task]->ipc_queue[task_table[current_task]->ipc_queue_ptr-1].length);
            
    kfree(task_table[current_task]->ipc_queue[task_table[current_task]->ipc_queue_ptr-1].payload);

    int pid = task_table[current_task]->ipc_queue[task_table[current_task]->ipc_queue_ptr-1].sender;
    
    task_table[current_task]->ipc_queue = krealloc(task_table[current_task]->ipc_queue,
                                          --task_table[current_task]->ipc_queue_ptr * sizeof(ipc_packet_t));
    
    return pid;
}

void* alloc(uint32_t size) {
    // search of a big enough, free, heap chunk
    heap_chunk_t* heap_chunk = (heap_chunk_t*)task_table[current_task]->heap_begin;
    heap_chunk_t* new_chunk;
    uint32_t heap_chunk_ptr;

again:
    if ((heap_chunk->free) && (heap_chunk->size > (size + sizeof(heap_chunk_t)))) {
        // split off a new heap_chunk
        new_chunk = (heap_chunk_t*)((uint32_t)heap_chunk + size + sizeof(heap_chunk_t));
        new_chunk->free = 1;
        new_chunk->size = heap_chunk->size - (size + sizeof(heap_chunk_t));
        new_chunk->prev_chunk = (uint32_t)heap_chunk;
        new_chunk->prev_chunk -= task_table[current_task]->base;
        heap_chunk->free = !heap_chunk->free;
        heap_chunk->size = size;
        return (void*)(((uint32_t)heap_chunk + sizeof(heap_chunk_t)) - task_table[current_task]->base);
    } else {
        heap_chunk_ptr = (uint32_t)heap_chunk;
        heap_chunk_ptr += heap_chunk->size + sizeof(heap_chunk_t);
        if (heap_chunk_ptr >= (task_table[current_task]->base + (task_table[current_task]->pages * 4096)))
            return (void*)0;        // alloc fail
        heap_chunk = (heap_chunk_t*)heap_chunk_ptr;
        goto again;
    }
}

void free(void* addr) {
    uint32_t heap_chunk_ptr = (uint32_t)addr;
    heap_chunk_ptr += task_table[current_task]->base;
    
    heap_chunk_ptr -= sizeof(heap_chunk_t);
    heap_chunk_t* heap_chunk = (heap_chunk_t*)heap_chunk_ptr;
    
    heap_chunk_ptr += heap_chunk->size + sizeof(heap_chunk_t);
    heap_chunk_t* next_chunk = (heap_chunk_t*)heap_chunk_ptr;
    
    heap_chunk_t* prev_chunk;
    if (heap_chunk->prev_chunk)
        prev_chunk = (heap_chunk_t*)(heap_chunk->prev_chunk + task_table[current_task]->base);
    else
        prev_chunk = (heap_chunk_t*)0;
    
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

void* realloc(void* prev_ptr, uint32_t size) {
    return (void*)0;
}

// prints a char to the current standard output
// for now, it will just print raw to the text driver
void char_to_stdout(int c) {
    text_putchar((char)c, task_table[current_task]->tty);
    return;
}

void enter_iowait_status(void) {
    task_table[current_task]->status = KRN_STAT_IOWAIT_TASK;
    return;
}

void enter_ipcwait_status(void) {
    task_table[current_task]->status = KRN_STAT_IPCWAIT_TASK;
    return;
}

void pwd(char* pwd_dump) {
    uint32_t pwd_dump_ptr = (uint32_t)pwd_dump;
    pwd_dump_ptr += task_table[current_task]->base;
    pwd_dump = (char*)pwd_dump_ptr;

    kstrcpy(pwd_dump, task_table[current_task]->pwd);
    return;
}
