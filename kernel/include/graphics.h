#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <stdint.h>
#include <stddef.h>
#include <kernel.h>

typedef struct window_t {
    int id;
    char title[2048];
    size_t x;
    size_t y;
    size_t x_size;
    size_t y_size;
    uint32_t *framebuffer;
    char *grid;
    uint32_t *gridbg;
    uint32_t *gridfg;
    int cursor_x;
    int cursor_y;
    int cursor_status;
    uint32_t cursor_bg_col;
    uint32_t cursor_fg_col;
    uint32_t text_bg_col;
    uint32_t text_fg_col;
    char kb_l1_buffer[KB_L1_SIZE];
    char kb_l2_buffer[KB_L2_SIZE];
    uint16_t kb_l1_buffer_index;
    uint16_t kb_l2_buffer_index;
    int escape;
    int *esc_value;
    int esc_value0;
    int esc_value1;
    int *esc_default;
    int esc_default0;
    int esc_default1;
    int raw;
    int noblock;
    int noscroll;
    struct window_t *next;
} window_t;

typedef struct {
    uint8_t version_min;
    uint8_t version_maj;
    uint32_t oem;   // is a 32 bit pointer to char
    uint32_t capabilities;
    uint32_t vid_modes;     // is a 32 bit pointer to uint16_t
    uint16_t vid_mem_blocks;
    uint16_t software_rev;
    uint32_t vendor;   // is a 32 bit pointer to char
    uint32_t prod_name;   // is a 32 bit pointer to char
    uint32_t prod_rev;   // is a 32 bit pointer to char
} __attribute__((packed)) vbe_info_struct_t;

typedef struct {
    uint8_t padding[8];
    uint16_t manufacturer_id_be;
    uint16_t edid_id_code;
    uint32_t serial_num;
    uint8_t man_week;
    uint8_t man_year;
    uint8_t edid_version;
    uint8_t edid_revision;
    uint8_t video_input_type;
    uint8_t max_hor_size;
    uint8_t max_ver_size;
    uint8_t gamma_factor;
    uint8_t dpms_flags;
    uint8_t chroma_info[10];
    uint8_t est_timings1;
    uint8_t est_timings2;
    uint8_t man_res_timing;
    uint16_t std_timing_id[8];
    uint8_t det_timing_desc1[18];
    uint8_t det_timing_desc2[18];
    uint8_t det_timing_desc3[18];
    uint8_t det_timing_desc4[18];
    uint8_t unused;
    uint8_t checksum;
} __attribute__((packed)) edid_info_struct_t;

typedef struct {
    uint8_t pad0[18];
    uint16_t res_x;
    uint16_t res_y;
    uint8_t pad1[3];
    uint8_t bpp;
    uint8_t pad2[14];
    uint32_t framebuffer;
    uint8_t pad3[212];
} __attribute__((packed)) vbe_mode_info_t;

typedef struct {
    uint32_t vbe_mode_info;      // is a 32 bit pointer to vbe_mode_info_t
    uint16_t mode;
} get_vbe_t;

void plot_px(int x, int y, uint32_t hex);
void plot_px_window(int x, int y, uint32_t hex, int window);

int create_window(char *title, size_t x, size_t y, size_t x_size, size_t y_size);
void gui_refresh(void);
void window_focus(int window);
void window_move(int x, int y, int window);
window_t *get_window_ptr(int id);

extern uint32_t *framebuffer;
extern uint8_t vga_font[4096];
extern int edid_width;
extern int edid_height;

extern int modeset_done;
extern int gui_needs_refresh;

extern int current_window;


#endif
