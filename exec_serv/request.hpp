#include <iostream>
#include <cstdlib>
class Request{
    public:
    Request(){
        this->type = "";
        this->host_ip = "";
        this->host_address = "";
        this->page = "";
        this->error_type = 200;
        this->status_is_finished = false;
        this->status_is_handled = false;
        this->content_type = "";
        this->encoding = "";
        this->content_size = 0;
        this->body = "";
        this->is_cgi = false;
    }
    Request& operator=( const Request & one){
        this->type = one.type;
        this->host_ip = one.host_ip;
        this->host_address = one.host_address;
        this->page = one.page;
        this->error_type = one.error_type;
        this->status_is_finished = one.status_is_finished;
        this->status_is_handled = one.status_is_handled;
        this->content_type = one.content_type;
        this->encoding = one.encoding;
        this->content_size = one.content_size;
        this->body = one.body;
        this->is_cgi = one.is_cgi;
        return (*this);
    }
    Request( const Request &one ){
    *this = one;
    }
    std::string type;// get delete post
    std::string host_ip; // port 8000 5000
    std::string host_address; // 17.0.0.1 example.com
    std::string page; // /cat/bebe.jpg
    int error_type; // num de l'erreur
    bool status_is_handled; // indique si requete en cours de reponse
    bool status_is_finished; // indique si requete finie
    std::string content_type; // indique type de contenu reçu (POST)
    std::string encoding; // indique type de encodage (POST)
    long content_size; // taille du message
    std::string body; // message
    bool is_cgi;
};

class Bundle_for_response{
    public:
    int fd_listen;
    int fd_accept;
    int fd_read;
    int fd_write;
    Request re;
    int specs;
    Bundle_for_response() {}
    Bundle_for_response& operator=( const Bundle_for_response & one){
        if (this == &one)
		    return (*this);
        this->fd_accept = one.fd_accept;
        this->fd_read = one.fd_read;
        this->fd_write = one.fd_write;
        this->fd_listen = one.fd_listen;
        this->specs = one.specs;
        this->re = one.re;
        return (*this);
    }
    void init_re(){
        this->re.type = "";
        this->re.host_ip = "";
        this->re.host_address = "";
        this->re.page = "";
        this->re.error_type = 200;
        this->re.status_is_finished = false;
        this->re.status_is_handled = false;
        this->re.content_type = "";
        this->re.encoding = "";
        this->re.content_size = 0;
        this->re.body = "";
        this->re.is_cgi = false;
    }
};

long get_content_size(std::string mess);
std::string get_body(std::string mess);
void fill_request_basic(char *msg, int n);
void classic_post(std::string mess, struct request *r);
void chunked_post(std::string mess, struct request *r);
void fill_request_post(char *msg, struct request r);
void first_dispatch(char *msg, Request *re);