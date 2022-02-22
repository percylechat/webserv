#include "../socket.hpp"
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h> // queue handling
// #include "request.hpp"
#include <vector>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>

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
struct request{
    std::string type;
    std::string host_ip;
    std::string host_address;
    std::string page;
    int error_type;
    bool status_is_handled;
    bool status_is_finished;
    std::string content_type;
    std::string encoding;
    long content_size;
    std::string body;
};

long get_content_size(std::string mess){
    std::size_t file_start = mess.find("Content-Length: ") + 16;
    if (file_start == 16)
        return -1;
    std::size_t file_end = file_start;
    while (mess[file_end] != '\r' && mess[file_end] != '\n' && mess[file_end] != ' ')
        file_end++;
    std::string size = mess.substr(file_start, file_end - file_start);
    return atoi(size.c_str());
}

std::string get_body(std::string mess){
    std::size_t start = mess.find("\r\n\r\n");
    if (start == mess.npos)
        return "";
    start += 4;
    return mess.substr(start, mess.size() - start);
}

request fill_request_basic(char *msg, int n){
    struct request r;
    int i;
    if (n == 1){
        r.type = "GET";
        i = 4;
    }
    else if (n == 3){
        r.type = "POST";
        i = 5;
    }
    else {
        r.type = "DELETE";
        i = 7;
    }
    while (msg[i] != ' ')
        i++;
    std::string mess = &msg[4];
    r.page = mess.substr(0, i - 4);
    i++;
    int j = i;
    while (msg[i] != ' ' && msg[i] != '\n' && msg[i] != '\r')
        i++;
    std::string http = mess.substr(j, i - j);
    if (http != "HTTP/1.1"){
        r.error_type = 505;
        return r;
    }
    std::size_t k = mess.find("Host: ");
    if (k == mess.npos){
        r.error_type = 400;
        return r;
    }
    size_t l = k + 7;
    while (mess[l] != ':')
        l++;
    r.host_address = mess.substr(k, l - k);
    l++;
    k = l;
    while (mess[l] != ' ' && mess[l] != '\n' && mess[l] != '\r')
        l++;
    std::string ip = mess.substr(k, l - k);
    r.host_ip = atoi(ip.c_str());
    r.error_type = 200;
    return r;
}

void classic_post(std::string mess, struct request *r){
    r->content_size = get_content_size(mess);
    if (r->content_size == -1){
        r->error_type = 411;
        return ;
    }
    r->body = get_body(mess);
    if (r->body == "")
        r->error_type = 400;
    r->status_is_finished = true;
}

void chunked_post(std::string mess, struct request *r){
    r->status_is_finished = false;
    (void)mess;
}

request fill_request_post(char *msg, struct request r){
    std::string mess = msg;
    std::size_t type = mess.find("Content-Type: ") + 14;
    if (type == 14){
        r.error_type = 411;
        return r;
    }
    size_t end = type;
    while (mess[end] != '\n' && mess[end] != ' ' && mess[end] != '\r')
        end++;
    std::string content_type = mess.substr(type, end - type);
    if (content_type == "multipart/form-data"){
        std::size_t start = mess.find("boundary=\"");
        if (start == mess.npos){
            r.error_type = 400;
            r.status_is_finished = true;
            return r;
        }
        r.body = mess.substr(start, mess.size() - start);
        r.status_is_finished = true;
    }
    else if (content_type == "application/x-www-form-urlencoded"){
        classic_post(mess, &r);
        if (r.error_type != 200)
            return r;
    }
    else if (content_type == "text/plain"){
        std::size_t type = mess.find("Transfer-Encoding: ") + 19;
        if (type != 19){
            size_t end = type;
            while (mess[end] != '\n' && mess[end] != ' ' && mess[end] != '\r')
                end++;
            r.encoding = mess.substr(type, end - type);
            if (r.encoding == "chunked")
                chunked_post(mess, &r);
        }
        classic_post(mess, &r);
        if (r.error_type != 200)
            return r;
    }
    else
        classic_post(mess, &r);
    return r;
}

request first_dispatch(char *msg){
    struct request r;
    if (msg[0] == 'G' && msg[1] == 'E' && msg[2] == 'T' && msg[3] == ' '){
        r = fill_request_basic(msg, 1);
        r.status_is_finished = true;
    }
    else if (msg[0] == 'D' && msg[1] == 'E' && msg[2] == 'L' && msg[3] == 'E' && msg[4] == 'T' && msg[5] == 'E' && msg[6] == ' '){
        r = fill_request_basic(msg, 2);
        r.status_is_finished = true;
    }
    else if (msg[0] == 'P' && msg[1] == 'O' && msg[2] == 'S' && msg[3] == 'T' && msg[4] == ' '){
        r = fill_request_basic(msg, 3);
        r = fill_request_post(msg, r);
    }
    else if (msg == NULL)
        r.error_type = 400;
    else
        r.error_type = 405;
    return r;
}

// int launch(std::vector<std::string> cat){
//     //create server
//     struct epoll_event event;
//     struct epoll_event events[MAX_EVENTS];
//     int event_count;
//     // crea de la socket
//     Socket serv(1);
//     // take port and address style (here ipv4)
//     char *check = serv.create_socket(8080, INADDR_ANY);
//     if (check != NULL){
//         std::cout << check << std::endl;
//         return 1;
//     }
//     check = serv.server_binding();
//     if (check != NULL){
//         std::cout << check << std::endl;
//         return 1;
//     }
// // TO DO include socket in queue
//     check = serv.server_listening(10);
//     if (check != NULL){
//         std::cout << check << std::endl;
//         return 1;
//     }
//     while (1){
//         int new_sock = serv.server_accept();
//         if (new_sock == -1){
//             std::cout << "Error: failure to accept in server_socket" << std::endl;
//             return 1;
//         }
//         char buffer[30000] = {0};
//         // crea du systeme de gestion de requetes
//         int queue = epoll_create1(0); // takes flag so none for now
//         if (queue == -1){
//             std::cout << "wtf epoll1" << std::endl;
//             return 1;
//         }
//         event.events = EPOLLIN;
//         event.data.fd = new_sock;
//         if (epoll_ctl(queue, EPOLL_CTL_ADD, new_sock, &event) == -1)
//         {
//             std::cout << "Failed to add file descriptor to epoll\n" << std::endl;
//             close(queue);
//             return 1;
//         }
//         event_count = epoll_wait(queue, events, MAX_EVENTS, 30000);
//         std::cout << "events: " << event_count << std::endl;
//         for(int i = 0; i < event_count; i++)
//         {
//             std::cout << "Reading file descriptor" << events[i].data.fd << std::endl;
//             long valread = read(events[i].data.fd, buffer, 30000);
//             if (valread == -1)
//                 return 0;
//             // gestion requete
//             std::string response = first_dispatch(buffer, events[i].data.fd, cat);
//         response.append("done");
//             write(events[i].data.fd , response.c_str(), response.size());
//             std::cout << "percy" << std::endl;
//             std::vector<std::string>::iterator test = cat.begin();
//             while (test != cat.end()){
//                 std::cout << *test << std::endl;
//                 test++;
//             }
//         }
//         close(queue);
//         close(new_sock);
//     }
//     return 0;
// }

// void check_methods(request *r, server s){
//     std::vector<std::string>iterator it;
//     it = s.method.begin();
//     while (it != )
// }

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

struct bundle_for_response{
    int fd;
    int num_serv;
    request re;
};

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
            //     if (j != -1){ // need accept socket
    // std::map< int, t_serverData >::iterator found = _serverSet.find(fd);
    Socket sock[1];
    sock[0].fd_sock = fd;


        // if (found != _serverSet.end())
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
        else if (event->events & EPOLLIN)
            // _handleEpollin(sock, _requests[fd], epoll_fd, event);
            {
                // recv(current_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                int ret = 0;
                char buffer[300];
                bzero(buffer, sizeof(buffer));
                ret = recv(sock[0].fd_sock, buffer, sizeof(buffer), MSG_DONTWAIT);
                if (ret == 0)
                    return ;
                else if (ret == -1)
                    // CLIENT REMOVED IN THIS CASE
                    return ;

    // return std::string(static_cast< char * >(buffer), ret);
    //             const std::string content = sock.readContent();
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

int launch(std::vector<struct server> s_list){
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
    unsigned int i = 0;
    int nbr_sockets = s_list.size();
    Socket serv[nbr_sockets];
    while (i < s_list.size()){
        check = serv[i].create_socket(s_list[i].port, INADDR_ANY);
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
        if (s_list[i].size < 0) // if no client size, put -1 and we go default. Else if put at 0 what happen?
            s_list[i].size = 10;
        check = serv[i].server_listening(s_list[i].size);
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
            // int current_fd = events[i].data.fd;
            // std::cout << "hello" << std::endl;
            // //chgr pr vecteur avec sockets branchées?
            // //chgr ordre
            // if (current_fd && EPOLLIN){ // read ds socket
            //  // need read request
            //         // TO DO change reading from char to byte?
            //         // TO DO handle chunked
            //         // TO DO check where check correspondance port server port request
            //                 std::cout << "hello" << std::endl;
            //     int j = check_conn(current_fd, serv, nbr_sockets);
            //     if (j != -1){ // need accept socket
            //                 std::cout << "hello" << std::endl;
            //         struct bundle_for_response one;
            //         struct epoll_event hello;
            //                     std::cout << "hello" << std::endl;
            //         one.num_serv = j;
            //         one.fd = serv[j].server_accept();
            //                     std::cout << "hello" << std::endl;
            //         if (fcntl(one.fd, F_SETFL, O_NONBLOCK) < 0){
            //             std::cout << "Error setting nonblocking socket" << std::endl;
            //                 return 1;
            //         }
            //         hello.events = EPOLLIN | EPOLLRDHUP;
            //         // hello.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
            //         hello.data.fd = one.fd;
            //                     std::cout << "hello" << std::endl;
            //         if (epoll_ctl(queue, EPOLL_CTL_ADD, one.fd, &hello) == -1)
            //         // event.events = EPOLLIN | EPOLLRDHUP;
            //         // event.data.fd = one.fd;
            //         //             std::cout << "hello" << std::endl;
            //         // if (epoll_ctl(queue, EPOLL_CTL_ADD, one.fd, &event) == -1)
            //         {
            //             std::cout << "Failed to add file descriptor to epoll\n" << std::endl;
            //             close(queue);
            //             return 1;
            //         }
            //         bfr.push_back(one);
            //         std::cout << bfr.size() << "accept fd is " << one.fd << std::endl;
            //     }
            //     else{
            //         std::cout << "read fd is " << current_fd << std::endl;
            //         recv(current_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
            //             // if (ret == 0)
            //             //     buffer = NULL;
            //             // read(events[i].data.fd, buffer, 300000);
            //             //TO DO better check for server
            //         int k = get_fd_serv(current_fd, bfr);
            //         if (k == -1)
            //             break; //error somechere
            //         bfr[k].re = first_dispatch(buffer);
            //         std::cout << bfr[k].re.type << std::endl;
            //         if (bfr[k].re.status_is_finished == true){
            //             struct epoll_event hello;
            //             hello.events = EPOLLOUT;
            //             hello.data.fd = current_fd;
            //             if (epoll_ctl(queue, EPOLL_CTL_ADD, current_fd , &hello) == -1)
            //             {
            //                 std::cout << errno << "Failed to add file descriptor to epoll\n" << std::endl;
            //                 // close(queue);
            //                 // return 1;
            //             }
            //         }
            //     }
            // }
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

// int launch(std::vector<struct server> s_list){
//     std::vector<int> sockets;
//     std::vector<struct server>::iterator it = s_list.begin();
//     while it != s_list.end(){
//         this->address.sin_family = AF_INET; //(protocol ipv4)
//         this->address.sin_port = htons(port); // TO DO CHECK BONNE CONV
//         this->address.sin_addr.s_addr = htonl(inter);
//         this->fd_sock = socket(AF_INET, SOCK_STREAM, 0);
//         it++;
//     }
//     {
//         /* data */
//     };
    
//     return 1;
// }

int main(){
    
    struct server s[1];
    s->port = 8000;
    s->name = "John";
    s->size = 10;
    s->error_page = "";
    struct location l[1];
    l->root = "\\";
    l->index = "index.html";
    l->autoindex = true;
    l->cgi = "";
    l->upload_dir = "";
    l->redirect = "";
    s->l = l;
    std::vector<struct server> s_list;
    s_list.push_back(*s);
    //(int argc, char *argv[]){
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

        // std::vector<std::string> cat;
        //             std::vector<std::string>::iterator test = cat.begin();
        //     while (test != cat.end()){
        //         std::cout << *test << std::endl;
        //         test++;
        //     }
        // cat.push_back("CHATON");
        // cat.push_back("KERO");

        return launch(s_list);
    // return 0;
}