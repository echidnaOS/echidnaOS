#include <stdint.h>
#include <kernel.h>

// global variables declaration

uint32_t memory_size;
uint32_t memory_bottom;
task_t* current_task;
uint8_t current_tty;
