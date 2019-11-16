#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <pthread.h>
#include <errno.h>
#include <list>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "thread.cpp"
#include "queue.cpp"

#define THREAD_POOL_SIZE 10

using namespace std;

static vector<Queue*> taskQueues;

static pthread_t threadPool[THREAD_POOL_SIZE];

static list<int> socketList;

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
        Queue *q = new Queue(-1);
        taskQueues.push_back(q);
    }
    for(size_t i = 0; i < THREAD_POOL_SIZE; i++){
        pthread_create(threadPool + i, NULL,
              &handleConnection, (void*)taskQueues[i]);
    }

    struct addrinfo addr, *result;
    memset(&addr, 0, sizeof(struct addrinfo));
    addr.ai_family = AF_INET;
    addr.ai_socktype = SOCK_STREAM;
    addr.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, "5005", &addr, &result);
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);
    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    bind(sock, result->ai_addr, result->ai_addrlen);
    listen(sock, 20);
    cout << "Listening" << endl;

    size_t count = 0;
    while (true) {
        //accept new client
        int client_sock = accept4(sock, NULL, NULL, SOCK_NONBLOCK);
        if (client_sock != -1){
            cout << "New client" << endl;
            socketList.push_back(client_sock);
            break;
        }
    }
    while (true) {
        //receive msg
        for (list<int>::iterator it = socketList.begin(); it != socketList.end();) {
          string msg(1024, 0);
          // ssize_t bytesRead = recv(*it, &msg[0], 1024-1, MSG_DONTWAIT);
          ssize_t bytesRead = recv(*it, &msg[0], 1024-1, 0);
          if (bytesRead > 0){
              cout<<"Msg received: "<<msg<<endl;
              Task *t = new Task{*it, msg};
              taskQueues[count]->push(t);
              count = (count + 1) % THREAD_POOL_SIZE;

              ++it;
          } else if (bytesRead == 0){// && errno == ENOTSOCK){
              //it = socketList.erase(it);
          }
        }
    }
}
