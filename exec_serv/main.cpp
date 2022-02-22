#include "../socket.hpp"
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h> // queue handling
// #include "request.hpp"
#include <vector>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include "../parsing_conf.hpp"
#include "request.hpp"

#define MAX_EVENTS 5

// GET /index.html HTTP/1.1
// Host: localhost:8080
// User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:97.0) Gecko/20100101 Firefox/97.0
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8
// Accept-Language: fr,fr-FR;q=0.8,en-US;q=0.5,en;q=0.3
// Accept-Encoding: gzip, deflate
// Connection: keep-alive
// Upgrade-Insecure-Requests: 1
// Sec-Fetch-Dest: document
// Sec-Fetch-Mode: navigate
// Sec-Fetch-Site: cross-site
// Cache-Control: max-age=0
struct location{
    std::string root;
    std::string index;
// no need
    std::vector<std::string> methods;
    bool autoindex;
    std::string cgi;
    std::string upload_dir;
    std::string redirect;
};
struct server{
    int port;
    std::string name;
    std::string error_page;
    int size;
    struct location *l;
};
struct bundle_for_response{
    int fd;
    int num_serv;
    request re;
};

const char *go_error(int err, server s){
    std::string response = "HTTP/1.1 ";
    if (s.error_page != "")
        //DO smthing
        ;
    else{
        if (err == 400)// for now for missing extension in file
            response.append("400 BAD REQUEST");
        else if (err == 404)// for now, couldn't open file so does not exist
            response.append("404 NOT FOUND");
        else if (err == 405)// is not GET POST or DELETE
            response.append("405 METHOD NOT ALLOWED");
        else if (err == 411)// content lenght missing
            response.append("411 LENGHT REQUIRED");
        else if (err == 500)// For now, couldn't delete file
            response.append("500 INTERNAL SERVER ERROR");
        else if (err == 505)// bad hhtp protocol version
            response.append("505 HTTP VERSION NOT SUPPORTED");
    }
    return response.c_str();
}

const char *get_response(request r, server s){
    r.status_is_handled = true;
    if (r.error_type != 200)
        return go_error(r.error_type, s);
    // TO DO need check which location
    // check_methods(&r, s);
    else{
        std::string response = "HTTP/1.1 200 OK \r\n hello";
        return response.c_str();
    }
}

int check_conn(int fd_1, Socket *serv, int nbr){
    int i = 0;
                std::cout << "hello" << std::endl;
    while (i < nbr){
        if (fd_1 == serv[i].get_fd())
            return i;
        i++;
    }
    return -1;
}

int get_fd_serv(int fd_1, std::vector<struct bundle_for_response> bfr){
    unsigned int k = 0;
    while (k < bfr.size()){
        if (fd_1 == bfr[k].fd)
            return k;
        k++;
    }
    return -1;
}

int fd_in_queue(int fd, int queue, int mode){
    //mode 1 EPOLLIN 2 EPOLLIN EPOLLRDHUP 3 EPOLLOUT
    struct epoll_event event;
    
    if (mode == 1)
        event.events = EPOLLIN;
    else if (mode == 2)
        event.events = EPOLLIN | EPOLLRDHUP;
    else
        event.events = EPOLLOUT;
    event.data.fd = fd;
    if (epoll_ctl(queue, EPOLL_CTL_ADD, fd, &event))
    {
        close(fd);
        return -1;
    }
    return 0;
}

void _handleReady(int epoll_fd, const int fd, struct epoll_event *event, Socket *serv, int nbr_sockets)
{
    int j = check_conn(fd, serv, nbr_sockets);
    Socket sock[1];
    sock[0].fd_sock = fd;

        if (j != -1)
        {
            int clientsocket = sock[0].server_accept();
            Socket test[1];
            test[0].fd_sock = clientsocket;
            if (fcntl(test[0].fd_sock, F_SETFL, O_NONBLOCK) < 0)
                return;
            if (fd_in_queue(test[0].fd_sock, epoll_fd, 2))
                return;
            // _add_fd_to_poll(epoll_fd, accepted, EPOLLIN | EPOLLRDHUP);
            // _requests[accepted] = std::make_pair(http::Request(), found->second);
        }
        else if (event->events & EPOLLRDHUP)
            // _removeAcceptedFD(sock);
            std::cout << "is stop" << std::endl;
        else if (event->events & EPOLLOUT)
            // _handleEpollout(sock, _requests[fd], event, epoll_fd);
            std::cout << "success" << std::endl;
        else if (event->events & EPOLLIN){
            int ret = 0;
            char buffer[300];
            bzero(buffer, sizeof(buffer));
            ret = recv(sock[0].fd_sock, buffer, sizeof(buffer), MSG_DONTWAIT);
            if (ret == 0)
                return ;
            else if (ret == -1)
                // CLIENT REMOVED IN THIS CASE
                return ;
            request re = first_dispatch(buffer);
            // data.first.parse(content, data.second.client_max_body_size);
            if (re.status_is_finished == true)
            {
                // std::cout << data.first << std::endl;
                event->events = EPOLLOUT;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock[0].fd_sock, event);
            }
        }
}

int launch(serverConf conf){
// TO DO think max event variable
    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS];
    unsigned int event_count;
    const char *check = NULL;
    // char buffer[300000] = {0}; // TO DO check buffer size for reading
    std::vector<struct bundle_for_response> bfr;

    std::string home = "127.0.0.1";
    // TO DO check error handling while creating queue
    int queue = epoll_create1(EPOLL_CLOEXEC);
    if (queue == -1){
        std::cout << "wtf epoll1" << std::endl;
        return 1;
    }
    int i = 0;
    int nbr_sockets = conf.http.size();
    Socket serv[nbr_sockets];
    while (i < nbr_sockets){
        check = serv[i].create_socket(conf.http.data()[i]["server"]["listen"][0], INADDR_ANY);
        if (check != NULL){
            std::cout << check << std::endl;
            return 1;
        }
        check = serv[i].server_binding();
        if (check != NULL){
            std::cout << check << std::endl;
            return 1;
        }
        if (fcntl(serv[i].get_fd(), F_SETFL, O_NONBLOCK) < 0){
            std::cout << "Error setting nonblocking socket" << std::endl;
            return 1;
        }
        // TO DO CLAIRE test nginx si client max body size obligatoire et que ce passe t il si 0 ou -1
        // if (s_list[i].size < 0) // if no client size, put -1 and we go default. Else if put at 0 what happen?
        //     s_list[i].size = 10;
        // TO DO why listen 3 connexions
        check = serv[i].server_listening(3);
        if (check != NULL){ // TO DO can't listen because full ? Maybe send proper error to bounce
            std::cout << check << std::endl;
            return 1;
        }
        event.events = EPOLLIN;
        event.data.fd = serv[i].get_fd();
        if (epoll_ctl(queue, EPOLL_CTL_ADD, serv[i].get_fd(), &event))
        {
            close(serv[i].get_fd());
            std::cout << "Failed to add file descriptor to epoll\n" << std::endl;
            return -1;
        }
        i++;
    }

    while (1){
        event_count = epoll_wait(queue, events, MAX_EVENTS, -1);
        i = 0;
        while (i < event_count){
            _handleReady(queue, events[i].data.fd, &events[i], serv, nbr_sockets);
            // else if (events[i].data.fd && EPOLLRDHUP){//socket was closed client side
            //     int j = get_fd_serv(events[i].data.fd, bfr);
            //     close(bfr[j].fd);
            // }
            // else { //writing time
            // std::cout << "bute" << std::endl;
            //     unsigned int k = 0;
            //     while (k < bfr.size() && events[i].data.fd != bfr[k].fd)
            //         k++;
            //     if (bfr[k].re.status_is_finished == true){
            //         std::cout << "bote" << std::endl;
            //         const char *response = get_response(bfr[k].re, s_list[bfr[k].num_serv]);
            //         write(events[i].data.fd, response, strlen(response));
            //     }
            // }
            // std::cout << "tour " << i << std::endl;
            i++;
        }
        i = 0;
    std::cout << "ping" << std::endl;
    }
    //     int new_sock = serv.server_accept();
    //     if (new_sock == -1){
    //         std::cout << "Error: failure to accept in server_socket" << std::endl;
    //         return 1;
    //     }

    //     // crea du systeme de gestion de requetes
        
        
    //     event.data.fd = new_sock;
        
        
    //     std::cout << "events: " << event_count << std::endl;
    //     for(int i = 0; i < event_count; i++)
    //     {
    //         std::cout << "Reading file descriptor" << events[i].data.fd << std::endl;
    //         long valread = read(events[i].data.fd, buffer, 30000);
    //         if (valread == -1)
    //             return 0;
    //         // gestion requete
    //         std::string response = first_dispatch(buffer, events[i].data.fd, cat);
    //     response.append("done");
    //         write(events[i].data.fd , response.c_str(), response.size());
    //         std::cout << "percy" << std::endl;
    //         std::vector<std::string>::iterator test = cat.begin();
    //         while (test != cat.end()){
    //             std::cout << *test << std::endl;
    //             test++;
    //         }
    //     }
        close(queue);
    //     close(new_sock);
    // }
    return 0;
}

int main(int argc, char *argv[]){
    if (argc != 2){
        std::cout << "ERROR, conf file missing" << std::endl;
    }
    serverConf conf = start_conf(argv[1]);
    // struct server s[1];
    // s->port = 8000;
    // s->name = "John";
    // s->size = 10;
    // s->error_page = "";
    // struct location l[1];
    // l->root = "\\";
    // l->index = "index.html";
    // l->autoindex = true;
    // l->cgi = "";
    // l->upload_dir = "";
    // l->redirect = "";
    // s->l = l;
    // std::vector<struct server> s_list;
    // s_list.push_back(*s);
        return launch(conf);
    return 0;
}