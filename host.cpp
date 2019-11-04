#include <iostream>
#include <unistd.h>
#include <stdlib.h>
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

using namespace std;


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
    int num_threads = 0;
    pthread_t * threads = NULL;
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
    while(true){
        int client_sock = accept(sock, NULL, NULL);
	cout << "New client" << endl;
        num_threads += 1;
        threads = (pthread_t*)realloc(threads, 
				sizeof(pthread_t) * (size_t)num_threads);
        pthread_create(threads + (num_threads-1), NULL, &handleConnection, (void*)(size_t)client_sock);
    }
}
