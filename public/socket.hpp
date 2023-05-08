//socket.hpp
#pragma once

#include <netinet/in.h>
namespace chatroom {
namespace net {

class SocketStream{
protected:
  sockaddr_in addr; //ipv4 addr
  int sock_fd;  

public:
    int close();    //关闭socket
    SocketStream(const char *addr, in_port_t port);//构造函数
    int get_sock_fd();
};

class SocketStreamClient : public SocketStream {
public:
    int connect();
    SocketStreamClient(const char *addr, in_port_t port);
};

class SocketStreamHost : public SocketStream {
public:
    int host();
    int accept();
    SocketStreamHost(const char *addr, in_port_t port);
};

}
}