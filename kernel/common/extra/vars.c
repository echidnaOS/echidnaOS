#include <stdint.h>
#include <kernel.h>

// global variables declaration

uint32_t memory_size;
uint32_t memory_bottom = 0x1000000;
task_t* current_task = (task_t*)0x1000000;
uint8_t current_tty;
