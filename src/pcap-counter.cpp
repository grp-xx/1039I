#include <algorithm>
#include <cstdio>
#include <iostream>
#include <net/ethernet.h>
#include <pcap/pcap.h>
#include "pcap.hpp"
#include "parse.hpp"
#include "pcap_frame.hpp"
#include <map>

template <typename T>
void fun(u_char* user, const pcap_pkthdr* phdr, const u_char* ptr)
{
        T& state = *reinterpret_cast<T*>(user);

        // from now on can forget pointers, just use the state variable
        npl::pcap::frame ff(phdr,ptr);

        if (ff.has<hdr::ipv4>()) 
        {
            ++state[ff.get<hdr::ipv4>().src()];
        }

}



int main(int argc, char** argv) 
{

    if (argc != 3 ) {
        std::cout << "Usage: " << argv[0] << " -i <device> | -f <filename>" << std::endl;
        exit(1);
    }

    std::map<std::string,int> flow_map;

    if (std::string(argv[1]) == "-i") {
        npl::pcap::reader<live> probe(argv[2]);
        std::cout << "Data Link Type: " << probe.datalink() << std::endl;
        probe.loop(fun<decltype(flow_map)>, flow_map);
    }


    if (std::string(argv[1]) == "-f") {
        npl::pcap::reader<offline> trace(argv[2]);
        std::cout << "Data Link: " << trace.datalink() << std::endl;
        trace.loop(fun<decltype(flow_map)>, flow_map);
    }

    std::vector<std::pair<std::string,int> > flows_vec;

    for (auto x : flow_map) {
        flows_vec.push_back(x);
    }

    auto compare = [](std::pair<std::string, int> x, std::pair<std::string, int> y) {
                    return (x.second < y.second);
    };
    

    std::sort(flows_vec.begin(),flows_vec.end(),compare);


    for (auto x : flows_vec) {
        std::cout << x.first << " " << x.second << std::endl;
    }



    return 0;
}