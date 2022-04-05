#include <iostream>
#include "socket.hpp"
#include "sockaddress.hpp"

int main(int argc, char** argv) {

    npl::socket<AF_UNIX, SOCK_STREAM> sock;

    std::string servername = "/tmp/test";

    npl::sockaddress<AF_UNIX> srvAddr(servername);

    sock.bind(srvAddr);

    sock.listen();
    




    return 0;
}