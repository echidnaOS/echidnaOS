#ifndef __TTY_H__
#define __TTY_H__

#include <kernel.h>
#include <stddef.h>

void text_putchar(char c, int window);

void plot_char(char c, int x, int y, uint32_t hex_fg, uint32_t hex_bg);

int keyboard_fetch_char(int window);

#endif
