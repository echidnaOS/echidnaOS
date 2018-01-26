#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys_api.h>

int get_argc(const char* string);
void get_argv(char** argv, char* string);

char prog_path[128];
char prog_stdin[128];
char prog_stdout[128];
char prog_stderr[128];
char prog_pwd[128];

int pid;

extern char **environ;

task_info_t prog_info = {
    prog_path,
    prog_stdin,
    prog_stdout,
    prog_stderr,
    prog_pwd,
    0,
    0,
    0,
    0,
    0
};

int no_block = 0;

// built in shell

int main(int argc, char** argv) {
    char input[256];
    int s_argc;
    char* s_argv[128];
    char pwd[2048];
    
    int c;
    uint64_t i;
    
    vfs_metadata_t metadata = {0};

    for (;;) {
        printf("\e[32mechidnaOS\e[37m:\e[36m%s\e[37m# ", getwd(pwd));
        fflush(stdout);

        fgets(input, 256, stdin);
        input[strlen(input) - 1] = 0;
        s_argc = get_argc(input);
        get_argv(s_argv, input);

        if (!strcmp("heap", s_argv[0])) {
            printf("heap base: %d\n"
                   "heap size: %d\n", OS_get_heap_base(), OS_get_heap_size());
            OS_resize_heap(0x10000);
        }

        else if (!strcmp("col", s_argv[0]))
            puts("\e[40m \e[41m \e[42m \e[43m \e[44m \e[45m \e[46m \e[47m \e[40m");
        
        else if (!strcmp("pid", s_argv[0])) {
            printf("pid: %d\n", OS_getpid());
        }
        
        else if (!strcmp("read", s_argv[0])) {
            char buf[6] = {0};
            if (s_argc == 1) continue;
            int handle = OS_open(s_argv[1], ECH_O_RDONLY, 0);
            if (handle == -1) fputs("couldn't open file\n", stderr);
            printf("received handle %d\n", handle);
            if (OS_read(handle, buf, 5) == -1)
                printf("couldn't read\n");
            else
                printf("read 5 bytes\n");
            puts(buf);
            if (OS_close(handle) == -1)
                printf("couldn't close\n");
            else
                printf("handle closed\n");
        }
        
        else if (!strcmp("write", s_argv[0])) {
            char buf[6] = "12345";
            if (s_argc == 1) continue;
            int handle = OS_open(s_argv[1], ECH_O_WRONLY | ECH_O_CREAT | ECH_O_TRUNC, 0);
            if (handle == -1) fputs("couldn't open file\n", stderr);
            printf("received handle %d\n", handle);
            if (OS_write(handle, buf, 5) == -1)
                printf("couldn't write\n");
            else
                printf("wrote 5 bytes\n");
            puts(buf);
            if (OS_close(handle) == -1)
                printf("couldn't close\n");
            else
                printf("handle closed\n");
        }
        
        else if (!strcmp("div0", s_argv[0])) {/*
            int a = 0;
            a = a / 0;
*/        }
        
        else if (!strcmp("esc", s_argv[0])) {
            if (s_argc == 1) continue;
            putchar('\e');
            fputs(s_argv[1], stdout);
        }
        
        else if (!strcmp("mkdir", s_argv[0])) {
            if (s_argc == 1) continue;
            if (OS_vfs_mkdir(s_argv[1], 0))
                fprintf(stderr, "couldn't create directory `%s`.\n", s_argv[1]);
        }
        
        else if (!strcmp("touch", s_argv[0])) {
            if (s_argc == 1) continue;
            if (OS_vfs_create(s_argv[1], 0))
                fprintf(stderr, "couldn't create file `%s`.\n", s_argv[1]);
        }
        
        else if (!strcmp("rm", s_argv[0])) {
            if (s_argc == 1) continue;
            if (OS_vfs_remove(s_argv[1]))
                fprintf(stderr, "couldn't remove file `%s`.\n", s_argv[1]);
        }
        
        else if (!strcmp("clear", s_argv[0])) {
            fputs("\e[2J\e[H", stdout);
        }
        
        else if (!strcmp("fork", s_argv[0])) {
            
            pid_t pid = fork();

            if (pid == -1) {
                puts("Fork fail.");
            }
            else if (pid == 0) {
                puts("Child process OK.");
            }
            else {
                printf("Child PID: %d\n", pid);
            }

        }
        
        else if (!strcmp("ls", s_argv[0])) {
            char* ls_path;
            if (s_argc == 1) ls_path = pwd;
            else ls_path = s_argv[1];
            for (int i = 0; ; i++) {
                if (OS_vfs_list(ls_path, &metadata, i) == -2) break;
                if (metadata.filetype == 1) fputs("\e[36m", stdout);
                if (metadata.filetype == 2) fputs("\e[33m", stdout);
                printf("%s", metadata.filename);
                fputs("\e[37m", stdout);
                putchar('\n');
            }
        }
        
        else if (!strcmp("beep", s_argv[0])) {
            if (s_argc == 1) continue;
            OS_vfs_write("/dev/pcspk", 0, atoi(s_argv[1]));
        }
        
        else if (!strcmp("rdspk", s_argv[0]))
            printf("%d\n", OS_vfs_read("/dev/pcspk", 0));
        
        else if (!strcmp("exit", s_argv[0])) return 0;
        
        else if (!strcmp("dump", s_argv[0])) {
            FILE* fp;
            if (s_argc == 1) continue;
            if (!(fp = fopen(s_argv[1], "r"))) {
                fprintf(stderr, "can't open `%s`.\n", s_argv[1]);
                continue;
            }
            while ((c = fgetc(fp)) != EOF)
                putchar(c);
            printf("\nFile length: %d\n", (int)ftell(fp));
            fclose(fp);
        }
        
        else if (!strcmp("size", s_argv[0])) {
            FILE* fp;
            if (s_argc == 1) continue;
            if (!(fp = fopen(s_argv[1], "r"))) {
                fprintf(stderr, "can't open `%s`.\n", s_argv[1]);
                continue;
            }
            fseek(fp, 0L, SEEK_END);
            printf("\nFile length: %d\n", (int)ftell(fp));
            fclose(fp);
        }
        
        else if (!strcmp("dumpr", s_argv[0])) {
            FILE* src;
            FILE* dest;
            if (s_argc < 3) continue;
            if (!(src = fopen(s_argv[1], "rb"))) {
                fprintf(stderr, "can't open `%s`.\n", s_argv[1]);
                continue;
            }
            if (!(dest = fopen(s_argv[2], "rb+"))) {
                fprintf(stderr, "can't open `%s`.\n", s_argv[2]);
                continue;
            }
            while ((c = fgetc(src)) != EOF)
                fputc(c, dest);
            fclose(dest);
            continue;
        }
        
        else if (!strcmp("cd", s_argv[0])) {
            if (s_argc == 1) continue;
            if (OS_vfs_cd(s_argv[1]) == -2)
                fprintf(stderr, "shell: invalid directory: `%s`.\n", s_argv[1]);
        }
        
        else if (!strcmp("execve", s_argv[0])) {
            if (s_argc == 1) continue;
            if (OS_execve(s_argv[1], &s_argv[2], environ) == -1)
                fprintf(stderr, "shell: invalid file: `%s`.\n", s_argv[1]);
        }

        // return to prompt if no input
        else if (!input[0]) continue;

        // if the input did not match any command
        else {
            strcpy(prog_path, s_argv[0]);
            OS_pwd(prog_pwd);
            prog_info.argc = s_argc;
            prog_info.argv = s_argv;
            prog_info.environ = environ;
            if (!no_block) {
                if (OS_general_execute_block(&prog_info) == -1)
                    fprintf(stderr, "shell: invalid command: `%s`.\n", input);
            } else {
                if ((pid = OS_general_execute(&prog_info)) == -1)
                    fprintf(stderr, "shell: invalid command: `%s`.\n", input);
                else
                    fprintf(stderr, "PID %d launched.\n", pid);
            }
        }
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
        if (!strncmp(&string[index], "2>", 2) || !strncmp(&string[index], "&>", 2)) {
            index += 2;
            while (string[index] == ' ')
                index++;
            while ((string[index] != ' ') && (string[index]))
                index++;
            continue;
        }
        if (string[index] == '>' || string[index] == '<') {
            index++;
            while (string[index] == ' ')
                index++;
            while ((string[index] != ' ') && (string[index]))
                index++;
            continue;
        }
        if (string[index] == '&') {
            index++;
            continue;
        }
        while ((string[index] != ' ') && (string[index]))
            index++;
        argc++;
    }
    //printf("argc: %d\n", argc);
    return argc;
}

void get_argv(char** argv, char* string) {
    uint32_t index=0;
    uint8_t arg=0;
    
    no_block = 0;
    
    OS_what_stdin(prog_stdin);
    OS_what_stdout(prog_stdout);
    OS_what_stderr(prog_stderr);

    if (!*string) {
        argv[0] = string;
        return;
    }
    
    while (string[index]) {
        if (string[index] == ' ') {
            string[index++] = 0;
            continue;
        }
        if (string[index] == '>') {
            int i = 0;
            string[index++] = 0;
            //puts("> parsed");
            while (string[index] == ' ')
                string[index++] = 0;
            while ((string[index] != ' ') && (string[index])) {
                prog_stdout[i++] = string[index];
                string[index++] = 0;
            }
            prog_stdout[i] = 0;
            remove(prog_stdout);
            if (OS_vfs_create(prog_stdout, 0) == VFS_FAILURE) {
                // error handler
                //fprintf(stderr, "error: couldn't create %s.\n", prog_stdout);
            } else {
                //fprintf(stderr, "creating stdout %s\n", prog_stdout);
            }
            continue;
        }
        if (string[index] == '<') {
            int i = 0;
            vfs_metadata_t metadata;
            string[index++] = 0;
            //puts("< parsed");
            while (string[index] == ' ')
                string[index++] = 0;
            while ((string[index] != ' ') && (string[index])) {
                prog_stdin[i++] = string[index];
                string[index++] = 0;
            }
            prog_stdin[i] = 0;
            if (OS_vfs_get_metadata(prog_stdin, &metadata, VFS_FILE_TYPE) == VFS_FAILURE &&
                OS_vfs_get_metadata(prog_stdin, &metadata, VFS_DEVICE_TYPE) == VFS_FAILURE) {
                // error handler
                //fprintf(stderr, "error: couldn't access %s.\n", prog_stdin);
            } else {
                //fprintf(stderr, "reading %s\n", prog_stdin);
            }
            continue;
        }
        if (!strncmp(&string[index], "2>", 2)) {
            int i = 0;
            string[index++] = 0;
            string[index++] = 0;
            //puts("2> parsed");
            while (string[index] == ' ')
                string[index++] = 0;
            while ((string[index] != ' ') && (string[index])) {
                prog_stderr[i++] = string[index];
                string[index++] = 0;
            }
            prog_stderr[i] = 0;
            remove(prog_stderr);
            if (OS_vfs_create(prog_stderr, 0) == VFS_FAILURE) {
                // error handler
                //fprintf(stderr, "error: couldn't create %s.\n", prog_stderr);
            } else {
                //fprintf(stderr, "creating stderr %s\n", prog_stderr);
            }
            continue;
        }
        if (!strncmp(&string[index], "&>", 2)) {
            int i = 0;
            string[index++] = 0;
            string[index++] = 0;
            //puts("&> parsed");
            while (string[index] == ' ')
                string[index++] = 0;
            while ((string[index] != ' ') && (string[index])) {
                prog_stdout[i] = string[index];
                prog_stderr[i++] = string[index];
                string[index++] = 0;
            }
            prog_stdout[i] = 0;
            prog_stderr[i] = 0;
            remove(prog_stdout);
            remove(prog_stderr);
            if (OS_vfs_create(prog_stdout, 0) == VFS_FAILURE) {
                // error handler
                //fprintf(stderr, "error: couldn't create stdout %s.\n", prog_stdout);
            } else {
                //fprintf(stderr, "creating stdout %s\n", prog_stdout);
            }
            if (OS_vfs_create(prog_stderr, 0) == VFS_FAILURE) {
                // error handler
                //fprintf(stderr, "error: couldn't create stderr %s.\n", prog_stderr);
            } else {
                //fprintf(stderr, "creating stderr %s\n", prog_stderr);
            }
            continue;
        }
        if (string[index] == '&') {
            string[index++] = 0;
            no_block = 1;
            continue;
        }
        
        // point the argv argument
        argv[arg++] = &string[index];
        
        // skip over string and set 0 terminator
        while ((string[index] != ' ') && (string[index]))
            index++;
        if (string[index])
            string[index++] = 0;
        else break;
    }

    argv[arg] = (char *)0;

    return;
}
