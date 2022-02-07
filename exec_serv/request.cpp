#include "request.hpp"

void Request::set_error(int err){
    this->error_code = err;
    this->success = false;
    if (err == 400){
// for now for missing extension in file
        this->error_msg = "Bad Request";
        this->response.append("400 ");
        this->response.append(this->error_msg);
        this->response.append(" Content-Type: text/html Context-Lenght: 91\n\n");
        this->response.append("<html><body>BAD REQUEST<img src=\"error/400.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
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
    else if (err == 500){
// For now, couldn't delete file
        this->error_msg = "Internal Server Error";
        this->response.append("500 ");
        this->response.append(this->error_msg);
        this->response.append(" Content-Type: text/html Context-Lenght: 112\n\n");
        this->response.append("<html><body>500 INTERNAL SERVER ERROR <img src=\"error/500.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
}

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
    std::string file = this->get_file(msg);
    if (remove(file.c_str()) != 0)
        this->set_error(500);
    else
        this->response.append("200 OK");
}

// CHECK \n ou \r pour fin de ligne??
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
std::cout << "hello" << std::endl;
    int file_start = mess.find("Content-Length: ") + 16;
    if (file_start == 16)
//maybe other?
        return this->set_error(400);
    int file_end = file_start;
std::cout << "hello" << std::endl;
    while (mess[file_end] != '\r' && mess[file_end] != '\n')
        file_end++;
std::cout << "hello" << std::endl;
    std::string size = mess.substr(file_start, file_end);
    int len = atoi(size.c_str());
    file_start = mess.find("\n\n") + 2;
std::cout << "hello size" << len << std::endl;
    std::string content = mess.substr(file_start, len);
    std::basic_ofstream<char> fin(filename.c_str(), std::ios::out | std::ios::binary);
    for (std::size_t i = 0; i < content.size(); ++i)
  {
      fin << std::bitset<8>(content.c_str()[i]);
  }
    // std::cout << content << std::endl;
	fin << content;
    fin.close();
std::cout << "hello" << std::endl;
        this->response.append("200 OK");
	// std::string data(oss.str());
}

// void Request::handle_multipart(std::string mess){
//     if (mess.find("Content-Disposition: ") != mess.npos)

// }

void Request::handle_post(char *msg, int fd){
//https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Content-Disposition
    this->fd = fd;
    std::string mess(msg);
    // int type = mess.find("Content-Type: ") + 15;
    // if (type == 15)
    //     return this->set_error(400);
    // std::string content_type = mess.substr(type, mess.find(" \n", mess[type]));
    // if (content_type == "multipart/form-data")
    //     return handle_multipart(mess);
    // if (content_type != "application/x-www-form-urlencoded" && content_type != "multipart/form-data" && content_type != "text/plain")
        return this->get_file_post(mess);
// Content-Disposition: form-data; name="files[]"; filename="chat_bulle.jpeg"\r
    // std::string file_name = mess.substr()
}
// POST / HTTP/1.1
// [[ Less interesting headers ... ]]
// Content-Type: application/x-www-form-urlencoded
// Content-Length: 51

// text1=text+default&text2=a%CF%89b&file1=a.txt&file2=a.html&file3=binary

void Request::handle_get(char *msg, int fd){
    this->fd = fd;
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