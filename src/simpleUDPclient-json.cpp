#include <iostream>
#include "socket.hpp"
#include "sockaddress.hpp"
#include "json.hpp"

using json = nlohmann::json;

struct message {
    std::string sender;
    int id;
    std::string text;
};

void to_json(json& j, const message& m) {
    j = json{{"sender", m.sender}, {"id", m.id}, {"text", m.text}};
}

void from_json(const json& j, message& m) {
    j.at("sender").get_to(m.sender);
    j.at("id").get_to(m.id);
    j.at("text").get_to(m.text);
}

int main(int argc, char** argv) {

//    npl::socket<AF_UNIX, SOCK_DGRAM> sock;
//    std::string servername = "/tmp/test";
//    npl::sockaddress<AF_UNIX> srvAddr(servername);
//    npl::sockaddress<AF_UNIX> cliaddr(servername + "." + std::to_string(getpid()));
//    sock.bind(cliaddr);


    npl::socket<AF_INET, SOCK_DGRAM> sock;
    npl::sockaddress<AF_INET> srvAddr("localhost",10000);
    std::string line;

    for (;;)
    {
        std::cout << "Ping: ";
        std::getline(std::cin, line);

        message msg 
        {
            .sender = "Gregorio",
            .id = 42,
            .text = line,
        };

        json msg_j = msg;

        std::string aaa = msg_j.dump();
        
        sock.sendto(npl::buffer(aaa.begin(),aaa.end()), srvAddr);
        auto [buf, remote] = sock.recvfrom(80);
        auto rx = std::string(buf.begin(),buf.end());

        json rx_j = json::parse(rx);
        message mrx = rx_j;

        std::cout << "Sender: " << rx_j["sender"].get<std::string>() << " id: " << rx_j.at("id").get<int>() << " text: " << mrx.text << std::endl; 

        // std::cout << "Pong: " << std::string(buf.begin(),buf.end()) << std::endl;
    }

    sock.close();








    return 0;
}