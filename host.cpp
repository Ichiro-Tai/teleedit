#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "thread.cpp"
using namespace std;
int main(){
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
    while(true){
        int client_sock = accept(sock, NULL, NULL);
        num_threads += 1;
        threads = (pthread_t*)realloc(threads, 
				sizeof(pthread_t) * (size_t)num_threads);
        pthread_create(threads + (num_threads-1), NULL, &handleConnection, (void*)(size_t)client_sock);
    }

}
