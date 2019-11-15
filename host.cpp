#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <pthread.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#include "thread.cpp"
#include "queue.h"

#define THREAD_POOL_SIZE 10

using namespace std;

typedef struct{
    string msg;
    int sock;
} Task;

typedef struct{
    SockLinkedListNode *last, *next;
    int sock;
} SockLinkedListNode;

/**
 *  Use taskQueue[threadNumber].pop() to get Task, where threadNumber is the
 *  parameter passed when creating a working thread.
 */
static vector taskQueues = NULL;

static pthread_t *threadPool = NULL;
static SockLinkedListNode *head = NULL;

void addSocket(int sock){
    SockLinkedListNode *node = malloc(sizeof(SockLinkedListNode));
    node->sock = sock;
    node->last = NULL;
    node->next = head;
    head = node;
}

void deleteSocket(SockLinkedListNode *node){
    if(node == head){
        head = node->next;
    } else {
        if(node->last) node->last->next = node->next;
        if(node->next) node->next->last = node->last;
    }
    free(node);
}

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

int main(){
    printHostName();
    printLocalIpAddr();
    
    for(size_t i = 0; i < THREAD_POOL_SIZE; i++){
        taskQueues.push_back(Queue(-1)); 
    }
    pthread_t * threads = malloc(sizeof(pthread_t) * THREAD_POOL_SIZE);
    for(size_t i = 0; i < THREAD_POOL_SIZE; i++){
        pthread_create(threads + i, NULL, &handleConnection, (void*)i);
    }

    struct addrinfo addr, *result;
	memset(&addr, 0, sizeof(struct addrinfo));
    addr.ai_family = AF_INET;
    addr.ai_socktype = SOCK_STREAM;
    addr.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, "5005", &addr, &result);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    bind(sock, result->ai_addr, result->ai_addrlen);
    listen(sock, 20);
    cout << "Listening" << endl;

    SockLinkedListNode *p, *p_next;
    size_t count = 0;
    while(true){
        //accept new client
        int client_sock = accept4(sock, NULL, NULL, SOCK_NONBLOCK);
        if (client_sock != -1){
            cout << "New client" << endl;
            addSocket(client_sock);
        }
        //receive msg
        p = head;
        while(p){
            p_next = p->next;
            string msg(1024, 0);
            ssize_t bytesRead = recv(socket, &msg[0], 1024-1, MSG_DONTWAIT);
            if(bytesRead > 0){
                Task t = malloc(sizeof(Task));
                t->sock = p->sock;
                t->msg = msg;
                taskQueues[count].push(t);
                count = (count + 1) % THREAD_POOL_SIZE;
            } else if(bytesRead == -1 && errno == ENOTSOCK){
                deleteSocket(p);
            }
            p = p_next;
        }
    }
}
