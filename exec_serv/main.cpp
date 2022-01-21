#include "../socket.hpp"
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h> // queue handling
#define MAX_EVENTS 5
int main(){
    //create server
    try{
        // Socket serv(AF_INET, SOCK_STREAM, 0, 3000, INADDR_ANY, 1);
        // struct epoll_event event;
        struct epoll_event event, events[MAX_EVENTS];
        int event_count;
        char hello[] = "bites from server";
        
        Socket serv(8080, INADDR_ANY, 1);
        serv.server_binding();
        serv.server_listening(10);
        while (1){
            int new_sock = serv.server_accept();
                        char buffer[30000] = {0};

            int queue = epoll_create1(0); // takes flag so none for now
            if (queue == -1){
                std::cout << "wtf epoll1" << std::endl;
                return 1;
            }
            event.events = EPOLLIN;
            // event.data.fd = 0; //<< PB?
            event.data.fd = new_sock;
            if (epoll_ctl(queue, EPOLL_CTL_ADD, new_sock, &event) == -1)
            {
                std::cout << "Failed to add file descriptor to epoll\n" << std::endl;
                close(queue);
                return 1;
            }
            event_count = epoll_wait(queue, events, MAX_EVENTS, 30000);
            std::cout << "events: " << event_count << std::endl;
            for(int i = 0; i < event_count; i++)
            {
                std::cout << "Reading file descriptor" << events[i].data.fd << std::endl;
                long valread = read(events[i].data.fd, buffer, 30000);
                std::cout << "percy" << std::endl;
                if (valread == -1)
                    return 0;
                // bytes_read = read(events[i].data.fd, read_buffer, READ_SIZE);
                // printf("%zd bytes read.\n", bytes_read);
                // read_buffer[bytes_read] = '\0';
                std::cout << buffer << std::endl;
                write(events[i].data.fd , hello , 18);
                // printf("------------------Hello message sent-------------------\n");
                std::cout << "percy" << std::endl;
                // if(!strncmp(read_buffer, "stop\n", 5))
                //     running = 0;
            }

            // char buffer[30000] = {0};
            // long valread = read( new_sock , buffer, 30000);
            // // long valread = read(events[i].data.fd, buffer, 30000);
            //         std::cout << "help" << std::endl;
            // if (valread == 0)
            //     return 0;
            // printf("%s\n",buffer );
            // write(new_sock , hello , 18);
            // printf("------------------Hello message sent-------------------\n");
                close(queue);
                close(new_sock);
            }
        }
    catch (Socket::failed_socket &e){
        std::cerr << e.what() << std::endl;
        return (0);
    }
    return 1;
}