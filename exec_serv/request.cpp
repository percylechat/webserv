#include "request.hpp"

void Request::set_error(int err){
    this->success = false;
    if (err == 400){
        //http.cat/400
        this->error_code = 400;
        this->error_msg = "Bad Request";
        this->response.append("400 ");
        this->response.append(this->error_msg);
        this->response.append(" Content-Type: text/html Context-Lenght: 91\n\n");
        this->response.append("<html><body>BAD REQUEST<img src=\"400.jpeg\" alt=\"\" width=\"600\" height=\"750\"> </body></html>");
    }
}

void Request::handle_get(char *msg){
    int i = 4;
    while (msg[i] != ' ')
        i++;
    std::string mess = &msg[5];
    std::string file = mess.substr(0, i - 5);
    std::cout << file << std::endl;
    std::ifstream file_data;
    file_data.open(file.c_str(), std::ios::in | std::ios::binary);



    struct stat results;
    int si;
    if (stat(file.c_str(), &results) == 0)
        // The size of the file in bytes is in
        si = results.st_size;
    else
        si = 0;
        // An error occurred
    // file_data.seekg(0, std::ios::end);
    // int file_size = file_data.tellg();
    //     file_data.seekg(0, std::ios::beg);
    // int check = 
    // if (check == -1)
    //     this->set_error(400);//???? error code
    this->response.append("200 ");
        this->response.append("OK");
        this->response.append(" Content-Type: image/gif Context-Lenght: ");
//convert int to string, then check how to put file in string

std::ifstream in_file(file.c_str(), std::ios::binary);
			in_file.seekg(0, std::ios::end);
			unsigned long long file_size = in_file.tellg();
			std::stringstream ss;
			ss << file_size;
			std::string test;
            ss >> test;
std::basic_ifstream<char> fin(file.c_str(), std::ios::binary);
			std::ostringstream oss;
			oss << fin.rdbuf();
			std::string data(oss.str());


        // char *buf;
        // itoa(file_size, buf, 10);
        // std::string lol = buf;
//         char mdr[si];
//         file_data.read(mdr, si);
//         std::string gr = mdr;
// std::cout << "hello" << si << std::endl;
//             std::stringstream sstream;
//             sstream << si;
//             std::string test1 = sstream.str();

                this->response.append(test);
        this->response.append("\n\n");
        // read(buf1, file_data);
        // this->response.append("<html><body>CHAT<img src=\"/chat.jpg\" alt=\"bebe percy\"></body></html>\n");
        this->response.append(data);
        this->response.append("\n");
}

Request::Request(){
    this->success = true;
    this->response = "HTTP/1.1 ";
}