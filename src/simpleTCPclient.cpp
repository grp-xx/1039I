#include <iostream>
#include "socket.hpp"
#include "sockaddress.hpp"

int main(int argc, char** argv) {

//    npl::socket<AF_UNIX, SOCK_DGRAM> sock;
//    std::string servername = "/tmp/test";
//    npl::sockaddress<AF_UNIX> srvAddr(servername);
//    npl::sockaddress<AF_UNIX> cliaddr(servername + "." + std::to_string(getpid()));
//    sock.bind(cliaddr);


    npl::socket<AF_INET, SOCK_STREAM> sock;
    npl::sockaddress<AF_INET> srvAddr("localhost",10000);
    
    sock.connect(srvAddr);

    


    for (;;)
    {
        std::string line;
        std::cout << "Ping: ";
        std::getline(std::cin, line);
        sock.write(npl::buffer(line.begin(),line.end()));
        auto [buf, remote] = sock.recvfrom(80);
        std::cout << "Pong: " << std::string(buf.begin(),buf.end()) << std::endl;
    }

    sock.close();








    return 0;
}