#include <iostream>
#include <cstdlib>
struct request{
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

struct bundle_for_response{
    int fd_listen;
    int fd_accept;
    int fd_read;
    int fd_write;
    request re;
    int specs;
};

long get_content_size(std::string mess);
std::string get_body(std::string mess);
request fill_request_basic(char *msg, int n);
void classic_post(std::string mess, struct request *r);
void chunked_post(std::string mess, struct request *r);
request fill_request_post(char *msg, struct request r);
request first_dispatch(char *msg);