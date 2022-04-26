#include <cstdio>
#include <cstdlib>
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

    npl::socket<AF_INET, SOCK_STREAM> sock;
    npl::sockaddress<AF_INET> srvAddr("",10000);
    sock.bind(srvAddr);
    sock.listen();

    // std::cout << "Press any key to start" << std::endl;
    // std::getchar();

    for (;;) 
    {
        auto [conn_sock, peer] = sock.accept();

        auto pid = fork();

        if (pid == 0) // child process
        {
            std::cout << "Connected to remote host: " << peer.host() << std::endl;
            for (;;) 
            {
                auto buf = conn_sock.read(2048);
                if (buf.empty()) 
                    break;
                // std::cout << "Received: " << std::string(buf.begin(),buf.end()) << std::endl;
                conn_sock.write(buf);
            }
            std::cout << "Client disconnected" << std::endl; 
            exit(0);
        }
        conn_sock.close();
        
    }

    return 0;
}