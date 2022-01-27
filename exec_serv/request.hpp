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
    public:
        Request();
        // ~Request();
        void set_error(int error);
        void handle_get(char *msg);
        // void handle_post(char *msg);
        // void handle_delete(char *msg);
};

#endif // !REQUEST_HPP