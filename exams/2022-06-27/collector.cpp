#include <iostream>
#include <thread>
#include <tuple>
#include "socket.hpp"
#include "sockaddress.hpp"
#include "json.hpp"

using json = nlohmann::json;


typedef std::tuple<std::string, std::string, unsigned int, unsigned int> flow_t;
typedef std::pair<flow_t, int> entry_t;

std::map<flow_t,int> flow_map;
std::mutex m;



struct report {
    std::string src_ip;
    std::string dst_ip;
    unsigned short src_prt;
    unsigned short dst_prt;
    unsigned int   length;
};

void inline to_json(json& j, const report& r)
{
    j = json {{"src_ip", r.src_ip}, {"dst_ip", r.dst_ip}, {"src_prt", r.src_prt}, {"dst_prt", r.dst_prt}, {"length", r.length} };
}

void inline from_json(const json& j, report& r)
{
    j.at("src_ip").get_to(r.src_ip);
    j.at("dst_ip").get_to(r.dst_ip);
    j.at("src_prt").get_to(r.src_prt);
    j.at("dst_prt").get_to(r.dst_prt);
    j.at("length").get_to(r.length);
}


void print_stats()
{
    std::vector<entry_t> flows_vec;

    if ( flow_map.empty() )
        return;

    for (auto x : flow_map) {
        flows_vec.push_back(x);
    }

    auto compare = [](entry_t x, entry_t y) {
                    return ( x.second > y.second);
    };
    

    std::sort(flows_vec.begin(),flows_vec.end(),compare);

    for (auto i = 0; i < 5; ++i)
        std::cout << i+1 << ". " << "(" << std::get<0>(flows_vec[i].first) << " --> " << std::get<1>(flows_vec[i].first) << ")"
                                 << "(" << std::get<2>(flows_vec[i].first) << " --> " << std::get<3>(flows_vec[i].first) << ")"
                                 << "   Length: " << (flows_vec[i].second) << " bytes" << std::endl;

    std::cout << std::endl << std::endl;
}


void stats_and_clean() 
{
    std::this_thread::sleep_for(std::chrono::seconds(10));
    for(;;)
    {
        print_stats();

        m.lock();
        flow_map.clear();
        m.unlock();
    
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}


int main(int argc, char** argv) {

    std::string srv_ip = "";
    if (argc == 2) {
        srv_ip = argv[1];
    }

    int srv_port       = 12000;
    auto srv_addr= npl::sockaddress<AF_INET>(srv_ip,srv_port); 

    npl::socket<AF_INET, SOCK_DGRAM> sock;
    sock.bind(srv_addr);

    std::thread t(stats_and_clean);
    t.detach();

    // Receive UDP message reports
    for (;;) 
    {
        // Receive message
        auto [buf, client] = sock.recvfrom(360);
        // Parse json representation
        json msg_j = json::parse( std::string(buf.begin(),buf.end()) );

        // std::cout << "Received report: " << msg_j.dump() << std::endl;

        // create C++ object
        report r = msg_j;

        // Populate the map;
        flow_t key = std::make_tuple(r.src_ip, r.dst_ip, r.src_prt, r.dst_prt); 
        m.lock();
        flow_map[key] += r.length;
        m.unlock();
    }

    exit(0);
}