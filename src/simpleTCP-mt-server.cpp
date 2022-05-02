#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>
#include <thread>
#include "socket.hpp"
#include "sockaddress.hpp"


void echoreply(npl::socket<AF_INET, SOCK_STREAM>&& client_sock)
{  
    for (;;) 
    {
        auto buf = client_sock.read(2048);
        if (buf.empty())  
           break;
        client_sock.write(buf);
    }
    std::cout << "Client disconnected" << std::endl; 
    client_sock.close();
}

int main(int argc, char** argv) {

    npl::socket<AF_INET, SOCK_STREAM> sock;
    npl::sockaddress<AF_INET> srvAddr("",10000);
    sock.bind(srvAddr);
    sock.listen();

    // std::cout << "Press any key to start" << std::endl;
    // std::getchar();

    for (;;) 
    {
        auto [conn_sock, peer] = sock.accept();
        std::cout << "Connected to remote host: " << peer.host() << std::endl;
        std::thread t(echoreply,std::move(conn_sock));
        t.detach();
    }

    return 0;
}