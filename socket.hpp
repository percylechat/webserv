#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/socket.h> //socket
#include <netinet/in.h> //struct sockaddr_in
#include <arpa/inet.h> // inet_addr
#include <iostream>

#include <errno.h>  //debug, get errno value

class Socket{
    protected:
        struct sockaddr_in address;
// struct sockaddr_in {
//  short   sin_family;       /* la famille de protocole */
//  u_short sin_port;         /* numéro de port */
//  struct  in_addr sin_addr; /* adresse IP de la machine */
//  char    sin_zero[8];      /* remplissage pour faire 16 octets */
// };
        int fd_sock;
        int connexion;
        bool is_server;
    public:
/////////////////////////////////////////////
//SERVER SOCKET
/////////////////////////////////////////////
        // Socket(int domain, int service, int protocol, int port, u_long interface, bool server);
        Socket(bool server);
// server funct: first the socket is binded to a specifi address, then it is set to listen.
// when a connexion occurs, server_accept creates a new socket modelling the original and handles it.
        char *create_socket(int port, in_addr_t interface);
        char *server_binding();
        char *server_listening(int backlog);
        int server_accept();
/////////////////////////////////////////////
//CLIENT SOCKET
/////////////////////////////////////////////
        // Socket(int domain, int service, int protocol, int port, const char *ip, bool server);
        Socket(int port, const char *ip, bool server);
        void client_connect();
        void client_send(const char *msg, int len);
/////////////////////////////////////////////
//GLOBAL
/////////////////////////////////////////////
    int get_fd();
        class failed_socket{
            private:
                char *msg;
            public:
                failed_socket(char *msg);
                // failed_socket(char * err);
                const char * what() const throw();
                // const char * what(void) const throw();
        };
    

};

#endif // !SOCKET_HPP