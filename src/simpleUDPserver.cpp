#include <cstdio>
#include <iostream>
#include "socket.hpp"
#include "sockaddress.hpp"

int main(int argc, char** argv) {

    // SOCK_STREAM
    
//    npl::socket<AF_UNIX, SOCK_STREAM> sock;
//
//    std::string servername = "/tmp/test";
//
//    npl::sockaddress<AF_UNIX> srvAddr(servername);
//
//    sock.bind(srvAddr);
//
//    sock.listen();
//
//    auto [connected, peer] = sock.accept();
//
//    // Read a line of text
//    npl::buffer buf = connected.read(80);
//
//    // Send back the line to the client
//    connected.write(buf);
//
//    sock.close();


    // SOCK_DGRAM

//    npl::socket<AF_UNIX, SOCK_DGRAM> sock;
//    std::string servername = "/tmp/test";
//    npl::sockaddress<AF_UNIX> srvAddr(servername);

    npl::socket<AF_INET, SOCK_DGRAM> sock;
    npl::sockaddress<AF_INET> srvAddr("",10000);
    sock.bind(srvAddr);

    // std::cout << "Press any key to start" << std::endl;
    // std::getchar();

    for (;;) 
    {
        auto [buf, remote] = sock.recvfrom(2048);
        std::cout << "Received: " << std::string(buf.begin(),buf.end()) << std::endl;
        std::cout << "from: " << remote.host() << std::endl;
        sock.sendto(buf, remote);
    } 



    sock.close();



    return 0;
}