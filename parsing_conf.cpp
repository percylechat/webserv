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

    if (length == -1)
    {
        is.close();
        return "";
    }
    char *buffer = new char[length];

    //std::cout << "Reading " << length << " characters... ";
    // read data as a block:
    is.read(buffer,length);

    //if (is)
      //std::cout << "all characters read successfully.";
    //else
      //std::cout << "error: only " << is.gcount() << " could be read";

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
    location.insert(std::pair< std::string, std::vector< std::string > >("proxy_set_header", std::vector< std::string >()));
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

int serverConf::validSeparator(std::vector< std::string > ids, std::string content, size_t pos, size_t idx)
{
    size_t prevIdIndex = 0;
    size_t index = 0;
    bool found = 0;

    while (index < ids.size())
    {
        if (content.find(ids[index], pos) != std::string::npos && isspace(content.at(content.find(ids[index], pos) + ids[index].length())) && \
        (!pos || (pos && isspace(content.at(content.find(ids[index], pos) - 1))))) // this line can be removed if cgi ID is renamed
        {
            if (content.find(ids[index], pos) < prevIdIndex || prevIdIndex == 0)
                prevIdIndex = content.find(ids[index], pos);
            found = 1;
        }
        index++;
    }
    if (idx > prevIdIndex && found)
        return FALSE;
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
        while (pos < content.length() && isspace(content.at(pos)))
            pos++;
        if (pos == content.length())
            return TRUE;
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
        else if (content.find(";", pos) != std::string::npos && content.find(";", pos) != pos && (isspace(content.at(content.find(";", pos) + 1)) || content.at(content.find(";", pos) + 1) == '}'))
        {
            idx = content.find(";", pos);
            if (validSeparator(_locationIds, content, pos + key.length(), idx) == FALSE)
                return FALSE;
        }
        else
            return FALSE;
        i = 0;
        std::string rawContent = content.substr(pos + key.length(), idx - (pos + key.length()));
        while (i < rawContent.length() && isspace(rawContent.at(i)))
            i++;
        if (!rawContent[i])
            return FALSE;
        i = 0;
        std::string trimContent = "";
        if (rawContent.find_first_not_of("\t\n\r\v\f ") != std::string::npos)
            trimContent = rawContent.substr(rawContent.find_first_not_of("\t\n\r\v\f "), rawContent.length() - rawContent.find_first_not_of("\t\n\r\v\f "));
        else
            trimContent = rawContent;
        pos = idx + 1;
        if (key == "autoindex" && trimContent == "on")  
            trimContent = "1";
        else if (key == "autoindex" && trimContent == "off")  
            trimContent = "0";
        else if (key == "autoindex")
            return FALSE;
        http.data()[http.size() - 1][category][key].push_back(trimContent);
    }
    return TRUE;
}

int serverConf::getLocation(std::string content, std::string key, size_t *pos, bool *isLocation)
{
    std::string locationName = "";
    std::string blockLocation = "";
    size_t i = 0;

    if (content.find(key, *pos) != std::string::npos && content.substr(content.find(key, *pos) + key.length()).find("{", 0) != std::string::npos)
        locationName = content.substr(content.find(key, *pos) + key.length(), content.substr(content.find(key, *pos) + key.length()).find("{", 0));
    else
        return FALSE;
    if (locationName.find_first_not_of("\t\n\r\v\f ") != std::string::npos && locationName.find_last_not_of("\t\n\r\v\f ") != std::string::npos)
        locationName = locationName.substr(locationName.find_first_not_of("\t\n\r\v\f "), locationName.find_last_not_of("\t\n\r\v\f "));
    else
        return FALSE;
    setLocationId(locationName);
    if (content.find("{", *pos) != std::string::npos)
        blockLocation = getBlockLocation(&content[content.find("{", *pos) + 1]);
    else
        return FALSE;    
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
        if (findRelevantId(content, _serverIds, &key, pos) == FALSE)
            return FALSE;
        if (key == "location")
        {
            if (getLocation(content, key, &pos, &isLocation) == FALSE)
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
            else if (content.find(";", pos) != std::string::npos && content.find(";", pos) != pos && (isspace(content.at(content.find(";", pos) + 1)) || content.at(content.find(";", pos) + 1) == '}'))
            {
                idx = content.find(";", pos);
                if (validSeparator(_serverIds, content, pos + key.length(), idx) == FALSE)
                    return FALSE;
            }
            else
                return FALSE;
            i = 0;
            std::string rawContent = content.substr(pos + key.length(), idx - (pos + key.length()));
            while (i < rawContent.length() && isspace(rawContent.at(i)))
                i++;
            if (!rawContent[i])
                return FALSE;
            i = 0;
            std::string trimContent = "";
            if (rawContent.find_first_not_of("\t\n\r\v\f ") != std::string::npos)
                trimContent = rawContent.substr(rawContent.find_first_not_of("\t\n\r\v\f "), rawContent.length() - rawContent.find_first_not_of("\t\n\r\v\f "));
            else
                trimContent = rawContent;
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

std::string serverConf::getBlock(std::string content)
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

int serverConf::brackets(std::string content)
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
    return TRUE;
}

int serverConf::checkBlock(std::string content, bool isHttp)
{
    size_t pos = 0;

    while (pos < content.length())
    {
        if (isHttp)
        {
            while (pos < content.length() && isspace(content.at(pos)))
                pos++;
            if (!content.compare(pos, std::string("server").length(), "server"))
            {
                pos += std::string("server").length();
                while (pos < content.length() && isspace(content.at(pos)))
                    pos++;
                if (content.at(pos) != '{')
                    return FALSE;
                if (content.rfind("}", content.length()) == std::string::npos)
                    return FALSE;
                else
                    pos = content.rfind("}", content.length()) + 1;
            }
        }
        while (pos < content.length() && isspace(content.at(pos)))
            pos++;
        if (pos == content.length())
            return TRUE;
        else if (content.find(";", pos) != std::string::npos && content.find(";", pos) != pos)
            pos = content.find(";", pos) + 1;
        else
            return FALSE;
    }
    return TRUE;
}

int serverConf::topLevelDirectives(std::string content)
{
    size_t pos = 0;
    size_t i = 0;
    size_t count[4] = {0, 0, 0, 0};
    std::string block = "";
    size_t knownDirective = 0;

    while (pos < content.length())
    {
        while (pos < content.length() && isspace(content.at(pos)))
            pos++;
        i = 0;
        while (i < _directives.size())
        {
            if (!content.compare(pos, _directives[i].length(), _directives[i]))
            {
                count[i]++;
                knownDirective++;
                pos += _directives[i].length();
                break ;
            }
            i++;
        }
        if (content.find("{", pos) != std::string::npos && knownDirective)
        {
            block = getBlock(&content[content.find("{", pos) + 1]);
            if (checkBlock(block, (i == 1)) == FALSE)
                return FALSE;
            pos = block.length() + content.find("{", pos) + 1;
            if (content.find("}", pos) != std::string::npos)
                pos = content.find("}", pos) + 1;
            else
                return FALSE;
        }
        else
        {
            while (pos < content.length() && isspace(content.at(pos)))
                pos++;
            if (pos == content.length())
                return TRUE;
            return FALSE;
        }
    }
    i = 0;
    while (i < 4)
    {
        if (count[i] > 1)
            return FALSE;
        if (i == 1 && count[i] == 0)
            return FALSE;
        i++;
    }
    return TRUE;
}

int serverConf::parseContent(std::string content)
{
    if (brackets(content) == FALSE)
        return FALSE;
    if (topLevelDirectives(content) == FALSE)
        return FALSE;
    size_t pos = 0;
    std::string blockServer = "";
    while (pos != content.length())
    {
        if ((pos = content.find("server", pos)) != std::string::npos)
        {
            pos += std::string("server").length();
            if (content.find("{", pos) != std::string::npos)
                blockServer = getBlock(&content[content.find("{", pos) + 1]);
            else
                return FALSE;
            pos = content.find("{", pos) + blockServer.length();
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
/*
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
            std::cout << "nom de la location : " << it->first << std::endl;
            while (j < http.data()[i][it->first]["index"].size())
            {
                std::cout << http.data()[i]["server"]["client_max_body_size"][j] << std::endl;
                j++;
            }
        }
        j = 0;
    }
    //pour un serveur et une location, recuperer autoindex
    //pour un serveur et une location, recuperer l'upload dir
    //pour un serveur et une location, recuperer cgi
    //pour un serveur et une location, recuperer redirect
    // -> pareil
    return 0;
}*/

int serverConf::checkMissing()
{
    size_t i = 0;
    size_t j = 1;

    if (http.size() == 0)
        return FALSE;
    while (i < http.size())
    {
        if (http.data()[i]["server"]["listen"].empty())
        {
            std::cout << "server " << i << " listen/port missing" << std::endl;
            return FALSE;
        }
        if (http.data()[i]["server"]["root"].empty())
        {
            std::cout << "server " << i << " root missing" << std::endl;
            return FALSE;
        }
        if (j == http.data()[i].size())
        {
            std::cout << "no location in server " << i << std::endl;
            return FALSE;
        }
        i++;
    }
    return TRUE;
}

int serverConf::checkNegValues()
{
    size_t i = 0;
    size_t j = 0;

    if (http.size() == 0)
        return FALSE;
    while (i < http.size())
    {
        if (!http.data()[i]["server"]["listen"].empty())
        {
            while (j < http.data()[i]["server"]["listen"].size())
            {
                if (atoi(http.data()[i]["server"]["listen"][j].c_str()) < 0)
                {
                    std::cout << "server " << i << " invalid port - negative number" << std::endl;
                        return FALSE;
                }
                j++;
            }
        }
        j = 0;
        if (!http.data()[i]["server"]["client_max_body_size"].empty())
        {
            while (j < http.data()[i]["server"]["client_max_body_size"].size())
            {
                if (atoi(http.data()[i]["server"]["client_max_body_size"][j].c_str()) < 0)
                {
                    std::cout << "server " << i << " invalid client_max_body_size - negative number" << std::endl;
                        return FALSE;
                }
                j++;
            }
        }
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
    //empty file
    ret = !noComment.empty();
    std::cout << "is not empty : " << ret << std::endl;
    //correct format
    ret = (ret) ? conf.parseContent(noComment) : 0;
    std::cout << "is correct format : " << ret << std::endl;
    //missing info
    ret = (ret) ? conf.checkMissing() : 0;
    ret = (ret) ? conf.checkNegValues() : 0;
    //clé valeurs
    conf.printMap();
    //is valid ?
    conf._valid = ret;
    if (conf._valid)
        std::cout << str << " is valid" << std::endl;
    else
        std::cout << str << " is not valid" << std::endl;
    return conf;
}