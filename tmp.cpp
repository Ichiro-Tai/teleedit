#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
int main(){
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
        cout << accept(sock, NULL, NULL) << endl;
    }
}