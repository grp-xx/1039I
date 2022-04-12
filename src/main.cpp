#include <iostream>
#include "socket.hpp"
#include "sockaddress.hpp"
int main(int, char**) {

   npl::sockaddress<AF_INET> ip4("www.unipi.it","domain");
   std::cout << "IP: " << ip4.host() << " Port: " << ip4.port() << std::endl;

}
