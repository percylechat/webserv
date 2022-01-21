#include "../socket.hpp"
#include <stdio.h>
#include <unistd.h>

int main(){
    //create server
    try{
        // Socket serv(AF_INET, SOCK_STREAM, 0, 3000, INADDR_ANY, 1);
        Socket serv(8080, INADDR_ANY, 1);
        serv.server_binding();
        serv.server_listening(10);
        char hello[] = "bites from server";
        std::cout << "help" << std::endl;
        while (1){
            int new_sock = serv.server_accept();
                    std::cout << "help" << std::endl;
            char buffer[30000] = {0};
            long valread = read( new_sock , buffer, 30000);
                    std::cout << "help" << std::endl;
            if (valread == 0)
                return 0;
            printf("%s\n",buffer );
            write(new_sock , hello , 18);
            printf("------------------Hello message sent-------------------\n");
            close(new_sock);
            }
        }
    catch (Socket::failed_socket &e){
        std::cerr << e.what() << std::endl;
        return (0);
    }
    return 1;
}