#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <pthread.h>
int main(){
    int num_threads = 0;
    pthread_t * threads = NULL;
    struct sockaddr addr, *result;
    int sock = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
    addr.ai_family = AF_INET;
    addr.ai_socktype = SOCK_STREAM;
    addr.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, "5005", &addr, &result);
    connect(sock, result->ai_addr, result->ai_addrlen);
    freeaddrinfo(retult);
    bind(sock, result->ai_addr, result->ai_addrlen);
    while(true){
        int client_sock = accept(sock, NULL, NULL);
        num_threads += 1;
        threads = realloc(sizeof(pthread_t) * num_threads);
        pthread_create(threads + (num_threads-1), NULL, handleConnection, (void*)client_sock);
    }
    
}
