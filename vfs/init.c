#include <stdio.h>
#include <stdlib.h>
#include <sys_api.h>

int main(int argc, char** argv) {
    printf("\nVFS running!\n");
    for (;;)
        OS_ipc_await();
}
