#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>

class Config{
    private:
        char *name;
        std::ifstream file;
        int status;
        std::string status_error;
        int port;
    public:
        Config();
        ~Config();
        void checkfile(char *name);
        void filup();
        int get_status();
        std::string get_status_error();
};

#endif // !CONFIG_HPP
