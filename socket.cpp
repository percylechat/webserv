#include "socket.hpp"

/////////////////////////////////////////////
//GLOBAL
/////////////////////////////////////////////
int Socket::get_fd(){
    return this->fd_sock;
}
// const char * Socket::failed_socket::what(char * msg) const throw()
// {
//     return msg;
// }
Socket::failed_socket::failed_socket(char *msg){
    this->msg = msg;
}
const char *Socket::failed_socket::what() const throw(){
    return this->msg;
}
/////////////////////////////////////////////
//SERVER SOCKET
/////////////////////////////////////////////

// Socket::Socket(int domain, int service, int protocol, int port, u_long interface, bool server){
Socket::Socket(int port, in_addr_t interface, bool server){
    this->is_server = server;
// preparation pour l'assignation de la,socket a un port(bind)
    this->address.sin_family = AF_INET; //(AF_INET?)
    this->address.sin_port = htons(port); // A CHECK BONNE CONV
    this->address.sin_addr.s_addr = htonl(interface);
    this->fd_sock = socket(AF_INET, SOCK_STREAM, 0);
// DOMAIN: communication domain in which the socket should be created.(ipv4, ipv6)
// SERVICE: type of service. This is selected according to the properties required by the application: SOCK_STREAM (virtual circuit service)
// PROTOCOL: For TCP/IP sockets, we want to specify the IP address family (AF_INET) and virtual circuit service (SOCK_STREAM).
// Since there’s only one form of virtual circuit service, there are no variations of the protocol, so the last argument, protocol, is zero.
    if (this->fd_sock == -1)
        throw failed_socket((char*)"Error: failure to create server socket");
}
void Socket::server_binding(){
//this part linked to memset that we try not to do?
    if (sizeof(this->address) < 0)
        throw failed_socket((char*)"Error: failure to bind server_socket");
    int res = bind(this->fd_sock, (struct sockaddr *)&this->address, sizeof(this->address));
    if (res != 0){
        std::cout << errno << std::endl;
        throw failed_socket((char*)"Error: failure to bind server_socket");
    }
}
void Socket::server_listening(int backlog){
    int res = listen(this->fd_sock, backlog);
    if (res != 0)
        throw failed_socket((char*)"Error: failure to listen from server_socket");
}
int Socket::server_accept(){
            std::cout << "halp" << std::endl;
    int buf = sizeof(this->address);
    if (buf < 0)
        throw failed_socket((char*)"Error: failure to accept in server_socket");
                std::cout << "halp" << std::endl;
    int res = accept(this->fd_sock, (struct sockaddr *)&this->address, (socklen_t*)&buf);
                std::cout << "halp" << std::endl;
    if (res == -1)
        throw failed_socket((char*)"Error: failure to accept in server_socket");
    return res;
}

/////////////////////////////////////////////
//CLIENT SOCKET
/////////////////////////////////////////////

// Socket::Socket(int domain, int service, int protocol, int port, const char *ip, bool server){
Socket::Socket(int port, const char *ip, bool server){
    this->is_server = server;
// preparation pour l'assignation de la,socket a un port(connect)
    this->address.sin_family = AF_INET; //(AF_INET?)
    this->address.sin_port = htons(port); // A CHECK BONNE CONV
// MAY NEED CHECK
    this->address.sin_addr.s_addr = inet_addr(ip);
    this->fd_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd_sock == -1)
        throw failed_socket((char*)"Error: failure to create client socket");
}
void Socket::client_connect(){
    if (sizeof(this->address) < 0)
        throw failed_socket((char*)"Error: failure to connect client_socket");
    int res = connect(this->fd_sock, (struct sockaddr *)&this->address, sizeof(this->address));
    if (res != 0)
        throw failed_socket((char*)"Error: failure to connect client_socket");
}
void Socket::client_send(const char *msg, int len){
    int res = send(this->fd_sock, msg, len, 0);
    if (res == -1)
        throw failed_socket((char*)"Error: failure to send message from client_socket");
}