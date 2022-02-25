#include "../socket.hpp"
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h> // queue handling
// #include "request.hpp"
#include <vector>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include <fcntl.h>
// #include "../parsing_conf.hpp"
// #include "request.hpp"
#include "cgi.hpp"

#define MAX_EVENTS 5

std::vector<Bundle_for_response> bfr;

// clang++ -Wall -Wextra -Werror -fsanitize=address -std=c++98 main.cpp ../parsing_conf.cpp cgi.cpp request.cpp ../socket.cpp 

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

std::string findExtension(std::string filepath)
{
    size_t i;

    if ((i = filepath.rfind('.')) == std::string::npos)
        return "none";
    std::string ext = filepath.substr(i+1);
    if (ext == "html")
        return "text/" + ext;
    else if (ext == "cgi")
        return "test/html";
    else
        return "image/" + ext;
}

std::string go_error(int err, serverConf conf, Bundle_for_response bfr){
    std::string response = "HTTP/1.1 ";
    if (conf.http.data()[bfr.specs]["server"]["error_page"].size() == 0)
    {
        //DO smthing
        // si error page presente
        // 404 NOT FOUND -> code d'erreur et explication
        // Content-Type: text/html Context-Lenght: 109\r\n\r\n -> type de page et taille fichier
        // ->fichier
    }
    else{
        int errComp = atoi(conf.http.data()[bfr.specs]["server"]["error_page"][0].c_str());
        std::string url = conf.http.data()[bfr.specs]["server"]["error_page"][0];

        if (errComp == err && url.find_first_not_of("\t\n\r\v\f ", 3) != std::string::npos)
        {
            url = url.substr(url.find_first_not_of("\t\n\r\v\f ", 3), url.length() - url.find_first_not_of("\t\n\r\v\f ", 3));
            std::ifstream is (url.c_str(), std::ifstream::binary);
            if (is) {
            //if (is)
            //std::cout << "all characters read successfully.";
            //else
            //std::cout << "error: only " << is.gcount() << " could be read";
            is.seekg(3, is.end);
            int length = is.tellg();
            is.seekg(3, is.beg);
            //In this example, seekg is used to move the position to the end of the file, and then back to the beginning.
            char *buffer = new char[length];
            if (length != -1)
                is.read(buffer,length);
            is.close();
            std::string content(buffer, length);
            delete [] buffer;

            std::ostringstream digit;
            digit << length;
            std::string numberString(digit.str());
            if (err == 400)// for now for missing extension in file
                response.append("400 BAD REQUEST Content-Type: " + findExtension(url) + "Context-Lenght: " + numberString + "\r\n\r\n");
            else if (err == 404)// for now, couldn't open file so does not exist
                response.append("404 NOT FOUND Content-Type: " + findExtension(url) + "Context-Lenght: " + numberString + "\r\n\r\n");
            else if (err == 405)// is not GET POST or DELETE
                response.append("405 METHOD NOT ALLOWED Content-Type: " + findExtension(url) + "Context-Lenght: " + numberString + "\r\n\r\n");
            else if (err == 411)// content lenght missing
                response.append("411 LENGHT REQUIRED Content-Type: " + findExtension(url) + "Context-Lenght: " + numberString + "\r\n\r\n");
            else if (err == 500)// For now, couldn't delete file
                response.append("500 INTERNAL SERVER ERROR Content-Type: " + findExtension(url) + "Context-Lenght: " + numberString + "\r\n\r\n");
            else if (err == 505)// bad hhtp protocol version
                response.append("505 HTTP VERSION NOT SUPPORTED Content-Type: " + findExtension(url) + "Context-Lenght: " + numberString + "\r\n\r\n");
        }
    }
    else
    {
        if (errComp == err)
            std::cout << "no url" << std::endl;
        else
            std::cout << "err != errComp" << std::endl;
        return conf.http.data()[bfr.specs]["server"]["error_page"][0].substr(0, 3);
    }
    }
    return response;
}

std::string get_response(Bundle_for_response bfr, serverConf conf){
    bfr.re.status_is_handled = true;
    if (bfr.re.error_type != 200)
        return go_error(bfr.re.error_type, conf, bfr);
// TO DO need check which location
    // check_methods(&r, s);
    if (bfr.re.is_cgi == true)
        return handle_cgi(bfr, conf);
    else{
        std::string response = "HTTP/1.1 200 OK \r\n hello";
        return response.c_str();
    }
}

int check_conn(int fd_1, Socket *serv, int nbr){
    int i = 0;
    while (i < nbr){
        if (fd_1 == serv[i].get_fd())
            return i;
        i++;
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

void poll_handling(int epoll_fd, const int fd, struct epoll_event *event, Socket *serv, serverConf conf)
{
    int j = check_conn(fd, serv, conf.http.size());
    Socket sock[1];
    sock[0].fd_sock = fd;
    if (j != -1)
    {
        unsigned int i = 0;
        while (i != bfr.size() && bfr[i].fd_listen != fd)
            i++;
        int clientsocket = sock[0].server_accept();
        Socket test[1];
        test[0].fd_sock = clientsocket;
        if (fcntl(test[0].fd_sock, F_SETFL, O_NONBLOCK) < 0)
            return ;
        if (fd_in_queue(test[0].fd_sock, epoll_fd, 2))
            return ;
        bfr[i].fd_accept = test[0].fd_sock;
    }
    else if (event->events & EPOLLRDHUP){
        close(sock[0].fd_sock);
// TO DO remove attached request
    }
    else if (event->events & EPOLLOUT){
        std::cout << "hello write" << std::endl;
        unsigned int i = 0;
        while (i != bfr.size() && bfr[i].fd_read != fd)
            i++;
        int ret;
        std::string content = get_response(bfr[i], conf);
        std::cout << content << std::endl;
        ret = send(sock[0].fd_sock, content.c_str(), content.size(), 0);
        if (ret == 0 || ret == -1)
            return ;
        
        close(sock[0].fd_sock);

// TO DO check quand maintenir co
        // event->events = EPOLLIN;
        // epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock[0].fd_sock, event);
        // bfr[i].fd_accept = sock[0].fd_sock; // write?
        // bfr[i].init_re();
        std::cout << "success" << std::endl;
    }
    else if (event->events & EPOLLIN){
        unsigned int i = 0;
        while (bfr[i].fd_accept != fd){
            std::cout << fd << bfr[i].fd_accept << std::endl;
            i++;
        }
        int ret = 0;
        char buffer[3000];
// TO DO find way either to adapt buffer size or to limit if not big enough or else buffer overflow
        bzero(buffer, sizeof(buffer));
        ret = recv(sock[0].fd_sock, buffer, sizeof(buffer), MSG_DONTWAIT);
        std::cout << buffer << std::endl;
        if (ret == 0)
            return ;
        else if (ret == -1)
// TO DO client exit or empty request
            return ;
        // Request o;
        // Bundle_for_response one;
        // one.re = o;
        // one = bfr[i];
        first_dispatch(buffer, &(bfr[i].re));
        std::cout << "request" << bfr[i].re.error_type << bfr[i].re.host_address << bfr[i].re.host_ip << std::endl;
        // bfr[i].re = re;
        if (bfr[i].re.status_is_finished == true)
        {
            event->events = EPOLLOUT;
            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock[0].fd_sock, event);
            bfr[i].fd_read = sock[0].fd_sock;
        }
    }
    else
        std::cout << "bug somewhere" << std::endl;
    std::cout << "ping" << std::endl;
    return ;
}

int launch(serverConf conf){
// TO DO think max event variable
    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS];
    unsigned int event_count;
    const char *check = NULL;
    // std::vector<Bundle_for_response> bfr;

    std::string home = "127.0.0.1";
// TO DO check error handling while creating queue
    int queue = epoll_create1(EPOLL_CLOEXEC);
    if (queue == -1){
        std::cout << "wtf epoll1" << std::endl;
        return 1;
    }
    std::size_t i = 0;
    Socket serv[conf.http.size()];
    while (i < conf.http.size()){
        int port = atoi(conf.http.data()[i]["server"]["listen"][0].c_str());
// TO DO pas creer socket si port deja pris, mais garder conf car peut etre difference avec server names
        check = serv[i].create_socket(port, INADDR_ANY);
        if (check != NULL){
            std::cout << check << std::endl;
            return 1;
        }
        check = serv[i].server_binding();
        if (check != NULL){
            std::cout << check << errno << std::endl;
            return 1;
        }
        if (fcntl(serv[i].get_fd(), F_SETFL, O_NONBLOCK) < 0){
            std::cout << "Error setting nonblocking socket" << std::endl;
            return 1;
        }
// TO DO CLAIRE test nginx si client max body size obligatoire et que ce passe t il si 0 ou -1 // 0 le client_max_body_size n'est pas checké | -1 erreur cmbs invalid
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
        if (epoll_ctl(queue, EPOLL_CTL_ADD, serv[i].get_fd(), &event)){
            close(serv[i].get_fd());
            std::cout << "Failed to add file descriptor to epoll\n" << std::endl;
            return -1;
        }
        Bundle_for_response one;
        // Request re();
        one.init_re();
        one.fd_listen = serv[i].get_fd();
        one.specs = i;
        bfr.push_back(one);
        i++;
    }
    while (1){
        std::cout << "hello" << std::endl;
        event_count = epoll_wait(queue, events, MAX_EVENTS, -1);
        i = 0;
        while (i < event_count){
            std::cout << "hello" << std::endl;
            poll_handling(queue, events[i].data.fd, &events[i], serv, conf);
            i++;
        }
        // i = 0;
    }
    close(queue);
    return 0;
}

int main(int argc, char *argv[]){
    if (argc != 2){
        std::cout << "ERROR, conf file missing" << std::endl;
    }
    serverConf conf = start_conf(argv[1]);
    // std::cout << "hello" << std::endl;
    return launch(conf);
}