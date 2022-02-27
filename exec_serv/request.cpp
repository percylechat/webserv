#include "request.hpp"

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

bool get_content_type(std::string file){
    int i = file.find_last_of(".");
    if (i == -1)
        return false;
    std::string ext = file.substr(i);
    if (ext == ".sh")
        return true;
    return false;
}

std::string get_query(std::string page){
    std::size_t test = page.find_first_of("?");
    if (test == page.npos)
        return "";
    return page.substr(test, page.size() - test);
}

void fill_request_basic(char *msg, int n, Request *r){
    int i;
    int j;
    if (n == 1){
        r->type = "GET";
        i = 4;
    }
    else if (n == 3){
        r->type = "POST";
        i = 5;
    }
    else {
        r->type = "DELETE";
        i = 7;
    }
    j = i;
    while (msg[j] != ' ')
        j++;
    std::string mess = msg;
    r->page = mess.substr(i, j - i);
    std::cout << "page " << r->page << std::endl;
    r->is_cgi = get_content_type(r->page);
    r->query = get_query(r->page);
    j += 1;
    i = j;
    while (msg[i] != ' ' && msg[i] != '\n' && msg[i] != '\r')
        i++;
    std::string http = mess.substr(j, i - j);
    std::cout << "http " << http << std::endl;
    if (http != "HTTP/1.1"){
        r->error_type = 505;
        return ;
    }
    std::size_t k = mess.find("Host: ");
    if (k == mess.npos){
        r->error_type = 400;
        return ;
    }
    size_t l = k + 7;
    while (mess[l] != ':')
        l++;
    r->host_address = mess.substr(k, l - k);
    l++;
    k = l;
    while (mess[l] != ' ' && mess[l] != '\n' && mess[l] != '\r')
        l++;
    std::string ip = mess.substr(k, l - k);
    r->host_ip = atoi(ip.c_str());
    r->error_type = 200;
    return ;
}

void classic_post(std::string mess, Request *r){
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

void chunked_post(std::string mess, Request *r){
    r->status_is_finished = false;
    std::size_t deb = mess.find("\r\n\r\n") + 4;
    r->body = mess.substr(deb, mess.size() - deb);
    size_t j = 0;
    while (j < r->body.size()){

    deb = r->body.find_first_of('\r');
    }
}

void fill_request_post(char *msg, Request *r){
    std::string mess = msg;
    std::size_t type = mess.find("Content-Type: ") + 14;
    if (type == 14){
        r->error_type = 411;
        return ;
    }
    size_t end = type;
    while (mess[end] != '\n' && mess[end] != ' ' && mess[end] != '\r')
        end++;
    std::string content_type = mess.substr(type, end - type);
    if (content_type == "multipart/form-data"){
        std::size_t start = mess.find("boundary=\"");
        if (start == mess.npos){
            r->error_type = 400;
            r->status_is_finished = true;
            return ;
        }
        r->body = mess.substr(start, mess.size() - start);
        r->status_is_finished = true;
    }
    else if (content_type == "application/x-www-form-urlencoded"){
        classic_post(mess, r);
        if (r->error_type != 200)
            return ;
    }
    else if (content_type == "text/plain"){
        std::size_t type = mess.find("Transfer-Encoding: ") + 19;
        if (type != 19){
            size_t end = type;
            while (mess[end] != '\n' && mess[end] != ' ' && mess[end] != '\r')
                end++;
            r->encoding = mess.substr(type, end - type);
            if (r->encoding == "chunked")
                chunked_post(mess, r);
        }
        classic_post(mess, r);
        if (r->error_type != 200)
            return ;
    }
    else
        classic_post(mess, r);
    return ;
}

void first_dispatch(char *msg, Request *r){
    if (msg[0] == 'G' && msg[1] == 'E' && msg[2] == 'T' && msg[3] == ' '){
        fill_request_basic(msg, 1, r);
        r->status_is_finished = true;
    }
    else if (msg[0] == 'D' && msg[1] == 'E' && msg[2] == 'L' && msg[3] == 'E' && msg[4] == 'T' && msg[5] == 'E' && msg[6] == ' '){
        fill_request_basic(msg, 2, r);
        r->status_is_finished = true;
    }
    else if (msg[0] == 'P' && msg[1] == 'O' && msg[2] == 'S' && msg[3] == 'T' && msg[4] == ' '){
        fill_request_basic(msg, 3, r);
        fill_request_post(msg, r);
    }
    else if (msg == NULL)
        r->error_type = 400;
    else
        r->error_type = 405;
    // return r;
}