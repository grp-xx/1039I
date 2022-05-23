#include "frame.hpp"
#include "pcap.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    // std::string device = "any";
    std::string file;
    if (argc == 2) {
        //device = argv[1];
        file = argv[1];
    }
    else {
        exit(1);
    }
    
    //npl::pcap::reader<live> probe(device);
    npl::pcap::reader<offline> trace(file);

    for (;;) {
        //auto [ptr, hdr] = probe.next();
        auto [ptr, hdr] = trace.next();
        npl::frame ethframe(ptr,hdr.caplen);
        if (ethframe.is<icmp>()) {
            std::cout << hdr.ts.tv_sec << ":" << hdr.ts.tv_usec << " " << ethframe.header<ipv4>().src() << std::endl;
        }

    }


}