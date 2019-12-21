#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define SOCK_PATH "/tmp/preload_sock"

static void *(*callocp)(size_t count, size_t size);
static void *(*reallocp)(void *ptr, size_t size);
static void *(*mallocp)(size_t size);
static void *(*freep)(void *);

static int sock;

void __attribute__ ((constructor)) init(void);
void __attribute__ ((destructor)) fini(void);

void init(void) {
    struct sockaddr_un remote;
    int len;
    pid_t pid;
    char pidbuf[8] = {0};

    reallocp = dlsym(RTLD_NEXT, "realloc");
    mallocp  = dlsym(RTLD_NEXT, "malloc");
    callocp  = dlsym(RTLD_NEXT, "calloc");
    freep    = dlsym(RTLD_NEXT, "free"); 

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(sock, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }

    // first thing we write on the socket is our PID
    pid = getpid();
    snprintf(pidbuf, sizeof(pidbuf), "%d\n", pid);
    write(sock, pidbuf, sizeof(pidbuf));
}

void fini(void) {
    write(sock, "FIN\n", 4);
    close(sock);
}

void *realloc(void *ptr, size_t size)
{
    void *tmp;
    char buf[1024];
    int len;

    tmp = ptr;
    ptr = reallocp(ptr, size);
    len = snprintf(buf, sizeof(buf), "0x%lx:realloc(0x%lx, %zu)\t\t\t\t = 0x%lx\n", (unsigned long)__builtin_return_address(1),
            (unsigned long)tmp, size, (unsigned long)ptr);
    write(sock, buf, len);
    return ptr;
 
}

void *calloc(size_t count, size_t size)
{
    void *ptr;
    char buf[1024];
    int len;

    ptr = callocp(count, size);
    len = snprintf(buf, sizeof(buf), "0x%lx:calloc(%zu, %zu)\t\t\t\t = 0x%lx\n", (unsigned long)__builtin_return_address(1),
            count, size, (unsigned long)ptr);
    write(sock, buf, len);
    return ptr;
}

void *malloc(size_t size)
{
    void *ptr;
    char buf[1024];
    int len;

    ptr = mallocp(size);
    len = snprintf(buf, sizeof(buf), "0x%lx:malloc(%zu)\t\t\t\t = 0x%lx\n", (unsigned long)__builtin_return_address(0),
            size, (unsigned long)ptr);
    write(sock, buf, len);
    return ptr;
}

void free(void *ptr)
{
    char buf[1024];
    int len;
   
    len = snprintf(buf, sizeof(buf), "0x%lx:free(0x%lx)\t\t\t\t = <void>\n", (unsigned long)__builtin_return_address(1),
            (unsigned long)ptr);
    write(sock, buf, len);
    freep(ptr);
}
