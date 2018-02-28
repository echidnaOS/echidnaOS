#include <stdint.h>
#include <kernel.h>
#include <paging.h>
#include <klib.h>
#include <task.h>
#include <system.h>

int lseek(int handle, int offset, int type) {

    // redirect to new VFS stack
    return vfs_seek(handle, offset, type);

}

int read(int handle, char *ptr, int len) {

    // redirect to new VFS stack
    return vfs_uread(handle, ptr, len);

}

int write(int handle, char *ptr, int len) {

    // redirect to new VFS stack
    return vfs_uwrite(handle, ptr, len);

}

int open(char *path, int flags, int mode) {

    // redirect to new VFS stack
    return vfs_open(path, flags, mode);

}

int close(int handle) {

    // redirect to new VFS stack
    return vfs_close(handle);
    
}

size_t signal(int sig, size_t handler) {

    switch (sig) {
        size_t old_handler;
        case SIGABRT:
            old_handler = task_table[get_current_task()]->sigabrt;
            task_table[get_current_task()]->sigabrt = handler;
            return old_handler;
        case SIGFPE:
            old_handler = task_table[get_current_task()]->sigfpe;
            task_table[get_current_task()]->sigfpe = handler;
            return old_handler;
        case SIGILL:
            old_handler = task_table[get_current_task()]->sigill;
            task_table[get_current_task()]->sigill = handler;
            return old_handler;
        case SIGINT:
            old_handler = task_table[get_current_task()]->sigint;
            task_table[get_current_task()]->sigint = handler;
            return old_handler;
        case SIGSEGV:
            old_handler = task_table[get_current_task()]->sigsegv;
            task_table[get_current_task()]->sigsegv = handler;
            return old_handler;
        case SIGTERM:
            old_handler = task_table[get_current_task()]->sigterm;
            task_table[get_current_task()]->sigterm = handler;
            return old_handler;
        default:
            return SIG_ERR;
    }

}

int getpid(void) {
    return get_current_task();
}

size_t get_heap_base(void) {
    return task_table[get_current_task()]->heap_base;
}

size_t get_heap_size(void) {
    return task_table[get_current_task()]->heap_size;
}

int resize_heap(size_t heap_size) {
    size_t heap_pages = heap_size / PAGE_SIZE;
    if (heap_size % PAGE_SIZE) heap_pages++;

    size_t cur_heap_pages = task_table[get_current_task()]->heap_size / PAGE_SIZE;
    if (task_table[get_current_task()]->heap_size % PAGE_SIZE) cur_heap_pages++;

    pt_entry_t *pd = task_table[get_current_task()]->page_directory;

    size_t cur_heap_base = task_table[get_current_task()]->heap_base + cur_heap_pages * PAGE_SIZE;

    if (heap_pages > cur_heap_pages) {
        size_t needed_pages = heap_pages - cur_heap_pages;
        // generate the page tables
        // map the process's memory
        for (size_t i = 0; i < needed_pages; i++) {
            size_t page = (size_t)kmalloc(1);
            if (!page)
                return -1;
            map_page(pd, cur_heap_base + i * PAGE_SIZE, page, 0x07);
            (task_table[get_current_task()]->heap_size) += PAGE_SIZE;
        }
    } else if (heap_pages < cur_heap_pages) {
        size_t unneeded_pages = cur_heap_pages - heap_pages;
        for (size_t i = 0; i < unneeded_pages; i++) {
            kmfree((void *)get_phys_addr(pd, cur_heap_base + i * PAGE_SIZE), 1);
            unmap_page(pd, cur_heap_base + i * PAGE_SIZE);
            (task_table[get_current_task()]->heap_size) -= PAGE_SIZE;
        }
    }

    return 0;
}

void enter_iowait_status(char *dev, uint64_t loc, uint8_t payload, int type) {
    dev = (char *)get_phys_addr(task_table[get_current_task()]->page_directory, (size_t)dev);
    kstrcpy(task_table[get_current_task()]->iowait_dev, dev);
    task_table[get_current_task()]->iowait_loc = loc;
    task_table[get_current_task()]->status = KRN_STAT_IOWAIT_TASK;
    task_table[get_current_task()]->iowait_type = type;
    task_table[get_current_task()]->iowait_payload = payload;
    return;
}

void enter_iowait_status1(int handle, uint32_t ptr, int len, int type, int done) {
    task_table[get_current_task()]->status = KRN_STAT_IOWAIT_TASK;
    task_table[get_current_task()]->iowait_type = type;
    task_table[get_current_task()]->iowait_handle = handle;
    task_table[get_current_task()]->iowait_ptr = ptr;
    task_table[get_current_task()]->iowait_len = len;
    task_table[get_current_task()]->iowait_done = done;
    return;
}

void pwd(char *pwd_dump) {
    pwd_dump = (char *)get_phys_addr(task_table[get_current_task()]->page_directory, (size_t)pwd_dump);

    kstrcpy(pwd_dump, task_table[get_current_task()]->pwd);
    return;
}

void what_stdin(char *dump) {
    return;
}

void what_stdout(char *dump) {
    return;
}

void what_stderr(char *dump) {
    return;
}
