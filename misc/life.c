#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS 24
#define COLS 80

void escape_fg(int col) {
    fputs("\e[3", stdout);
    putchar(col + '0');
    putchar('m');
}

void escape_bg(int col) {
    fputs("\e[4", stdout);
    putchar(col + '0');
    putchar('m');
}

void escape_cursor_reset() {
    fputs("\e[H", stdout);
}

void render_grid(int* grid) {
    escape_cursor_reset();
    for (int i = 0; i < ROWS * COLS; i++) {
        escape_bg(grid[i] % 2 ? 7 : 0);
        escape_fg(grid[i] % 2 ? 0 : 7);
        putchar(grid[i] > 1 ? '#' : ' ');
    }
}

int get_neighbors(int* grid, int x, int y) {
    int total = 0;
    for (int x2 = x - 1; x2 <= x + 1; x2++) {
        for (int y2 = y - 1; y2 <= y + 1; y2++) {
            total += grid[COLS * ((y2 + ROWS) % ROWS) + ((x2 + COLS) % COLS)];
        }
    }

    return total - grid[COLS * y + x];
}

void tick(int* in, int* out) {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            int neighbors = get_neighbors(in, x, y);
            if (in[COLS * y + x])
                out[COLS * y + x] = neighbors > 1 && neighbors < 4;
            else
                out[COLS * y + x] = neighbors == 3;
        }
    }
}

void random_generate(char* seed, int* grid) {
    uint16_t lfsr = 0x4200u;
    for (int i = 0; seed[i]; i++)
        lfsr += seed[i];

    uint16_t bit;
    for (int i = 0; i < ROWS * COLS; i++) {
        bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
        lfsr =  (lfsr >> 1) | (bit << 15);
        grid[i] = bit;
    }
}

void quit() {
    puts("\e[r\e[b\e[s\e[30m");
    exit(0);
}

void edit_generate(int* grid) {
    int x = 0; int y = 0;
    for (;;) {
        int c = getchar();

        switch (c) {
            case '\n':
                return;
            case 'q':
                quit();
            case 'h': case 'a':
                x = (--x + COLS) % COLS;
                break;
            case 'l': case 'd':
                x = (++x + COLS) % COLS;
                break;
            case 'k': case 'w':
                y = (--y + ROWS) % ROWS;
                break;
            case 'j': case 's':
                y = (++y + ROWS) % ROWS;
                break;
            case ' ':
                grid[y * COLS + x] ^= 1;
                break;
        }

        grid[y * COLS + x] ^= 2;
        render_grid(grid);
        grid[y * COLS + x] ^= 2;
    }
}

int main(int argc, char** argv) {
    /* raw nonblocking */
    puts("\e[r\e[b\e[s");

    int g1[ROWS * COLS] = {0}; int* grid1 = g1;
    int g2[ROWS * COLS] = {0}; int* grid2 = g2;

    if (argc > 1)
        if (!strcmp(argv[1], "-e"))
            edit_generate(grid1);
        else
            random_generate(argv[1], grid1);
    else
        random_generate("", grid1);

    for (;;) {
        render_grid(grid1);
        tick(grid1, grid2);

        int* tmp = grid1; grid1 = grid2; grid2 = tmp;

        if (getchar() == 'q') quit();
    }

    getchar();
}
