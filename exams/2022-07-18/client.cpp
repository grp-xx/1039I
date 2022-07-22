#include "sockaddress.hpp"
#include "socket.hpp"
#include "2022-07-18.hpp"

int main(int argc, char** argv) 
{

    if ( argc != 3) {
        std::cout << "Usage: " << argv[0] << " <server> <port> " << std::endl;
        exit(1);
    }

    npl::sockaddress<AF_INET> srv_addr(argv[1], atoi(argv[2]));
    npl::socket<AF_INET, SOCK_DGRAM> sock;

    message fortune_req = {.type = "REQ", .text = ""};
    json f_j = fortune_req;
    std::string f_str = f_j.dump();

    sock.sendto(npl::buffer(f_str.begin(), f_str.end()), srv_addr);
    // Receive message
    auto [buf, server] = sock.recvfrom(1450);
    // Parse json representation
    json msg_j = json::parse( std::string(buf.begin(),buf.end()) );
    // create C++ object
    message m = msg_j;
    if (m.type == "REP") {
        std::cout << m.text << std::endl;
    }

    return 0;
}
