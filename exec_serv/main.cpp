#include "../socket.hpp"
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h> // queue handling
#include "request.hpp"
#define MAX_EVENTS 5

std::string first_dispatch(char *msg){
    Request R;
    if (msg[0] == 'G' && msg[1] == 'E' && msg[2] == 'T')
        R.handle_get(msg);
    // else if (msg[0] == 'D' && msg[1] == 'E')
    //     R.handle_delete(msg);
    // else if (msg[0] == 'P' && msg[1] == 'O' && msg[2] == 'S' && msg[3] == 'T')
    //     R.handle_post(msg);
    else
        R.set_error(400);
    return R.response;
}

int launch(){
    //create server
    try{
        // Socket serv(AF_INET, SOCK_STREAM, 0, 3000, INADDR_ANY, 1);
        // struct epoll_event event;
        struct epoll_event event;
        struct epoll_event events[MAX_EVENTS];
        int event_count;
        // char hello[] = "<html><body><p>Kikou</p></body></html>";
        
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
                // ICI INTEGRER TRAITEMENT REQUETE CLIENT
                std::cout << "percy" << std::endl;
                if (valread == -1)
                    return 0;
                std::cout << buffer << std::endl;
                std::string test = first_dispatch(buffer);
                std::cout <<  test << std::endl;

                write(events[i].data.fd , test.c_str(), test.size());
                
                std::cout << "percy" << std::endl;
                write(2, test.c_str(), test.size());
            }
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

int main(){//(int argc, char *argv[]){
    // if (argc != 2)
    //     std::cout << "Error: no config file specified" << std::endl;
    // else
        // Config conf();
        // conf.checkfile(argv[1]);
        // if (conf.get_status() == 1){
        //     std::cerr << conf.get_status_error() << std::endl;
        //     return 1;
        // }
        // conf.filup();
        // if (conf.getstatus() == 1){
        //     std::cerr << conf.getstatuserror() << std::endl;
        //     return 1;
        // }
        return launch();
    // return 0;
}