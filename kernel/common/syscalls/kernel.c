#include <stdint.h>
#include <kernel.h>

void pwd(char* pwd_dump) {
    uint32_t pwd_dump_ptr = (uint32_t)pwd_dump;
    pwd_dump_ptr += task_table[current_task]->base;
    pwd_dump = (char*)pwd_dump_ptr;

    kstrcpy(pwd_dump, task_table[current_task]->pwd);
    return;
}
