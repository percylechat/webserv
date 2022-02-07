#include "../socket.hpp"
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h> // queue handling

int main(){
    char hello[] = "Hello from client";
    char buffer[1024] = {0};
    try {
        Socket client(8080, "127.0.0.1", 0);
        client.client_connect();
        client.client_send(hello, 18);
        std::cout << "Hello message sent\n" << std::endl;
        long valread = read(client.get_fd() , buffer, 1024);
        if (valread == 0)
            return 0;
        printf("%s\n",buffer );
    }
    catch (Socket::failed_socket &e){
        std::cerr << e.what() << std::endl;
        return (0);
    }
    return 1;
}
