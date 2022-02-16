#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <istream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <cstring>

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <bitset>

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
        void handle_get(char *msg, int fd, std::vector<std::string> cat);
        void handle_post(char *msg, int fd, std::vector<std::string> cat);
        void handle_delete(char *msg);
        void go_cgi_get(std::string file);
        std::string get_file(char *msg);
        std::string get_content_type(std::string file);
        void get_file_post(std::string mess);
        void extract_data_post(std::string mess, std::vector<std::string> cat);
        void handle_multipart(std::string mess, std::vector<std::string> cat);
};

#endif // !REQUEST_HPP