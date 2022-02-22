#include "request.hpp"

//TO DO create "key" value that is checked to add stuff (serevarl keys?)

// void Request::go_cgi_get(std::string file){
// need env? mais env total ou que celui de server?
    // char *env = getenv(env);
// devra probablement parser requete pour infos
// dup la socket d envoi pour renvoyer infos?
// dup socket vers client pour que exexve parte dedqns
//bordel av ec pipes et fork et execve
// envoyer contenu direct ou avec header avant? Si oui avant execve?
// }

void Request::handle_delete(char *msg){
// TO DO change in order to delete elt in vector
    std::string file = this->get_file(msg);
    if (remove(file.c_str()) != 0)
        this->set_error(500);
    else
        this->response.append("200 OK");
}

// TO DO CHECK \n ou \r pour fin de ligne??
// WIP, can't get img file content for now, txt okay
void Request::get_file_post(std::string mess){
//     int file_start = mess.find("filename=\"") + 11;
//     if (file_start == 11)
// //maybe other?
//         return this->set_error(400);
//     int file_end = file_start;
//     while (mess[file_end] != '\"')
//         file_end++;
//     std::string filename = mess.substr(file_start, file_end);
std::string filename = "test.jpg";
std::cout << mess <<std::endl;
std::cout << "hello1" << std::endl;
    int file_start = mess.find("Content-Length: ") + 16;
    if (file_start == 16)
//maybe other?
        return this->set_error(411);
    int file_end = file_start;
std::cout << "hello2" << std::endl;
    while (mess[file_end] != '\r' && mess[file_end] != '\n' && mess[file_end] != ' ')
        file_end++;
std::cout << "hello3" << std::endl;
    std::string size = mess.substr(file_start, file_end);
    int len = atoi(size.c_str());
    file_start = mess.find("\r\n\r\n") + 4;
    std::string content = mess.substr(file_start, len);
std::cout << "hello size" << len << std::endl;
std::cout << content << std::endl;
std::ostringstream oss;
// if ((fd = open(filename.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644)) == -1)
// 						this->set_error(500);
					for (int i = 0; i < len; ++i)
  {
      oss << std::bitset<8>(content.c_str()[i]);
  }
                    // write(fd, content.c_str(), len);

					// close(fd);

std::cout << "hello4" << std::endl;
    std::basic_ofstream<char> fin(filename.c_str(), std::ios::app);
    fin << oss.rdbuf();
    // // std::cout << content << std::endl;
	// fin << content;
    fin.close();
std::cout << "hello end" << std::endl;
        this->response.append("200 OK");
	// std::string data(oss.str());
}

// POST /test.html HTTP/1.1
// Host: example.org
// Content-Type: multipart/form-data;boundary="boundary"

// --boundary
// Content-Disposition: form-data; name="field1"

// value1
// --boundary
// Content-Disposition: form-data; name="field2"; filename="example.txt"

// value2
void Request::handle_multipart(std::string mess, std::vector<std::string> cat){
    std::size_t start = mess.find("boundary=\"");
    if (start == mess.npos)
        return this->set_error(400);
    start += 11;
    size_t end = start;
    while (mess[end] != '"' && mess[end - 1] != '\\')
        end++;
// TO DO Encapsulation boundaries must not appear within the encapsulations, and must be no longer than 70 characters, not counting the two leading hyphens. 
    std::string boundary = mess.substr(start, end - start);
    if (boundary.size() > 70)
        return this->set_error(400);
    int i = count_occ(mess, boundary);
    int j = 1;
    std::size_t stop = mess.find("name=\"");
    if (stop == mess.npos)
        return this->set_error(400);
    std::string content = mess.substr(stop, mess.size() - stop);
// TO DO what if malformed request with no boundary
    while (j < i){
        std::size_t m = content.find("name=\"");
        if (m == content.npos)
            return this->set_error(400);
        m += 6;
        std::size_t n = m;
        while (content[n] != '\"')
            n++;
        std::string key = content.substr(m, n - m);
// TO DO content disposition and filename and content type
        std::size_t u = n += 1;
        while (content[u] != '\n'){
            std::cout << content[u] << std::endl;
            u++;
        }
        u += 2;
        std::size_t f = u;
        while (content[f] != ' ' && content[f] != '\n' && content[f] != '\r' && content[f] != '-')
            f++;
        std::string value = content.substr(u, f - u);
        if (key == "cat")
            cat.push_back(value);
        if (i - 1 != j)
            content = content.substr(f + boundary.size() + 3, content.size() - (3 + f + boundary.size()));
        j++;
    }
}

// POST / HTTP/1.1
// [[ Less interesting headers ... ]]
// Content-Type: application/x-www-form-urlencoded
// Content-Length: 51

// text1=text+default&text2=a%CF%89b&file1=a.txt&file2=a.html&file3=binary
void Request::extract_data_post(std::string mess, std::vector<std::string> cat){
//TO DO  Les caractères non alphanumériques sont percent encoded 
    int file_start = mess.find("Content-Length: ") + 16;
    if (file_start == 16)
        return this->set_error(411);
    int file_end = file_start;
    while (mess[file_end] != '\r' && mess[file_end] != '\n' && mess[file_end] != ' ')
        file_end++;
    std::string size = mess.substr(file_start, file_end - file_start);
    int len = atoi(size.c_str());
    int start = mess.find("\n\n");
    if (start == 0 && len != 0)
        return this->set_error(400);
    start += 2;
    std::string content = mess.substr(start, mess.size() - start);
    int j = std::count(content.c_str(), content.c_str() + len, '&');
    // TO DO check if a key has no value, if so error?
    int k = -1;
    while (k < j){
        std::string key = content.substr(0, content.find("="));
        std::size_t stop = content.find_first_of(" \n&\r");
        if (stop == content.npos){
            std::string value = content.substr(key.size() + 1, content.size() - (key.size() + 1));
            if (key == "cat")
                cat.push_back(value);
            this->response.append("200 OK");
// TO DO create struct or class to return with elts from list
            return ;
        }
        std::string value = content.substr(key.size() + 1, stop - (key.size() + 1));
        if (key == "cat")
            cat.push_back(value);
        content = content.substr(stop + 1, content.size() - (stop + 1));
        k++;
    }
    this->response.append("200 OK");
}

void Request::handle_post(char *msg, int fd, std::vector<std::string> cat){
//https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Content-Disposition
    this->fd = fd;
    std::string mess(msg);
    std::size_t type = mess.find("Content-Type: ") + 14;
    if (type == 14)
        return this->set_error(411);
    size_t end = type;
    while (mess[end] != '\n' && mess[end] != ' ' && mess[end] != '\r')
        end++;
    std::string content_type = mess.substr(type, end - type);
    std::cout << "type " << content_type << "stop" << std::endl;
    std::cout << "comp " << strcmp(content_type.c_str(), "application/x-www-form-urlencoded") << std::endl;
    if (content_type == "multipart/form-data")
        return handle_multipart(mess, cat);
    else if (content_type == "application/x-www-form-urlencoded")
        return extract_data_post(mess, cat);
    else
        return this->get_file_post(mess);
}

void Request::handle_get(char *msg, int fd, std::vector<std::string> cat){
    this->fd = fd;
    (void)cat;
// TO DO check if open binary meme si pas image
    std::string file = this->get_file(msg);
    std::ifstream file_data;
    file_data.open(file.c_str(), std::ios::in | std::ios::binary);
//get byte size of file, if failed then not found
    struct stat results;
    int si;
    if (stat(file.c_str(), &results) == 0)
        si = results.st_size;
    else
        return set_error(404);
// 2nd response elt, numerical code and its readable signifiance
    this->response.append("200 OK Content-Type: ");
// 3rd elt, the content type returned and its size
    std::string ext = this->get_content_type(file);
    if (this->success == false)
        return ;
    // if (this->cgi == true)
    //     return go_cgi_get(file);
    this->response.append(ext);
    this->response.append(" Context-Lenght: ");
    // get size of file 
	file_data.seekg(0, std::ios::end);
	unsigned long long file_size = file_data.tellg();
	std::stringstream ss;
	ss << file_size;
	std::string test;
    ss >> test;
//get file content in binary
    std::basic_ifstream<char> fin(file.c_str(), std::ios::binary);
	std::ostringstream oss;
	oss << fin.rdbuf();
	std::string data(oss.str());
    this->response.append(test);
    this->response.append("\n\n");
    this->response.append(data);
    this->response.append("\n");
}

Request::Request(){
    this->success = true;
    this->cgi = false;
// first elt of response, the HTTP used for response
    this->response = "HTTP/1.1 ";
}

std::string Request::get_file(char *msg){
    int i = 4;
    while (msg[i] != ' ')
        i++;
// start at 4 to get rid of GET, then search end of file name and extract
// TO DO parse before to be sure to start at the right place
    std::string mess = &msg[5];
    return mess.substr(0, i - 5);
}

std::string Request::get_content_type(std::string file){
    int i = file.find_last_of(".");
    if (i == -1){
        this->set_error(400);
        return NULL;
    }
    std::string ext = file.substr(i + 1);
    if (ext == "html")
        return "text/" + ext;
    else if (ext == "cgi"){
        this->cgi = true;
        return "test/html";
    }
    else
        return "image/" + ext;
}

void Request::set_error(int err){
    this->error_code = err;
    this->success = false;
    if (err == 400){
// for now for missing extension in file
        this->error_msg = "Bad Request";
        this->response.append("400 ");
        this->response.append(this->error_msg);
        this->response.append(" Content-Type: text/html Context-Lenght: 101\n\n");
        this->response.append("<html><body>400 BAD REQUEST<img src=\"error/400.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
    else if (err == 404){
// for now, couldn't open file so does not exist
        this->error_msg = "Not Found";
        this->response.append("404 ");
        this->response.append(this->error_msg);
        this->response.append(" Content-Type: text/html Context-Lenght: 91\n\n");
        this->response.append("<html><body>404 NOT FOUND<img src=\"error/404.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
    else if (err == 405){
// is not GET POST or DELETE
        this->error_msg = "Method Not Allowed";
        this->response.append("405 ");
        this->response.append(this->error_msg);
        this->response.append(" Content-Type: text/html Context-Lenght: 109\n\n");
        this->response.append("<html><body>405 METHOD NOT ALLOWED <img src=\"error/405.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
    else if (err == 411){
// content lenght missing
        this->error_msg = "Length Required";
        this->response.append("411 ");
        this->response.append(this->error_msg);
        this->response.append(" Content-Type: text/html Context-Lenght: 109\n\n");
        this->response.append("<html><body>411 LENGHT REQUIRED <img src=\"error/411.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
    else if (err == 500){
// For now, couldn't delete file
        this->error_msg = "Internal Server Error";
        this->response.append("500 ");
        this->response.append(this->error_msg);
        this->response.append(" Content-Type: text/html Context-Lenght: 112\n\n");
        this->response.append("<html><body>500 INTERNAL SERVER ERROR <img src=\"error/500.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
    else if (err == 505){
// bad hhtp protocol version
        this->error_msg = "HTTP Version not supported";
        this->response.append("505 ");
        this->response.append(this->error_msg);
        this->response.append(" Content-Type: text/html Context-Lenght: 57\n\n");
        this->response.append("<html><body>500 HTTP VERSION NOT SUPPORTED</body></html>");
    }
}