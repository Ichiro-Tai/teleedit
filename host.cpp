#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <pthread.h>
#include <errno.h>
#include <list>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "thread.cpp"
#include "queue.cpp"

#define THREAD_POOL_SIZE 10
#define MAX_CLIENTS (128)

using namespace std;

/**
 *  Use taskQueue[threadNumber].pop() to get Task, where threadNumber is the
 *  parameter passed when creating a working thread.
 */

static int exit_server = 0;

static int server_socket;

static pthread_t threadPool[THREAD_POOL_SIZE];

static list<int> socketList;



static Queue *task_queue;
static int epoll_fd;

void printHostName(){
    char buffer[256];
    gethostname(buffer, sizeof(buffer));
    cout << "Hostname: " << buffer << endl;
}

/**
    Reference: https://stackoverflow.com/questions/2146191/obtaining-local-ip-address-using-getaddrinfo-c-function
*/
void printLocalIpAddr(){
    struct ifaddrs *myaddrs, *ifa;
    void *in_addr;
    char buf[64];
    if(getifaddrs(&myaddrs) != 0){
        perror("getifaddrs");
        exit(1);
    }
    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next){
        if (ifa->ifa_addr == NULL) continue;
        if (!(ifa->ifa_flags & IFF_UP)) continue;
        switch (ifa->ifa_addr->sa_family){
            case AF_INET:
            {
                struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                in_addr = &s4->sin_addr;
                break;
            }
            case AF_INET6:
            {
                struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
                in_addr = &s6->sin6_addr;
                break;
            }
            default:
                continue;
        }
        if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf))){
            printf("%s: inet_ntop failed!\n", ifa->ifa_name);
        } else {
            printf("%s: %s\n", ifa->ifa_name, buf);
        }
    }
    freeifaddrs(myaddrs);
}

void signal_int(int signum){
    exit_server = 1;
}

void cleanup(){
    shutdown(server_socket, SHUT_RDWR);
    close(server_socket);
}

int main(){
    //Print host info
    printHostName();
    printLocalIpAddr();
    //Init server socket
    struct addrinfo addr, *result;
    memset(&addr, 0, sizeof(struct addrinfo));
    addr.ai_family = AF_INET;
    addr.ai_socktype = SOCK_STREAM;
    addr.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, "5005", &addr, &result);
    server_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    bind(server_socket, result->ai_addr, result->ai_addrlen);
    listen(server_socket, MAX_CLIENTS);
    cout << "Listening" << endl;   
    //Epoll
    epoll_fd = epoll_create1(0);
    if(epoll_fd == -1){perror("epoll_create1"); exit(-1);}
    struct epoll_event ev;
    memset(&ev, 0, sizeof(struct epoll_event));
    ev.events = EPOLLIN;
    ev.data.fd = server_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &ev) == -1) {
        perror("epoll_ctl: server_socket");
        exit(-1);
    }
    struct epoll_event *events = calloc(MAX_CLIENTS, sizeof(struct epoll_event));
    //Init working threads
    task_queue = new Queue(-1);
    thread_starter_kit work_thread_param;
    work_thread_param.taskQueue = task_queue;
    work_thread_param.epoll_fd = epoll_fd;
    for(size_t i = 0; i < THREAD_POOL_SIZE; i++){
        pthread_create(threadPool + i, NULL,
              &handleConnection, (void*)&work_thread_param);
    }
    while (exit_server == 0) {
        int nFds = epoll_wait(epollFd, events, MAX_CLIENTS, -1);
        for (int i = 0; i < nFds; i++){
            if(exit_server) break;
            int sock = events[i].data.fd;
            if(sock == server_sock){ //accept new client
                //accept
                socklen_t addrlen = sizeof(struct sockaddr);
                struct sockaddr addr;
                memset(&addr, 0, addrlen);
                int client_sock = accept(server_sock, &addr, &addrlen);
                int flags = fcntl(client_sock, F_GETFL, 0);
                fcntl(client_sock, F_SETFL, flags | O_NONBLOCK);
                //epoll
                memset(&ev, 0, sizeof(struct epoll_event));
                ev.events = EPOLLIN;
                ev.data.fd = client_sock;
                epoll_ctl(epollFd, EPOLL_CTL_ADD, client_sock, &ev);
            } else { // client has new message
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock, NULL);
                Task *t = new Task{sock};
                task_queue->push(t);
            }
        }
    }
    cleanup();
}
