#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <mutex>
#include "json.hpp"
#include "socket.hpp"
#include "sockaddress.hpp"
#include "chatroom.hpp"

npl::socket<AF_INET, SOCK_DGRAM> sock;
std::mutex m;

using json = nlohmann::json;

void msg_send(const message& msg, const npl::sockaddress<AF_INET>& dst ) 
{
    json msg_j = msg;
    std::string msg_str = msg_j.dump();
    // Send welcome message to connecting user                
    sock.sendto(npl::buffer(msg_str.begin(),msg_str.end()), dst);
}

message msg_rcv()
{
    auto [buf, srv] = sock.recvfrom(360);
    auto msg_str = std::string(buf.begin(),buf.end());
    json msg_j = json::parse(msg_str);
    message msg = msg_j;
    return msg; 
}

void transmitter(const npl::sockaddress<AF_INET>& srv_addr, const std::string& user)
{

    std::cin.ignore();

    for (;;) 
    {
        m.lock();
        std::cout << "<You>: ";
        m.unlock();
        std::string line;
        std::getline(std::cin,line);
        std::stringstream ss(line);   // Convert input line into a string stream (ingnore white spaces)
        std::string first, second;
        ss >> first >> second;        // retrieve first and second words (ignore white spaces)

        if (first == "#who") 
        {
            message msg {msg_type::C, "#who", "server", user, ""};
            msg_send(msg, srv_addr);
            continue;
        }

        if ( (first == "#bye") || (first == "#leave") ) 
        {
            message msg {msg_type::C, "#bye", "server", user, ""};
            msg_send(msg,srv_addr);
            continue;
        }

        message msg {msg_type::D, "", "all", user, line};
        msg_send(msg, srv_addr);
    }

}

void receiver(const std::string& user)
{
    for(;;)
    {
        message msg = msg_rcv();
        // auto [buf, srv] = sock.recvfrom(360);
        // auto msg_str = std::string(buf.begin(),buf.end());
        // json msg_j = json::parse(msg_str);
        // message msg = msg_j;

        m.lock();
        std::cout << std::endl << "<" << msg.from << ">" << ": " << msg.text << std::endl;
        m.unlock();
        if (msg.code == "#byeOK") {
            exit(0);
        }
        m.lock();
        std::cout << "<You>: " << std::flush;
        m.unlock(); 

    }
}


int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <server> <port>" << std::endl;
        exit(1);
    }

    npl::sockaddress<AF_INET> srv_addr(argv[1], std::atoi(argv[2])); 

    std::string user;

    for (;;)
    {
        // Select user name
        std::cout << "Select user name: ";
        std::cin >> user;

         // Prepare join message
        message mjoin = {
            .type = msg_type::C, 
            .code = "#join",
            .to   = "server",
            .from =  user,
            .text = "Knock knock...",
        };       

        msg_send(mjoin, srv_addr);
        // Receive response...

        // auto [buf, srv] = sock.recvfrom(360);
        // auto response_str = std::string(buf.begin(),buf.end());
        // json response_j   = json::parse(response_str);

        // Print initial message (welcome/refuse)
        // std::cout << response_j.at("text").get<std::string>() << std::endl;
        // if ( response_j.at("code").get<std::string>() == "OK" ) 
        //    break ;

        message response = msg_rcv();
        
        std::cout << response.text << std::endl;

        if (response.code == "OK") 
            break;

    }

    std::thread rx(receiver, user);
    std::thread tx(transmitter, srv_addr, user);

    rx.join();
    tx.join();

    return 0;
}
