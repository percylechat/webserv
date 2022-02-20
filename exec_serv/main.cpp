#include "../socket.hpp"
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h> // queue handling
#include "request.hpp"
#include <vector>
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
    // std::vector<std::string> methods;
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

long get_content_size(std::string mess){
    std::size_t file_start = mess.find("Content-Length: ") + 16;
    if (file_start == 16)
        return -1;
    std::size_t file_end = file_start;
    while (mess[file_end] != '\r' && mess[file_end] != '\n' && mess[file_end] != ' ')
        file_end++;
    std::string size = mess.substr(file_start, file_end - file_start);
    r.content_size = atoi(size.c_str());
}

std::string get_body(std::string mess){
    std::size_t start = mess.find("\r\n\r\n");
    if (start == mess.npos)
        return "";
    start += 4;
    return mess.substr(start, mess.size() - start);
}

void chunked_post(std::string mess, struct request *r){
    r->status_is_finished = false;
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


int check_conn(int fd_1, Socket *serv, int nbr){
    int i = 0;
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

int launch(std::vector<struct server> s_list){
    //create server
    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS];
    int event_count;
    char *check = NULL;
    char buffer[300000] = {0}; // TO DO check buffer size for reading
    std::vector<struct bundle_for_response> bfr;

    // TO DO infinite timeout (neg value)
    // TO DO check error handling while creating queue
    int queue = epoll_create1(0); // takes flag so none for now
    event.events = EPOLLIN | EPOLLOUT;
    if (queue == -1){
        std::cout << "wtf epoll1" << std::endl;
        return 1;
    }
    int i = 0;
    int nbr_sockets = s_list.size() - 1;
    Socket serv[nbr_sockets];
    while (i < s_list.size()){
        check = serv[i].create_socket(s_list[i].port, "127.0.0.1");
        if (check != NULL){
            std::cout << check << std::endl;
            return 1;
        }
        check = serv[i].server_binding();
        if (check != NULL){
            std::cout << check << std::endl;
            return 1;
        }
        if (s_list[i].size < 0) // if no client size, put -1 and we go default. Else if put at 0 what happen?
            s_list[i].size = 10;
        check = serv[i].server_listening(s_list[i].size);
        if (check != NULL){ // TO DO can't listen because full ? Maybe send proper error to bounce
            std::cout << check << std::endl;
            return 1;
        }
        event.data.fd = serv[i].get_fd();
        if (epoll_ctl(queue, EPOLL_CTL_ADD, serv[i].get_fd(), &event) == -1)
        {
            std::cout << "Failed to add file descriptor to epoll\n" << std::endl;
            close(queue);
            return 1;
        }
        i++;
    }
    while (1){
        event_count = epoll_wait(queue, events, MAX_EVENTS, 30000);
        i = 0;
        while (i < event_count){
            if (events[i].data.fd && EPOLLIN){
                int j = check_conn(events[i].data.fd, serv, nbr_sockets);
                if (j != -1){ // need accept socket
                    struct bundle_for_response one;
                    one.num_serv = j;
                    one.fd = serv[j].server_accept();
                    // event.data.fd = serv[i].get_fd();
                    if (epoll_ctl(queue, EPOLL_CTL_ADD, one.fd, &event) == -1)
                    {
                        std::cout << "Failed to add file descriptor to epoll\n" << std::endl;
                        close(queue);
                        return 1;
                    }
                    bfr.push_back(one);
                }
                else{ // need read request
                    // TO DO change reading from char to byte?
                    // TO DO handle chunked
                    // TO DO check where check correspondance port server port request
                    long valread = read(events[i].data.fd, buffer, 300000);
                    int k = 0;
                    while (k < bfr.size() && events[i].data.fd != bfr[k].fd)
                        k++;
                    bfr[k].re = first_dispatch(buffer);
                    free(buffer);
                }
            }
            else { //writing time
                int k = 0;
                while (k < bfr.size() && events[i].data.fd != bfr[k].fd)
                    k++;
                
            }
            i++;
        }
        int new_sock = serv.server_accept();
        if (new_sock == -1){
            std::cout << "Error: failure to accept in server_socket" << std::endl;
            return 1;
        }

        // crea du systeme de gestion de requetes
        
        
        event.data.fd = new_sock;
        
        
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

int main(){
    
    struct server *s;
    s->port = 8080;
    s->name = "John";
    s->size = 10;
    s->error_page = "";
    struct location *l;
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