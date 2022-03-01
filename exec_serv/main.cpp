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

#include <dirent.h>
#include <sys/types.h>
#include <algorithm>

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

std::string set_error(int err){
    std::string error_msg = "";
    std::string response = "";
    if (err == 400){
// for now for missing extension in file
        error_msg = "Bad Request";
        response.append("400 ");
        response.append(error_msg);
        response.append(" Content-Type: text/html Context-Lenght: 101\n\n");
        response.append("<html><body>400 BAD REQUEST<img src=\"error/400.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
    if (err == 403){
// for now for missing extension in file
        error_msg = "Forbidden";
        response.append("403 ");
        response.append(error_msg);
        response.append(" Content-Type: text/html Context-Lenght: 99\n\n");
        response.append("<html><body>403 FORBIDDEN<img src=\"error/403.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
    else if (err == 404){
// for now, couldn't open file so does not exist
        error_msg = "Not Found";
        response.append("404 ");
        response.append(error_msg);
        response.append(" Content-Type: text/html Context-Lenght: 91\n\n");
        response.append("<html><body>404 NOT FOUND<img src=\"error/404.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
    else if (err == 405){
// is not GET POST or DELETE
        error_msg = "Method Not Allowed";
        response.append("405 ");
        response.append(error_msg);
        response.append(" Content-Type: text/html Context-Lenght: 108\n\n");
        response.append("<html><body>405 METHOD NOT ALLOWED<img src=\"error/405.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
    else if (err == 411){
// content lenght missing
        error_msg = "Length Required";
        response.append("411 ");
        response.append(error_msg);
        response.append(" Content-Type: text/html Context-Lenght: 106\n\n");
        response.append("<html><body>411 LENGTH REQUIRED <img src=\"error/411.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
    else if (err == 413){
// content lenght missing
        error_msg = "Playload Too Large";
        response.append("413 ");
        response.append(error_msg);
        response.append(" Content-Type: text/html Context-Lenght: 108\n\n");
        response.append("<html><body>413 PLAYLOAD TOO LARGE<img src=\"error/413.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
    else if (err == 500){
// For now, couldn't delete file
        error_msg = "Internal Server Error";
        response.append("500 ");
        response.append(error_msg);
        response.append(" Content-Type: text/html Context-Lenght: 111\n\n");
        response.append("<html><body>500 INTERNAL SERVER ERROR<img src=\"error/500.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
    else if (err == 505){
// bad hhtp protocol version
        error_msg = "HTTP Version not supported";
        response.append("505 ");
        response.append(error_msg);
        response.append(" Content-Type: text/html Context-Lenght: 116\n\n");
        response.append("<html><body>505 HTTP VERSION NOT SUPPORTED<img src=\"error/505.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
    return response;
}

std::string go_error(int err, serverConf conf, Bundle_for_response bfr)
{

// EN PLUS NOW error 413 PAYLOAD TOO LARGE
// 403 FORBIDDEN


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
    if (conf.http.data()[bfr.specs][bfr.loc]["root"][0].size())
        url += conf.http.data()[bfr.specs][bfr.loc]["root"][0];
    while (i < conf.http.data()[bfr.specs]["server"]["error_page"].size())
    {
        if (conf.http.data()[bfr.specs]["server"]["error_page"][i].length() >= 3 && atoi(conf.http.data()[bfr.specs]["server"]["error_page"][i].substr(0, 3).c_str()) == err \
        && conf.http.data()[bfr.specs]["server"]["error_page"][i].find_first_not_of("\t\n\r\v\f ", 3) != std::string::npos)
            url += conf.http.data()[bfr.specs]["server"]["error_page"][i].substr(conf.http.data()[bfr.specs]["server"]["error_page"][i].find_first_not_of("\t\n\r\v\f ", 3), \
            conf.http.data()[bfr.specs]["server"]["error_page"][i].length() - conf.http.data()[bfr.specs]["server"]["error_page"][i].find_first_not_of("\t\n\r\v\f ", 3));
        i++;
    }
    std::cout << "err" << err << std::endl;
    const char *codes[8] = { "400", "403", "404", "405", "411", "413", "500", "505" };
    while (j < 8)
    {
        if (err == atoi(codes[j]))
            errComp = atoi(codes[j]);
        j++;
    }
    std::string numberString = "";
    std::string prefix = url;
    if (prefix.find_first_not_of("/", 0) != std::string::npos)
        prefix = prefix.substr(prefix.find_first_not_of("/", 0), prefix.length() - prefix.find_first_not_of("/", 0));
    std::cout << "prefix" << prefix << std::endl;
    std::basic_ifstream<char> fs(prefix.c_str());
    std::ostringstream oss;
    oss << fs.rdbuf();
    std::string content(oss.str());
    std::ostringstream digit;
    digit << content.size();
    numberString = digit.str();
    if (err == 400)// for now for missing extension in file
        response.append("400 BAD REQUEST");
    else if (err == 403 && content.size())
        response.append("403 FORBIDDEN");
    else if (err == 404 && content.size())// for now, couldn't open file so does not exist
        response.append("404 NOT FOUND");
    else if (err == 405 && content.size())// is not GET POST or DELETE
        response.append("405 METHOD NOT ALLOWED");
    else if (err == 411 && content.size())// content lenght missing
        response.append("411 LENGTH REQUIRED");
    else if (err == 413 && content.size()) // client_max_body_size : Sets the maximum allowed size of the client request body, specified in the “Content-Length” request header field. If the size in a request exceeds the configured value, the 413 (Request Entity Too Large) error is returned to the client. Setting size to 0 disables checking of client request body size.
        response.append("413 PAYLOAD TOO LARGE");
    else if (err == 500 && content.size())// For now, couldn't delete file
        response.append("500 INTERNAL SERVER ERROR");
    else if (err == 505 && content.size())// bad hhtp protocol version
        response.append("505 HTTP VERSION NOT SUPPORTED");
    else
        return set_error(err);
    std::cout << "content size" << content.size() << std::endl;
    if (content.size())
        response += " Content-Type: " + findExtension(prefix) + " Content-Length: " + numberString + "\r\n\r\n" + content;
    return response;
}

std::string go_redirect(Bundle_for_response bfr, serverConf conf){
    std::string response = "HTTP/1.1 ";
    std::string to_extract = conf.http.data()[bfr.specs][bfr.loc]["return"][0];
    std::string t = to_extract.substr(0, to_extract.find_first_of(" "));
    response.append(t);
    response.append("\r\nLocation: ");
    response.append(to_extract.substr(to_extract.find_first_of(" ") + 1, to_extract.size() - to_extract.find_first_of(" ") + 1));
    return response;
}

std::string go_directory(Bundle_for_response bfr, serverConf conf){
    std::string response = "";
    std::string body = "";
    std::string temp = bfr.absolut_path.substr(bfr.absolut_path.find_last_of("/") + 1, bfr.absolut_path.size() - bfr.absolut_path.find_last_of("/") + 1);
    std::cout << "temp test" << temp << std::endl;
    struct stat s;
    std::cout << "DIR " << std::endl;
    if ( stat(temp.c_str(), &s) == 0 ){
            std::cout << "DIR 2" << std::endl;
        if ( s.st_mode & S_IFDIR ){
            if (bfr.re.type == "DELETE")
                return go_error(403, conf, bfr);
            if (conf.http.data()[bfr.specs][bfr.loc]["autoindex"].size() > 0){
                if (conf.http.data()[bfr.specs][bfr.loc]["autoindex"][0] == "1"){
                    struct dirent *dp;
                    DIR *dirp = opendir(temp.c_str());
                    while ((dp = readdir(dirp)) != NULL){
                        std::cout << "ping" << dp->d_name << std::endl;
                        body.append(dp->d_name);
                        body.append("\n");
                    }
                    (void)closedir(dirp);
                    response.append(body);
                }
            }
            else
                return go_error(403, conf, bfr);
        }
        else{
            return response;
        }
    }
    else
    std::cout << errno << std::endl;
        return response;
    return response;
    // TO DO handle if absolut path fucked
}

std::string go_simple_upload(Bundle_for_response bfr, serverConf conf){
//TO DO si pas txt open binary
    if (bfr.re.encoding == "chunked" && bfr.re.filename == "")
        return "HTTP/1.1 202 ACCEPTED";
    std::string way = "";
    std::string te;
    std::cout << "check post filename " << bfr.re.filename << std::endl;
    if (conf.http.data()[bfr.specs][bfr.loc]["upload_dir"].size() > 0){
        way.append(conf.http.data()[bfr.specs][bfr.loc]["upload_dir"][0]);
        way = way.substr(1, way.size() - 1);
    }
    if (bfr.absolut_path.size() > 1)
        te = way + "/";
    else
        te = bfr.absolut_path + way + "/";
    // te.append(way);
    // te.append("/");
    // chdir(te.c_str());
    te.append(bfr.re.filename);
    if (bfr.re.body == "")
        return "HTTP/1.1 204 NO CONTENT";
    std::cout << "open file " << te << std::endl;
    std::ofstream fs;
    fs.open(te.c_str());
    fs << bfr.re.body;
    std::cerr << bfr.re.filename.c_str() << bfr.re.body << std::endl;
    fs.close();
    return "HTTP/1.1 201 CREATED";
}

std::string go_form_post(Bundle_for_response bfr, serverConf conf){
    std::string response = "HTTP/1.1 ";
    int j = std::count(bfr.re.query.c_str(), bfr.re.query.c_str() + bfr.re.query.size(), '&');
    if (j == 0)
        return go_error(400, conf, bfr);
    int k = -1;
    std::string content = bfr.re.query;
    while (k < j){
        std::string key = content.substr(0, content.find("="));
        std::size_t stop = content.find_first_of(" \n&\r");
        if (stop == content.npos){
            std::string value = content.substr(key.size() + 1, content.size() - (key.size() + 1));
            response.append("200 OK");
            return response;
        }
        std::string value = content.substr(key.size() + 1, stop - (key.size() + 1));
        content = content.substr(stop + 1, content.size() - (stop + 1));
        k++;
    }
    return response.append("200 OK");
}

std::string go_post_check(Bundle_for_response bfr, serverConf conf){
    std::cout << "test4" << bfr.re.filename << std::endl;
    if (bfr.re.content_type != "multipart/form-data" && bfr.re.content_type != "application/x-www-form-urlencoded")
        return go_simple_upload(bfr, conf);
    else
        return go_form_post(bfr, conf);
    return "";
}

std::string handle_delete(Bundle_for_response bfr, serverConf conf){
    std::string file = bfr.absolut_path;
    size_t i = 0;
    if (file.length() && file.find("/", 0) == 0)
    {
        while (i < file.length() && file.at(i) == '/')
            i++;
        file = file.substr(i, file.length() - i);
    }
    std::string response = "HTTP/1.1 200 OK";
    if (remove(file.c_str()) != 0)
        return go_error((bfr.re.error_type = 500), conf, bfr);
    return response;
}

std::string handle_get(Bundle_for_response bfr, serverConf conf)
{
    std::string response = "HTTP/1.1 200 OK ";
    std::string numberString = "";
    std::string url = bfr.absolut_path;
    size_t i = 0;
    if (url.length() && url.find("/", 0) == 0)
    {
        while (i < url.length() && url.at(i) == '/')
            i++;
        url = url.substr(i, url.length() - i);
    }
    std::basic_ifstream<char> fs(url.c_str());
    std::ostringstream oss;
    oss << fs.rdbuf();
    std::string content(oss.str());
    std::ostringstream digit;
    digit << content.size();
    numberString = digit.str();
    if (content.size())
        return response += "Content-Type: " + findExtension(bfr.absolut_path) + " Content-Length: " + numberString + "\r\n\r\n" + content;
    return go_error((bfr.re.error_type = 404), conf, bfr);
}

std::string get_response(Bundle_for_response bfr, serverConf conf){
    bfr.re.status_is_handled = true;
    std::string response = "HTTP/1.1 200 OK ";
    if (bfr.re.error_type != 200)
        return go_error(bfr.re.error_type, conf, bfr);
    std::cerr << "kikou" << std::endl;
    if (bfr.re.is_cgi == true)
        return handle_cgi(bfr, conf);
    if (conf.http.data()[bfr.specs][bfr.loc]["return"].size() > 0)
        return go_redirect(bfr, conf);
    std::string resp = go_directory(bfr, conf);
    if (resp != "")
        return resp;
    if (bfr.re.type == "POST")
        return go_post_check(bfr, conf);
    if (bfr.re.type == "DELETE")
        return handle_delete(bfr, conf);
    if (bfr.re.type == "GET")
        return handle_get(bfr, conf);
    return go_error((bfr.re.error_type = 405), conf, bfr);
}

int compare_path(std::string root, std::string page){
    int res = 0;
    if (root.size() > 1)
        root = root.substr(1, root.size() - 1);
    else
        return 1;
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

Bundle_for_response confirm_used_server(Bundle_for_response bfr, serverConf conf){
    if (conf.http.data()[bfr.specs].size() > 1){
        std::map<std::string,std::map<std::string,std::vector<std::string> > >::iterator it = conf.http.data()[bfr.specs].begin();
        int best = 0;
        int i = 0;
        while (it != conf.http.data()[bfr.specs].end()){
            if (it->first != "server"){
                std::string loc = it->first.substr(9, it->first.size() - 9);
                int test = compare_path(loc, bfr.re.page);
                if (test > best)
                    bfr.loc = it->first;
            }
            i++;
            it++;
        }
    }
    else
        bfr.loc = 1;
    std::size_t j = bfr.specs + 1;
    int best = compare_path(bfr.loc.substr(9, bfr.loc.size() - 9), bfr.re.page);
    while (j < conf.http.size()){
        std::map<std::string,std::map<std::string,std::vector<std::string> > >::iterator it = conf.http.data()[j].begin();
        int i = 0;
        while (it != conf.http.data()[j].end()){
            if (it->first != "server"){
                std::string loc = it->first.substr(9, it->first.size() - 9);
                int test = compare_path(loc, bfr.re.page);
                if (test > best){
                    bfr.loc = it->first;
                    bfr.specs = j;
                }
            }
            i++;
            it++;
        }
        j++;
    }
    bfr.absolut_path = conf.http.data()[bfr.specs][bfr.loc]["root"][0];
    int i = 0;
    int g = 0;
    std::cout << bfr.loc << std::endl;
    std::string temp = bfr.loc.substr(9, bfr.loc.size() - 9);
    std::cout << "temp check" << temp << bfr.re.page << std::endl;
    while (bfr.re.page[i] == temp[g]){
        i++;
        g++;
    }
    if (i >= 1)
        i--;
    bfr.absolut_path.append(bfr.re.page.substr(i, bfr.re.page.size() - i));
    std::cout << "absolut path" << bfr.absolut_path << std::endl;
    std::size_t d = bfr.re.page.find_last_of(".");
    if (d != bfr.re.page.npos){
        std::string ext = bfr.re.page.substr(d, bfr.re.page.size() - d);
        if (conf.http.data()[bfr.specs][bfr.loc]["cgi"].size() > 0){
            if (ext == conf.http.data()[bfr.specs][bfr.loc]["cgi"][0])
                bfr.re.is_cgi = true;
            else
                bfr.re.is_cgi = false;
        }
        else
            bfr.re.is_cgi = false;
    }
    if (conf.http.data()[bfr.specs]["server"]["client_max_body_size"].size() > 0 && atoi(conf.http.data()[bfr.specs]["server"]["client_max_body_size"][0].c_str()) > 0) {
        int g = atoi(conf.http.data()[bfr.specs]["server"]["client_max_body_size"][0].c_str());
        {
            std::cout << "bfr*****************" << static_cast< int >(bfr.re.body.size()) << std::endl;
            std::cout << "g*******************" << g << std::endl;
            if (static_cast< int >(bfr.re.body.size()) > g)
                bfr.re.error_type = 413;
        }
    }
    if (conf.http.data()[bfr.specs][bfr.loc]["methods"].size() > 0){
        std::size_t h = 0;
        int cont = 0;
        while (conf.http.data()[bfr.specs][bfr.loc]["methods"].size() > h){
            if (conf.http.data()[bfr.specs][bfr.loc]["methods"][h] == bfr.re.type)
                cont++;
            h++;
        }
        if (cont == 0)
            bfr.re.error_type = 405;
    }
    std::cout << "check is done" << std::endl;
    return bfr;
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
    if (epoll_ctl(queue, EPOLL_CTL_ADD, fd, &event)){
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
        if (bfr[i].re.status_is_handled == false){
                    std::cout << "test2bis" << bfr[i].re.filename << std::endl;
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
        }
        std::cout << "success" << std::endl;
    }
    else if (event->events & EPOLLIN){
        unsigned int i = 0;
        while (i < bfr.size() && (bfr[i].fd_accept != fd)){
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
        bfr[i] = confirm_used_server(bfr[i], conf);
        if (bfr[i].re.status_is_finished == true)
        {
            event->events = EPOLLOUT;
            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock[0].fd_sock, event);
            bfr[i].fd_read = sock[0].fd_sock;
        }
    }
    else
        ;
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
                check = serv[i].server_listening(5); // only 5 waiting conneions authorized so the server is not overcharged
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
        event_count = epoll_wait(queue, events, MAX_EVENTS, -1);
        i = 0;
        while (i < event_count){
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