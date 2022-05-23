#include <iostream>
#include <net/ethernet.h>
#include <pcap/pcap.h>
#include "pcap.hpp"
#include "parse.hpp"



template <typename T>
void fun(u_char* user, const pcap_pkthdr* hdr, const u_char* ptr)
{
        T& state = *reinterpret_cast<T*>(user);

        // from now on can forget pointers, just use the state variable 


        npl::pcap::frame ff(hdr, ptr);
        if (ff.has<hdr::icmp>()) {
            state++;
        }
}


int main(int argc, char** argv) 
{

    if (argc != 3 ) {
        std::cout << "Usage: " << argv[0] << " -i <device> | -f <filename>" << std::endl;
        exit(1);
    }

    int state = 0;

    if (std::string(argv[1]) == "-i") {
        npl::pcap::reader<live> probe(argv[2]);
        std::cout << "Data Link Type: " << probe.datalink() << std::endl;
        probe.loop(fun<decltype(state)>, state);
    }


    if (std::string(argv[1]) == "-f") {
        npl::pcap::reader<offline> trace(argv[2]);
        std::cout << "Data Link: " << trace.datalink() << std::endl;
        trace.loop(fun<decltype(state)>, state);
    }

    std::cout << "Number of packets: " << state << std::endl;

    return 0;
}