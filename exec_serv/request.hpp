#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <istream>

    #include <sys/stat.h>

class Request{
    public:
        bool success;
        std::string response;
        int error_code;
        std::string error_msg;
        bool cgi;
        int fd;
    public:
        Request();
        // ~Request();
        void set_error(int error);
        void handle_get(char *msg, int fd);
        void handle_post(char *msg, int fd);
        void handle_delete(char *msg);
        void go_cgi_get(std::string file);
        std::string get_file(char *msg);
        std::string get_content_type(std::string file);
};

#endif // !REQUEST_HPP