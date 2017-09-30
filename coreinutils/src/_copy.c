#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void copy(char* fsrc, char* fdest) {
    FILE* src;
    FILE* dest;
    char* tmp_dest;
    int c = 0;

    if (fopen(fsrc, "rb") == NULL) {
        fputs("Cannot open file.\n", stderr); /* Needs to be changed to perror() when it's implemented */
        exit(EXIT_FAILURE);
    }

    if (fopen(fdest, "w+b") == NULL) {
        tmp_dest = malloc(strlen(fdest) + strlen(fsrc) + 25);
        if (tmp_dest == NULL) {
            fputs("Malloc error.\n", stderr);
            exit(EXIT_FAILURE);
        }
        strcpy(tmp_dest, fdest);
        if (tmp_dest[strlen(tmp_dest) - 1] != '/')
            strcat(tmp_dest, "/");
        strcat(tmp_dest, fsrc);
        if (fopen(tmp_dest, "w+b") == NULL) {
            fputs("Cannot write.\n", stderr);
            exit(EXIT_FAILURE);
        } else
            fdest = tmp_dest;
    }

    src = fopen(fsrc, "rb");
    dest = fopen(fdest, "wb");

    while ((c = fgetc(src)) != EOF)
        fputc(c, dest);

    fclose(src);
    fclose(dest);
}
