#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            printf("%s", argv[i]);
            if (i != argc) putchar(' ');  // put a space in between each argument
        }
    }

    putchar('\n');
    return 0;
}
