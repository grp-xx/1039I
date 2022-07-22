#include <iostream>
#include <string>
#include "2022-07-18.hpp"
#include "sockaddress.hpp"
#include "socket.hpp"


int main(int argc, char** argv) {

    std::string srv_ip = "";
    if (argc == 2) {
        srv_ip = argv[1];
    }

    int srv_port       = 12000;
    auto srv_addr= npl::sockaddress<AF_INET>(srv_ip,srv_port); 
    npl::socket<AF_INET, SOCK_DGRAM> sock;
    sock.bind(srv_addr);

    // Receive UDP fortune requests
    for (;;) 
    {
        // Receive message
        auto [buf, client] = sock.recvfrom(360);
        // Parse json representation
        json msg_j = json::parse( std::string(buf.begin(),buf.end()) );
        // create C++ object
        message m = msg_j;

        if (m.type == "REQ") {
            message r = {.type = "REP", .text = get_fortune()};
            json r_j = r;
            std::string r_str = r_j.dump();
            sock.sendto(npl::buffer(r_str.begin(), r_str.end()), client);
        }

    }

    return 0;
}