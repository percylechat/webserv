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

std::string go_error(int err, serverConf conf, Bundle_for_response bfr)
{
    std::string response = "HTTP/1.1 ";
        //DO smthing
        // si error page presente
        // 404 NOT FOUND -> code d'erreur et explication
        // Content-Type: text/html Context-Lenght: 109\r\n\r\n -> type de page et taille fichier
        // ->fichier
    int errComp = 0;
    size_t i = 0;
    size_t j = 0;
    std::string url = "";
    std::string prefix("error/");
    const char *codes[6] = { "400", "404", "405", "411", "500", "505" };
    if (conf.http.data()[bfr.specs]["server"]["error_page"].size())
    {
        while (i < conf.http.data()[bfr.specs]["server"]["error_page"].size())
        {
            while (j < 6)
            {
                if (conf.http.data()[bfr.specs]["server"]["error_page"][i].length() > 3 && conf.http.data()[bfr.specs]["server"]["error_page"][i].substr(0, 3) == codes[j])
                {
                    errComp = atoi(codes[j]);
                    url = conf.http.data()[bfr.specs]["server"]["error_page"][i];
                }
                j++;
            }
            j = 0;
            i++;
        }   
    }
    std::cout << "code 1: " << err << std::endl;
    std::cout << "code 2: " << errComp << std::endl;
    int length = 0;
    char *buffer = NULL;
    std::string content = "";
    std::string numberString = "";
    if (url.length() > 3)
    {
        prefix += url.substr(0, 3);
        prefix += ".jpeg";
        std::ifstream is (prefix.c_str(), std::ifstream::binary);
        if (is)
        {
            is.seekg(3, is.end);
            length = is.tellg();
            is.seekg(3, is.beg);
            //In this example, seekg is used to move the position to the end of the file, and then back to the beginning.
            if (length != -1)
            {
                buffer = new char[length];
                is.read(buffer,length);
                content = std::string(buffer, length);
                delete [] buffer;
                std::ostringstream digit;
                digit << length;
                numberString = digit.str();
            }
            is.close();
        }
    }
    if (err == 400)// for now for missing extension in file
        response.append("400 BAD REQUEST ");
    else if (err == 404)// for now, couldn't open file so does not exist
        response.append("404 NOT FOUND ");
    else if (err == 405)// is not GET POST or DELETE
        response.append("405 METHOD NOT ALLOWED ");
    else if (err == 411)// content lenght missing
        response.append("411 LENGHT REQUIRED ");
    else if (err == 500)// For now, couldn't delete file
        response.append("500 INTERNAL SERVER ERROR ");
    else if (err == 505)// bad hhtp protocol version
        response.append("505 HTTP VERSION NOT SUPPORTED ");
    if (errComp == err)
        response += "Content-Type: " + findExtension(prefix) + " Content-Length: " + numberString + "\r\n\r\n" + content;
    return response;
}

std::string get_response(Bundle_for_response bfr, serverConf conf){
    bfr.re.status_is_handled = true;
    if (bfr.re.error_type != 200)
        return go_error(bfr.re.error_type, conf, bfr);
    // check_methods(&r, s);
    std::cerr << "kikou" << std::endl;
    if (bfr.re.is_cgi == true)
        return handle_cgi(bfr, conf);
    else{
        std::string response = "HTTP/1.1 200 OK \r\n hello";
        return response.c_str();
    }
}

int compare_path(std::string root, std::string page){
    int res = 1;
    root = root.substr(1, root.size() - 1);
    page = page.substr(1, page.size() - 1);
    std::size_t test1 = root.find_first_of("/");
    std::size_t test2 = page.find_first_of("/");
    while (test1 != root.npos && test2 != page.npos){
        std::string check1 = root.substr(0, test1);
        std::string check2 = page.substr(0, test2);
        if (check1 != check2)
            return res;
        res++;
        root = root.substr(test1 + 1, root.size() - test1 + 1);
        page = page.substr(test2 + 1, page.size() - test2 + 1);
        test1 = root.find_first_of("/");
        test2 = page.find_first_of("/");
    }
    return res;
}

//TO DO repmace root by location
void confirm_used_server(Bundle_for_response bfr, serverConf conf){
    std::cout << "check" << std::endl;
    if (conf.http.data()[bfr.specs]["server"]["root"].size() != 1){
        std::size_t l = 1;
        int best = compare_path(conf.http.data()[bfr.specs]["server"]["root"][0], bfr.re.page);
        while (l < conf.http.data()[bfr.specs]["server"]["root"].size()){
            int curr = compare_path(conf.http.data()[bfr.specs]["server"]["root"][l], bfr.re.page);
            if (curr > best){
                best = curr;
                bfr.root = l;
            }
            l++;
        }
    }
    else
        bfr.root = 0;
    std::size_t j = bfr.specs + 1;
    while (j < conf.http.size()){
        if (conf.http.data()[j]["server"]["listen"][0] == bfr.re.host_ip){
            std::size_t k = 0;
            int best = bfr.root;
            while (k < conf.http.data()[j]["server"]["root"].size()){
                int curr = compare_path(conf.http.data()[j]["server"]["root"][k], bfr.re.page);
                if (curr > best){
                    best = curr;
                    bfr.root = k;
                    bfr.specs = j;
                }
                k++;
            }
        }
        j++;
    }
    std::cout << "check2" << std::endl;
    std::string absolut_path = conf.http.data()[bfr.specs]["server"]["root"][bfr.root];
// TO DO here?
//     Define a directory or a file from where the file should be searched (for example,
// if url /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is
// /tmp/www/pouic/toto/pouet)
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

int fd_in_queue(int fd, int queue){
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLRDHUP;
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
        if (fd_in_queue(test[0].fd_sock, epoll_fd))
            return ;
        bfr[i].fd_accept = test[0].fd_sock;
    }
    else if (event->events & EPOLLRDHUP){
        unsigned int i = 0;
        while (i != bfr.size() && bfr[i].fd_read != fd)
            i++;
//TO DO clear request or delete
        // bfr[i].init_re();
        close(sock[0].fd_sock);
    }
    else if (event->events & EPOLLOUT){
        std::cout << "hello write" << std::endl;
        unsigned int i = 0;
        while (i != bfr.size() && bfr[i].fd_read != fd)
            i++;
        int ret;
        std::string content = get_response(bfr[i], conf);
        std::cout << "response= " << content << std::endl;
        ret = send(sock[0].fd_sock, content.c_str(), content.size(), 0);
        if (ret == 0){
            std::cout << "MT response" << std::endl;
            close(sock[0].fd_sock);
            exit(0);
            // close(bfr[i].fd_read);
        }
        if (ret == 0 || ret == -1)
            return ;
        close(sock[0].fd_sock);
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
        if (ret == -1)
// TO DO handle failed recv
            return ;
        first_dispatch(buffer, &(bfr[i].re));
        confirm_used_server(bfr[i], conf);
        std::cout << "request" << bfr[i].re.error_type << bfr[i].re.host_address << bfr[i].re.host_ip << std::endl;
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
    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS]; // number of fd attended at same time, here 5
    unsigned int event_count;
    const char *check = NULL;

    std::string home = "127.0.0.1";
    int queue = epoll_create1(EPOLL_CLOEXEC);
    if (queue == -1){
        std::cout << "Error: queue handler epoll couldn't be created" << std::endl;
        return 1;
    }
    std::size_t i = 0;
    Socket serv[conf.http.size()];
    while (i < conf.http.size()){
        std::size_t j = 0;
        while (j < conf.http.data()[i]["server"]["listen"].size()){
            int port = atoi(conf.http.data()[i]["server"]["listen"][j].c_str());
            check = serv[i].create_socket(port, INADDR_ANY);
            if (check == NULL){
                check = serv[i].server_binding();
                if (check != NULL){
                    std::cout << check << std::endl;
                    return 1;
                }
                if (fcntl(serv[i].get_fd(), F_SETFL, O_NONBLOCK) < 0){
                    std::cout << "Error setting nonblocking socket" << std::endl;
                    return 1;
                }
                check = serv[i].server_listening(3); // only 2 waiting conneions authorized so the server is not overcharged
                if (check != NULL){
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
                one.init_re();
                one.fd_listen = serv[i].get_fd();
                one.specs = i;
                bfr.push_back(one);
            }
            j++;
        }
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
    }
    close(queue);
    return 0;
}

void signalHandler( int signum ) {
   std::cout << "Closing Webserv\nGoodbye!" << std::endl;
   exit(signum);  
}

int main(int argc, char *argv[]){
    if (argc != 2){
        std::cout << "ERROR, conf file missing" << std::endl;
    }
    signal(SIGINT, signalHandler);
    serverConf conf = start_conf(argv[1]);
    if (!conf._valid){
        std::cout << "This configuration file is invalid" << std::endl;
        return 1;
    }
    return launch(conf);
}