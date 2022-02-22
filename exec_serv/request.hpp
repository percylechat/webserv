#include <iostream>

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

long get_content_size(std::string mess);
std::string get_body(std::string mess);
request fill_request_basic(char *msg, int n);
void classic_post(std::string mess, struct request *r);
void chunked_post(std::string mess, struct request *r);
request fill_request_post(char *msg, struct request r);
request first_dispatch(char *msg);