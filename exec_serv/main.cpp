#include "../socket.hpp"
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h> // queue handling
#include "request.hpp"
#include <vector>
#define MAX_EVENTS 5

std::string first_dispatch(char *msg, int fd, std::vector<std::string> cat){
    Request R;
// TO DO check http version to match
    if (msg[0] == 'G' && msg[1] == 'E' && msg[2] == 'T')
        R.handle_get(msg, fd, cat);
    else if (msg[0] == 'D' && msg[1] == 'E' && msg[2] == 'L' && msg[3] == 'E' && msg[4] == 'T' && msg[5] == 'E')
        R.handle_delete(msg);
    else if (msg[0] == 'P' && msg[1] == 'O' && msg[2] == 'S' && msg[3] == 'T')
        R.handle_post(msg, fd, cat);
    else if (msg == NULL)
        R.set_error(400);
    else
        R.set_error(405);
    return R.response;
}

int launch(std::vector<std::string> cat){
    //create server
// TO DO proper error handling without the big ass try catch
    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS];
    int event_count;
    // crea de la socket
    Socket serv(1);
    // take port and address style (here ipv4)
    char *check = serv.create_socket(8080, INADDR_ANY);
    if (check != NULL){
        std::cout << check << std::endl;
        return 1;
    }
    check = serv.server_binding();
    if (check != NULL){
        std::cout << check << std::endl;
        return 1;
    }
// TO DO include socket in queue
    check = serv.server_listening(10);
    if (check != NULL){
        std::cout << check << std::endl;
        return 1;
    }
    while (1){
        int new_sock = serv.server_accept();
        if (new_sock == -1){
            std::cout << "Error: failure to accept in server_socket" << std::endl;
            return 1;
        }
        char buffer[30000] = {0};
        // crea du systeme de gestion de requetes
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
            if (valread == -1)
                return 0;
            // gestion requete
            std::string response = first_dispatch(buffer, events[i].data.fd, cat);
        response.append("done");
            write(events[i].data.fd , response.c_str(), response.size());
            std::cout << "percy" << std::endl;
            std::vector<std::string>::iterator test = cat.begin();
            while (test != cat.end()){
                std::cout << *test << std::endl;
                test++;
            }
        }
        close(queue);
        close(new_sock);
    }
    return 0;
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
        // char **new_env;
        std::vector<std::string> cat;
                    std::vector<std::string>::iterator test = cat.begin();
            while (test != cat.end()){
                std::cout << *test << std::endl;
                test++;
            }
        cat.push_back("CHATON");
        cat.push_back("KERO");
        return launch(cat);
    // return 0;
}