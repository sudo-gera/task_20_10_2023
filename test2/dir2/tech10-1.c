#ifndef assert
#include <assert.h>
#endif
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <iso646.h>
#include <memory.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <tgmath.h>
#include <unistd.h>

#ifdef print
#undef print
#endif

#ifdef write
#undef write
#endif

struct array_s {
    size_t mem_size;
    size_t el_count;
    char data[0];
};

static inline size_t len(void *a) {
    if (a == NULL) {
        return 0;
    }
    return ((struct array_s *)(a))[-1].el_count;
}

static inline void del(void *a) {
    if (a != NULL) {
        free(((struct array_s *)(a)) - 1);
    }
}

static inline struct array_s *resize_f(struct array_s **vp, size_t el_size, size_t n) {
    if (*vp == NULL) {
        *vp = (struct array_s *)calloc(1, sizeof(struct array_s));
        assert(*vp);
        *vp += 1;
    }
    struct array_s *a = *vp - 1;
    assert(a->data == *(char **)vp);
    if (a->mem_size < n + 1) {
        size_t cur_size = a->mem_size * el_size;
        size_t new_size;
        if (a->mem_size * 2 > n) {
            new_size = a->mem_size * 2 * el_size;
        } else {
            new_size = (n + 1) * el_size;
        }
        a = (struct array_s *)realloc(a, sizeof(struct array_s) + new_size);
        assert(a);
        memset(a->data + a->mem_size * el_size, 0, new_size - cur_size);
        a->mem_size = new_size / el_size;
    }
    a->el_count = n;
    *vp = a + 1;
    return a + 1;
}
/////// resize(a, n) is resize_f(&a, sizeof(a[0]), n)
#ifdef __cplusplus
    template<typename T>
    auto resize(const T& a,uint64_t n){
        return (resize_f((struct array_s **)&(a), sizeof((a)[0]), (n)));
    }
    template<typename T,typename Y>
    auto append(const T& a,const Y& s){
        return (resize((a), len(a) + 1), (a)[len(a) - 1] = (s));
    }
    template<typename T>
    auto pop(const T& a){
        return (resize((a), len(a) - 1), (a)[len(a)]);
    }
    template<typename T>
    auto&back(const T& a){
        return ((a)[len(a)-1]);
    }
#else
    #define resize(a, ...) (resize_f((struct array_s **)&(a), sizeof((a)[0]), (__VA_ARGS__)))
    #define append(a, ...) (resize((a), len(a) + 1), (a)[len(a) - 1] = (__VA_ARGS__))
    #define pop(a) (resize((a), len(a) - 1), (a)[len(a)])
    #define back(a) ((a)[len(a)-1])
#endif

static inline int64_t getint() {
    int sign = 1;
    int c;
    size_t res = 0;
    while (c = getchar_unlocked(), isspace(c))
        ;
    if (c == '-') {
        sign = -1;
    } else {
        res = c - '0';
    }
    while (c = getchar_unlocked(), isdigit(c)) {
        res *= 10;
        res += c - '0';
    }
    return (int64_t)(res)*sign;
}

static inline void putint(uint64_t out) {
    if (out > (1LLU << 63) - 1) {
        putchar_unlocked('-');
        out = 1 + ~out;
    }
    char data[44];
    char *dend = data;
    while (out) {
        *++dend = (unsigned)('0') + out % 10;
        out /= 10;
    }
    if (dend == data) {
        putchar_unlocked('0');
    }
    for (; dend != data; --dend) {
        putchar_unlocked(*dend);
    }
}

static inline void print(uint64_t out) {
    putint(out);
    putchar('\n');
}

#define min(a,s) ((a)<(s)?(a):(s))
#define max(a,s) ((a)>(s)?(a):(s))

typedef int (*cmp_f_t)(const void *, const void *);

char* get_line(){
    char*str=0;
    int c=0;
    while ((c=getchar(),c!=EOF)){
        append(str,c);
    }
    return str;
}

#define elif else if

#define _str(x) #x
#define m_str(x) _str(x)

// #define perr if (errno){perror(__FILE__ " " m_str(__LINE__));errno=0;}
#define perr 

///////////////////////////////////////////////////end of lib

volatile int client_fd = -1;
volatile int socket_fd = -1;

void handler(int sig_num) {
    if (client_fd!=-1){
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
    }
    if (socket_fd!=-1){
        shutdown(socket_fd, SHUT_RDWR);
        close(socket_fd);
    }
    exit(0);
}

int main(int argc,char**argv) {

    struct sigaction sa = {
        .sa_handler = handler,
        .sa_flags = SA_RESTART,
    };
    
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 1) {
        perror("socket");
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
#if __has_include("d")
        .sin_port = htons(8889),
#else
        .sin_port = htons(atoi(argv[1])),
#endif
        .sin_addr.s_addr = INADDR_ANY,
    };

    bind(socket_fd, (struct sockaddr*)(&addr), sizeof(addr));

    listen(socket_fd, SOMAXCONN);
perr


    while(1){
        client_fd = accept(socket_fd, NULL, NULL);
perr
        char*str=0;
        char data[1024];
        int l=0;
        char* end=0;
        while ((l=recv(client_fd,data,1,0))){
            append(str,data[0]);
            if (len(str)>1 and str[len(str)-1]=='\n' and str[len(str)-2]=='\r'){
                end=str+len(str)-2;
                break;
            }
        }

        char*filestart=str+strlen("GET ");
        char*filestop=end-strlen(" HTTP/1.1");
        filestop[0]=0;

        if (access(filestart,F_OK)){
            const char*to_send="HTTP/1.1 404 Not Found\r\n";
            send(client_fd, to_send, strlen(to_send), 0);
perr
        }elif (access(filestart,R_OK)){
            const char*to_send="HTTP/1.1 403 Forbidden\r\n";
            send(client_fd, to_send, strlen(to_send), 0);
perr
        }else{
            const char*to_send="HTTP/1.1 200 OK\r\n";
            send(client_fd, to_send, strlen(to_send), 0);
perr
            char data[1024];
            struct stat stat_s;
            stat(filestart,&stat_s);
perr
            sprintf(data,"Content-Length: %d\r\n\r\n",(int)stat_s.st_size);
            send(client_fd, data, strlen(data), 0);
perr
            int fd=open(filestart,O_RDONLY);
perr
            while ((l=read(fd,data,sizeof(data)))){
                // print(l);
perr
                send(client_fd,data,l,0);
perr
            }
            close(fd);
perr
        }
        del(str);
        str=0;

        shutdown(client_fd, SHUT_RDWR);
perr
        close(client_fd);
perr
        client_fd=-1;
    }

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);

    return 0;
}