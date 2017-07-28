#include <stdio.h>
#include <stdlib.h>
#include <sys_api.h>

int main(int argc, char** argv) {
    int pid;
    int len;
    char* payload;

    printf("Server started!\n");

    for (;;) {
        OS_ipc_await();
        pid = OS_ipc_payload_sender();
        printf("\nPacket incoming from PID: %d\n", pid);
        len = OS_ipc_payload_length();
        printf("Packet length: %d\n", len);
        payload = malloc(len);
        OS_ipc_read_packet(payload);
        printf("Packet payload: %s\n", payload);
        free(payload);
    }
}
