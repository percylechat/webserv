#include "parsing_conf.hpp"

std::string serverConf::getContent(std::string file)
{
    std::string ret = "";
    std::ifstream is (file.c_str(), std::ifstream::binary);
    if (is) {
    // get length of file:
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);

    char *buffer = new char[length];

    std::cout << "Reading " << length << " characters... ";
    // read data as a block:
    is.read(buffer,length);

    if (is)
      std::cout << "all characters read successfully.";
    else
      std::cout << "error: only " << is.gcount() << " could be read";
    is.close();

    // ...buffer contains the entire file...

    ret = std::string(buffer, length);
    delete [] buffer;
  }
  return ret;
}

std::string serverConf::removeComments(std::string file)
{
    size_t pos = 0;
    size_t i = 0;
    size_t j = 0;
    std::string buf = "";

    while (file.find("#", pos) != std::string::npos)
    {
        i = file.find("#", pos);
        if (file.find("\n", i) != std::string::npos)
        {
            j = file.find("\n", i);
            buf += file.substr(pos, i - pos);
        }
        else
            return buf ;
        pos = j;
    }
    buf += file.substr(pos, file.length() - pos);
    return buf;
}

void serverConf::pushServerIds(std::map< std::string, std::vector< std::string > > server)
{
    //server.insert(std::pair< std::string, std::vector< std::string > >("location", std::vector< std::string >()));
    server.insert(std::pair< std::string, std::vector< std::string > >("listen", std::vector< std::string >()));
    server.insert(std::pair< std::string, std::vector< std::string > >("server_name", std::vector< std::string >()));
    server.insert(std::pair< std::string, std::vector< std::string > >("client_max_body_size", std::vector< std::string >()));
    server.insert(std::pair< std::string, std::vector< std::string > >("error_page", std::vector< std::string >()));
    server.insert(std::pair< std::string, std::vector< std::string > >("root", std::vector< std::string >()));
    server.insert(std::pair< std::string, std::vector< std::string > >("index", std::vector< std::string >()));
    server.insert(std::pair< std::string, std::vector< std::string > >("return", std::vector< std::string >()));
    server.insert(std::pair< std::string, std::vector< std::string > >("cgi", std::vector< std::string >()));
    server.insert(std::pair< std::string, std::vector< std::string > >("access_log", std::vector< std::string >()));
}

void serverConf::pushLocationIds(std::map< std::string, std::vector< std::string > > location)
{
    location.insert(std::pair< std::string, std::vector< std::string > >("root", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("index", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("methods", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("autoindex", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("upload_dir", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("cgi", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("redirect", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("return", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("try_files", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("proxy_set_headers", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("proxy_buffers", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("proxy_buffer_size", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("proxy_pass", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("method", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("default", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("upload", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("dirList", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("fastcgi_pass", std::vector< std::string >()));
    location.insert(std::pair< std::string, std::vector< std::string > >("expires", std::vector< std::string >()));
}

int serverConf::setLocationId(std::string name)
{
    std::map< std::string, std::vector< std::string > > location;

    pushLocationIds(location);
    http.data()[http.size() - 1].insert(std::make_pair("location " + name, location));
    return TRUE;
}

int serverConf::findRelevantId(std::string content, std::vector< std::string > ids, std::string *key, size_t pos)
{
    size_t i = 0;
    bool found = 0;
    size_t prevIdPos = 0;

    while (i < ids.size())
    {
        if (content.find(ids[i], pos) != std::string::npos && isspace(content.at(content.find(ids[i], pos) + ids[i].length())))
        {
            if (content.find(ids[i], pos) < prevIdPos || prevIdPos == 0)
            {
                *key = ids[i];
                prevIdPos = content.find(ids[i], pos);
            }
            found = 1;
        }
        i++;
    }
    if (!found)
    {
        i = 0;
        while (pos + i < content.length() && isspace(content.at(pos + i)))
            i++;
        if (pos + i == content.length())
            return TRUE;
        return FALSE;
    }
    return TRUE;
}

int serverConf::isValidLocation(std::string content, std::string locationName)
{
    size_t pos = 0;
    size_t idx = 0;
    size_t i = 0;
    std::string key = "";
    std::string category = "";

    while (pos != content.length())
    {
        if (findRelevantId(content, _locationIds, &key, pos) == FALSE)
            return FALSE;
        if (http.data()[http.size() - 1]["location " + locationName][key].empty())
        {
            pos = content.find(key, pos);
            category = "location " + locationName;
        }
        while (pos + i < content.length() && isspace(content.at(pos + i)))
            i++;
        if (pos + i == content.length())
            return TRUE;
        else if (content.find(";", pos) != std::string::npos)
            idx = content.find(";", pos);
        else
            return FALSE;
        i = 0;
        std::vector< std::string > value;
        std::string rawContent = content.substr(pos + key.length(), idx - (pos + key.length()));
        std::string trimContent = rawContent.substr(rawContent.find_first_not_of("\t\n\r\v\f "), rawContent.length() - rawContent.find_first_not_of("\t\n\r\v\f "));
        value.push_back(trimContent);
        pos = idx + 1;
        http.data()[http.size() - 1][category][key].push_back(trimContent);
    }
    return TRUE;
}

int serverConf::getLocation(std::string content, std::string *key, size_t *pos, bool *isLocation)
{
    std::string locationName = "";
    std::string blockLocation = "";
    size_t i = 0;

    locationName = content.substr(content.find(*key, *pos) + key->length(), content.substr(content.find(*key, *pos) + key->length()).find("{", 0));
    locationName = locationName.substr(locationName.find_first_not_of("\t\n\r\v\f "), locationName.find_last_not_of("\t\n\r\v\f "));
    setLocationId(locationName);
    if (content.find("{", *pos) != std::string::npos)
        blockLocation = getBlockLocation(&content[content.find("{", *pos) + 1]);
    if (isValidLocation(blockLocation, locationName) == FALSE)
        return FALSE;
    if (content.find("}", *pos) != std::string::npos)
        *pos = content.find("}", *pos) + 1;
    while (*pos + i < content.length() && isspace(content.at(*pos + i)))
        i++;
    if (*pos + i == content.length())
        return TRUE;
    *isLocation = 1;
    return TRUE;
}

int serverConf::isValidServer(std::string content)
{
    size_t pos = 0;
    size_t idx = 0;
    size_t i = 0;
    std::string key = "";
    std::string category = "";
    bool isLocation = 0;

    while (pos != content.length())
    {
        isLocation = 0;
        if (findRelevantId(content, _ServerIds, &key, pos) == FALSE)
            return FALSE;
        if (key == "location")
        {
            if (getLocation(content, &key, &pos, &isLocation) == FALSE)
                return FALSE;
        }
        else
        {
            if (content.find(key, pos) != std::string::npos)
                pos = content.find(key, pos);
            category = "server";
        }
        if (!isLocation)
        {
            while (pos + i < content.length() && isspace(content.at(pos + i)))
                i++;
            if (pos + i == content.length())
                return TRUE;
            else if (content.find(";", pos) != std::string::npos)
                idx = content.find(";", pos);
            else
                return FALSE;
            i = 0;
            std::string rawContent = content.substr(pos + key.length(), idx - (pos + key.length()));
            std::string trimContent = rawContent.substr(rawContent.find_first_not_of("\t\n\r\v\f "), rawContent.length() - rawContent.find_first_not_of("\t\n\r\v\f "));
            pos = idx + 1;
            http.data()[http.size() - 1][category][key].push_back(trimContent);
        }
    }
    return TRUE;
}

int serverConf::setServerId()
{
    std::map< std::string, std::vector< std::string > > server;

    pushServerIds(server);
    std::map< std::string, std::map< std::string, std::vector< std::string > > > map_ids;
    map_ids.insert(std::make_pair("server", server));
    http.push_back(map_ids);
    return TRUE;
}

std::string serverConf::getBlockServer(std::string content)
{
    size_t i = 0;
    size_t count1 = 0;
    size_t count2 = 0;

    while (i < content.length())
    {
        if (content.at(i) == '{')
            count1++;
        if (content.at(i) == '}')
            count2++;
        if (count2 > count1)
            return content.substr(0, i);
        i++;
    }
    return content.substr(0, i);
}

std::string serverConf::getBlockLocation(std::string content)
{
    size_t i = 0;

    while (i < content.length())
    {
        if (content.at(i) == '}')
            return content.substr(0, i);
        i++;
    }
    return content.substr(0, i);
}

int serverConf::parseContent(std::string content)
{
    size_t posStart = 0;
    size_t posEnd = content.length();
    while (posStart != posEnd)
    {
        if ((posStart = content.find("{", posStart)) == std::string::npos)
        {
            if ((posEnd = content.rfind("}", posEnd)) == std::string::npos)
                break ;
            else
                return FALSE;
        }
        if ((posEnd = content.rfind("}", posEnd)) == std::string::npos)
            return FALSE;
        posStart++;
        posEnd--;
    }
    size_t server_idx = 0;
    std::string blockServer = "";
    while (server_idx != content.length())
    {
        if ((server_idx = content.find("server", server_idx)) != std::string::npos)
        {
            if (content.find("{", server_idx + 7) != std::string::npos)
                blockServer = getBlockServer(&content[content.find("{", server_idx + 7) + 1]);
            server_idx = content.find("{", server_idx + 7) + blockServer.length();
            setServerId();
            if (isValidServer(blockServer) == FALSE)
                return FALSE;
        }
        else
            return TRUE;
    }
    return TRUE;
}

void serverConf::printMap()
{
    size_t i = 0;
    size_t j = 0;
    while (i < http.size())
    {
        std::cout << "indice : [" << i << "]" << std::endl;
        std::cout << "*************" << std::endl;
        for (std::map< std::string, std::map< std::string, std::vector< std::string > > >::iterator it = http.data()[i].begin(); it != http.data()[i].end(); it++)
        {
            std::cout << "clé générale : [" << it->first << "]";
            for (std::map< std::string, std::vector< std::string > >::iterator itk = http.data()[i][it->first].begin(); itk != http.data()[i][it->first].end(); itk++)
            {
                std::cout << " | clé [" << itk->first << "]";
                while (j < itk->second.size())
                {
                    std::cout << " | valeur [" << itk->second[j] << "]";
                    j++;
                }
                j = 0;
            }
            std::cout << std::endl;
        }
        std::cout << "*************" << std::endl;
        std::cout << std::endl;
        i++;
    }
}

int serverConf::getData()
{
    std::cout << "Pour chaque serveur, récupérer son port (crea socket) " << std::endl;
    size_t i = 0;
    size_t j = 0;

    while (i < http.size())
    {
        while (j < http.data()[i]["server"]["listen"].size())
        {
            std::cout << http.data()[i]["server"]["listen"][j] << std::endl;
            j++;
        }
        j = 0;
        i++;
    }
    std::cout << "Pour chaque serveur, récupérer son nbr max de connexion(taille) (crea socket) " << std::endl;
    i = 0;
    j = 0;

    while (i < http.size())
    {
        while (j < http.data()[i]["server"]["client_max_body_size"].size())
        {
            std::cout << http.data()[i]["server"]["client_max_body_size"][j] << std::endl;
            j++;
        }
        j = 0;
        i++;
    }
    std::cout << "Pour chaque serveur, recupérer son nom (check socket) " << std::endl;
    i = 0;
    j = 0;

    while (i < http.size())
    {
        while (j < http.data()[i]["server"]["server_name"].size())
        {
            std::cout << http.data()[i]["server"]["server_name"][j] << std::endl;
            j++;
        }
        j = 0;
        i++;
    }
    std::cout << "pour un serveur, recupere le port (gestion requete) " << std::endl;
    i = 2; //index du serveur;
    j = 0;

    while (j < http.data()[i]["server"]["listen"].size())
    {
        std::cout << http.data()[i]["server"]["listen"][j] << std::endl;
        j++;
    }
    std::cout << "pour un serveur, recuperer error page(gestion requete) " << std::endl;
    i = 0; //index du serveur;
    j = 0;

    while (j < http.data()[i]["server"]["error_page"].size())
    {
        std::cout << http.data()[i]["server"]["error_page"][j] << std::endl;
        j++;
    }
    std::cout << "pour un serveur, recuperer  la ou les roots (gestion requete) " << std::endl;
    i = 0; //index du serveur;
    j = 0;

    while (j < http.data()[i]["server"]["root"].size())
    {
        std::cout << http.data()[i]["server"]["root"][j] << std::endl;
        j++;
    }
    std::cout << "pour un serveur et une location, recuperer l'index " << std::endl;
    i = 0;
    j = 0;
    std::map< std::string, std::map< std::string, std::vector< std::string > > >::iterator it = http.data()[i].begin();
    std::map< std::string, std::map< std::string, std::vector< std::string > > >::iterator ite = http.data()[i].end();

    for(; it != ite; it++)
    {
        if (it->first == "server")
        {
            while (j < http.data()[i][it->first]["index"].size())
            {
                std::cout << http.data()[i][it->first]["index"][j] << std::endl;
                j++;
            }
        }
        else
        {
            j = 0;
            std::cout << "nom de la location : " << it->first << std::endl;
            while (j < http.data()[i][it->first]["index"].size())
            {
                std::cout << http.data()[i]["server"]["client_max_body_size"][j] << std::endl;
                j++;
            }
        }
    }
    //pour un serveur et une location, recuperer autoindex
    //pour un serveur et une location, recuperer l'upload dir
    //pour un serveur et une location, recuperer cgi
    //pour un serveur et une location, recuperer redirect
    // -> pareil
    return 0;
}

int serverConf::checkMissing()
{
    size_t i = 0;
    size_t j = 1;

    if (http.size() == 0)
        return FALSE;
    while (i < http.size())
    {
        if (!http.data()[i]["server"]["listen"].size())
            return FALSE;
        if (!http.data()[i]["server"]["root"].size())
            return FALSE;
        if (j == http.data()[i].size())
            return FALSE;
        i++;
    }
    return TRUE;
}

serverConf start_conf(char *str)
{
    serverConf conf;
    std::vector< std::string > empty;
    std::string noComment = "";
    int ret = 0;

    std::string file = str;
    std::string output = "";
    output += conf.getContent(file);
    noComment = conf.removeComments(output);
    std::cout << noComment << std::endl;
    if (noComment.empty())
    {
        std::cout << "IS VALID - EMPTY FILE 0" << std::endl;
        return conf;
    }
    else
        std::cout << "IS VALID - EMPTY FILE 1" << std::endl;
    ret = conf.parseContent(noComment);
    std::cout << "IS VALID FORMAT " << ret << std::endl;
    //if (!ret)
    //    return FALSE;
    ret = conf.checkMissing();
    std::cout << "IS VALID - MISSING INFO " << ret << std::endl;
    //if (!ret)
    //    return FALSE;
    conf.printMap();
    conf.getData();
    return conf;
}