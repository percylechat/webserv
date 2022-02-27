#ifndef PARSING_CONF_H
#define PARSING_CONF_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cctype>
#include <cstring>
#include <map>
#include <utility>
#include <cstdlib>

#define TRUE 1
#define FALSE 0

class serverConf
{
    public:
        bool _valid;
        std::vector< std::string > _directives;
        std::vector< std::string > _serverIds;
        std::vector< std::string > _locationIds;
        std::vector< std::map< std::string, std::map< std::string, std::vector< std::string > > > > http;
        serverConf() : _valid(0), _directives(std::vector< std::string >()), _serverIds(std::vector< std::string >()), _locationIds(std::vector< std::string >()), http(std::vector< std::map< std::string, std::map< std::string, std::vector< std::string > > > >())
        {
            _directives.push_back("events");
            _directives.push_back("http");
            //_directives.push_back("mail");
            //_directives.push_back("stream");
            _serverIds.push_back("listen");
            _serverIds.push_back("server_name");
            _serverIds.push_back("location");
            _serverIds.push_back("error_page");
            _serverIds.push_back("client_max_body_size");
            _serverIds.push_back("root");
            _serverIds.push_back("index");
            _serverIds.push_back("return");
            _serverIds.push_back("cgi");
            _serverIds.push_back("access_log");
            _locationIds.push_back("root");
            _locationIds.push_back("index");
            _locationIds.push_back("methods");
            _locationIds.push_back("autoindex");
            _locationIds.push_back("upload_dir");
            _locationIds.push_back("cgi");
            _locationIds.push_back("fastcgi_pass");
            _locationIds.push_back("redirect");
            _locationIds.push_back("return");
            _locationIds.push_back("try_files");
            _locationIds.push_back("proxy_set_header");
            _locationIds.push_back("proxy_buffers");
            _locationIds.push_back("proxy_buffer_size");
            _locationIds.push_back("proxy_pass");
            _locationIds.push_back("method");
            _locationIds.push_back("default");
            _locationIds.push_back("upload");
            _locationIds.push_back("dirList");
            _locationIds.push_back("expires");
        }
        serverConf(serverConf const & rhs) : _valid(0), _directives(std::vector< std::string >(rhs._directives)),  _serverIds(std::vector< std::string >(rhs._serverIds)), _locationIds(std::vector< std::string >(rhs._locationIds)), http(std::vector< std::map< std::string, std::map< std::string, std::vector< std::string > > > >(rhs.http)) {}
        serverConf operator=(serverConf & rhs) { _valid = 0, _directives = std::vector< std::string >(rhs._directives), _serverIds = std::vector< std::string >(rhs._serverIds); _locationIds = std::vector< std::string >(rhs._locationIds); http = std::vector< std::map< std::string, std::map< std::string, std::vector< std::string > > > >(rhs.http); return *this; }
        virtual ~serverConf() {}
        std::string getContent(std::string file);
        std::string removeComments(std::string file);
        int brackets(std::string content);
        int topLevelDirectives(std::string content);
        int validSeparator(std::vector< std::string > ids, std::string content, size_t pos, size_t idx);
        int parseContent(std::string content);
        void pushServerIds(std::map< std::string, std::vector< std::string > > server);
        void pushLocationIds(std::map< std::string, std::vector< std::string > > location);
        int findRelevantId(std::string content, std::vector< std::string > ids, std::string *key, size_t pos);
        int getLocation(std::string content, std::string key, size_t *pos, bool *isLocation);
        int checkBlock(std::string content, bool isHttp);
        std::string getBlock(std::string content);
        std::string getBlockLocation(std::string content);
        int isValidLocation(std::string content, std::string location_name);
        int isValidServer(std::string content);
        int setLocationId(std::string name);
        int setServerId();
        void printMap();
        //int getData();
        int checkMissing();
        int checkNegValues();
};

serverConf start_conf(char *str);

#endif