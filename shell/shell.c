#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys_api.h>

#include "cmds.h"

void getstring(char* string, uint32_t limit);
int get_argc(const char* string);
void get_argv(char** argv, char* string);

// built in shell

int main(int argc, char** argv) {
    char input[256];
    int s_argc;
    char* s_argv[128];
    char pwd[2048];
    
    int c;
    uint64_t i;
    
    vfs_metadata_t metadata = {0};

    while (1) {
        OS_pwd(pwd);
        printf("%s# ", pwd);

        getstring(input, 256);
        s_argc = get_argc(input);
        get_argv(s_argv, input);

        if (!strcmp("help", s_argv[0]))
            help_cmd(s_argc, s_argv);

        else if (!strcmp("echo", s_argv[0]))
            echo_cmd(s_argv, s_argc);

        else if ((!strcmp("cowsay", s_argv[0])) || (!strcmp("tuxsay", s_argv[0])) || (!strcmp("daemonsay", s_argv[0])))
            cowsay_cmd(s_argv, s_argc);

        else if ((!strcmp("bcowsay", s_argv[0])) || (!strcmp("dcowsay", s_argv[0])) || (!strcmp("gcowsay", s_argv[0])))
            cowsay_cmd(s_argv, s_argc);

        else if ((!strcmp("pcowsay",s_argv[0])) || (!strcmp("scowsay", s_argv[0])) || (!strcmp("tcowsay", s_argv[0])))
            cowsay_cmd(s_argv, s_argc);
        
        else if (!strcmp("div0", s_argv[0])) {
            int a = 0;
            a = a / 0;
        }
        
        else if (!strcmp("ls", s_argv[0])) {
            char* ls_path;
            if (s_argc == 1) ls_path = pwd;
            else ls_path = s_argv[1];
            for (int i = 0; ; i++) {
                if (OS_vfs_list(ls_path, &metadata, i) == -2) break;
                if (metadata.filetype == 1) putchar('[');
                printf("%s", metadata.filename);
                if (metadata.filetype == 1) putchar(']');
                putchar('\n');
            }
        }
        
        else if (!strcmp("beep", s_argv[0])) {
            if (s_argc == 1) continue;
            OS_vfs_write("/dev/pcspk", atoi(s_argv[1]), 0);
        }
        
        else if (!strcmp("rdspk", s_argv[0]))
            printf("%d\n", OS_vfs_read("/dev/pcspk", 0));
        
        else if (!strcmp("send", s_argv[0])) {
            char server[] = "server";
            uint32_t pid = OS_ipc_resolve_name(server);
            printf("%s's PID is: %d\n", server, pid);
            printf("payload is: %s\n", s_argv[1]);
            OS_ipc_send_packet(pid, s_argv[1], strlen(s_argv[1]) + 1);
        }
        
        else if (!strcmp("exit", s_argv[0])) return 0;
        
        else if (!strcmp("dump", s_argv[0])) {
            if (s_argc == 1) continue;
            for (i = 0; ((c = OS_vfs_read(s_argv[1], i)) != -1); i++) {
                if (c == -2) break;
                putchar(c);
            }
        }
        
        else if (!strcmp("cd", s_argv[0])) {
            if (s_argc == 1) continue;
            OS_vfs_cd(s_argv[1]);
        }

        // return to prompt if no input
        else if (!input[0]) continue;

        // if the input did not match any command
        else printf("shell: invalid command: `%s`.\n", input);
    }
    
    return 0;
}

int get_argc(const char* string) {
    uint32_t index=0;
    int argc=0;
    
    while (string[index]) {
        if (string[index] == ' ') {
            index++;
            continue;
        }
        while ((string[index]!=' ') && (string[index]!='\0'))
            index++;
        argc++;
    }
    return argc;
}

void get_argv(char** argv, char* string) {
    uint32_t index=0;
    uint8_t arg=0;
    
    while (string[index]) {
        if (string[index] == ' ') {
            string[index++] = 0;
            continue;
        }
        argv[arg++] = &string[index];
        while ((string[index]!=' ') && (string[index]!='\0'))
            index++;
        string[index++] = 0;
    }
    return;
}

void getstring(char* string, uint32_t limit) {
    uint32_t x=0;
    int c;
    while (1) {
        c = getchar();
        if (c=='\b') {
            if (x) {
                x--;
            }
        } else if (c=='\n') break;
        else if (x<(limit-1)) {
            string[x] = c;
            x++;
        }
    }
    string[x] = 0x00;
    return;
}
