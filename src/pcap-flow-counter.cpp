#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <net/ethernet.h>
#include <pcap/pcap.h>
#include "pcap.hpp"
#include "parse.hpp"
#include "pcap_frame.hpp"
#include <map>
#include <thread>
#include <utility>


typedef std::pair<std::string, std::string> flow_t;
typedef std::pair<int, int> ctr_t;
typedef std::pair<flow_t, ctr_t> fentry_t;

std::map<flow_t,ctr_t> flow_map;
std::mutex m;
bool active = true;


template <typename T>
void fun(u_char* user, const pcap_pkthdr* phdr, const u_char* ptr)
{
        T& state = *reinterpret_cast<T*>(user);

        // from now on can forget pointers, just use the state variable
        npl::pcap::frame ff(phdr,ptr);

        if (ff.has<hdr::ipv4>()) 
        {
            int nbytes = ff.get<hdr::ipv4>().len();
            auto ip_src = ff.get<hdr::ipv4>().src();
            auto ip_dst = ff.get<hdr::ipv4>().dst();
            flow_t key = std::make_pair(ip_src, ip_dst); 
            m.lock();
            ++(flow_map[key].first);
            flow_map[key].second += nbytes;
            m.unlock();
        }

}

void print_stats()
{
    std::vector<fentry_t> flows_vec;

    for (auto x : flow_map) {
        flows_vec.push_back(x);
    }

    auto compare = [](fentry_t x, fentry_t y) {
                    return ( (x.second).second > (y.second).second);
    };
    

    std::sort(flows_vec.begin(),flows_vec.end(),compare);

    for (auto i = 0; i < 5; ++i)
        std::cout << i+1 << ". " << (flows_vec[i].first).first << " --> " << (flows_vec[i].first).second << "   " << (flows_vec[i].second).first << "   " << (flows_vec[i].second).second << std::endl;

    std::cout << std::endl << std::endl;
}

void stats_clean() 
{

    std::this_thread::sleep_for(std::chrono::seconds(5));

    while(active)
    {
        print_stats();

        m.lock();
        flow_map.clear();
        m.unlock();
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}



int main(int argc, char** argv) 
{

    if (argc != 3 ) {
        std::cout << "Usage: " << argv[0] << " -i <device> | -f <filename>" << std::endl;
        exit(1);
    }

    std::thread t(stats_clean);
    t.detach();

    if (std::string(argv[1]) == "-i") {
        npl::pcap::reader<live> probe(argv[2]);
        std::cout << "Data Link Type: " << probe.datalink() << std::endl;
        probe.loop(fun<decltype(flow_map)>, flow_map, 10000);
    }


    if (std::string(argv[1]) == "-f") {
        npl::pcap::reader<offline> trace(argv[2]);
        std::cout << "Data Link: " << trace.datalink() << std::endl;
        trace.loop(fun<decltype(flow_map)>, flow_map);
    }

    active = false;





    return 0;
}