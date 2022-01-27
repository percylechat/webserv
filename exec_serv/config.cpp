#include "config.hpp"

Config::Config(){
    this->status = 0;
}
Config::~Config(){

}
void Config::checkfile(char *name){
    this->name = name;
    if (this->file.open(name[0]) == -1){
        this->status = 1;
        this->status_error = "Error: Unable to open file";
    }
}
void Config::filup(){

}
int Config::get_status(){
    return this->status;
}
std::string Config::get_status_error(){
    return this->status_error;
}