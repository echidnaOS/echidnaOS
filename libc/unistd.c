#include <unistd.h>
#include <sys_api.h>

pid_t fork(void) {
    return (pid_t)OS_fork();
}
