#include <stdint.h>
#include <kernel.h>

#define VDEV_MAX 12

#define READY 0
#define NOT_READY -1

typedef struct {
    int pid;
    int status_in;
    uint32_t payload_in_addr;
    uint32_t payload_in_flag;
    int status_out;
    uint32_t payload_out_addr;
    uint32_t payload_out_flag;
} vdev_t;

char* vdev_names[] = {
    "vdev0", "vdev1", "vdev2", "vdev3",
    "vdev4", "vdev5", "vdev6", "vdev7",
    "vdev8", "vdev9", "vdev10", "vdev11"
};

static vdev_t* vdevs;
static int vdev_ptr = 0;

int vdev_in_ready(int vdev) {
    if (vdevs[vdev].pid != current_task) return -1;
    vdevs[vdev].status_in = READY;
    return 0;
}

int vdev_out_ready(int vdev) {
    if (vdevs[vdev].pid != current_task) return -1;
    vdevs[vdev].status_out = READY;
    return 0;
}

int vdev_io_wrapper(uint32_t vdev, uint64_t unused, int type, uint8_t payload) {

    if (type == 0) {
        if (vdevs[vdev].status_out != READY)
            return IO_NOT_READY;
        if (task_table[vdevs[vdev].pid]->status == KRN_STAT_VDEVWAIT_TASK) {
            task_table[vdevs[vdev].pid]->status = KRN_STAT_ACTIVE_TASK;
            /* return vdev id to the vdev manager task */
            task_table[vdevs[vdev].pid]->eax_p = vdev;
        }
        vdevs[vdev].status_out = NOT_READY;
        *( (int*)(vdevs[vdev].payload_out_flag + task_table[vdevs[vdev].pid]->base) ) = 1;
        return *( (uint8_t*)(vdevs[vdev].payload_out_addr + task_table[vdevs[vdev].pid]->base) );
    } else if (type == 1) {
        if (vdevs[vdev].status_in != READY)
            return IO_NOT_READY;
        if (task_table[vdevs[vdev].pid]->status == KRN_STAT_VDEVWAIT_TASK) {
            task_table[vdevs[vdev].pid]->status = KRN_STAT_ACTIVE_TASK;
            /* return vdev id to the vdev manager task */
            task_table[vdevs[vdev].pid]->eax_p = vdev;
        }
        vdevs[vdev].status_in = NOT_READY;
        *( (int*)(vdevs[vdev].payload_in_flag + task_table[vdevs[vdev].pid]->base) ) = 1;
        *( (uint8_t*)(vdevs[vdev].payload_in_addr + task_table[vdevs[vdev].pid]->base) ) = payload;
        return 0;
    }

}

int register_vdev(uint32_t payload_in_addr, uint32_t payload_in_flag,
                  uint32_t payload_out_addr, uint32_t payload_out_flag) {
    if (vdev_ptr >= VDEV_MAX) return -1;
    
    if (    (payload_in_addr > (task_table[current_task]->pages * PAGE_SIZE))
         || (payload_in_flag > (task_table[current_task]->pages * PAGE_SIZE))
         || (payload_out_addr > (task_table[current_task]->pages * PAGE_SIZE))
         || (payload_out_flag > (task_table[current_task]->pages * PAGE_SIZE)) )
        return -1;
    
    vdev_t* tmp_ptr = krealloc(vdevs, (vdev_ptr + 1) * sizeof(vdev_t));
    if (!tmp_ptr) return -1;
    
    vdevs = tmp_ptr;

    vdevs[vdev_ptr].pid = current_task;
    vdevs[vdev_ptr].status_in = NOT_READY;
    vdevs[vdev_ptr].payload_in_addr = payload_in_addr;
    vdevs[vdev_ptr].payload_in_flag = payload_in_flag;
    vdevs[vdev_ptr].status_out = NOT_READY;
    vdevs[vdev_ptr].payload_out_addr = payload_out_addr;
    vdevs[vdev_ptr].payload_out_flag = payload_out_flag;
    
    kernel_add_device(vdev_names[vdev_ptr], vdev_ptr, 0, &vdev_io_wrapper);
    
    return vdev_ptr++;
}
