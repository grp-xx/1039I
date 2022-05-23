#include "parse.hpp"
#include "pcap.hpp"
#include <iostream>
#include <pcap/pcap.h>
#include <string>
#include <sys/_types/_u_char.h>

template<typename T>
void fun(u_char* user, const pcap_pkthdr* phdr, const u_char* ptr)
{
    T& state = *reinterpret_cast<T*>(user);


    npl::pcap::frame ff(phdr,ptr);

        if (ff.has<hdr::icmp>()) {
            ++state;
            std::cout << "From: " << ff.get<hdr::ipv4>().src() << std::endl;
        }
     


}

int main(int argc, char** argv)
{
    // std::string device = "any";
    // pcap_sniff -i iface | -f file 
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " -i <device> | -f <filename>" << std::endl;
        exit(1);
    }

    int state = 0;

    if (std::string(argv[1]) == "-i") {
        npl::pcap::reader<live> sock(argv[2]);
        sock.loop(fun<int>, state, 1000);

    } 

    else {
        npl::pcap::reader<offline> sock(argv[2]);
        sock.loop(fun<int>, state);
    }

    std::cout << "Numero di pacchetti ICMP: " <<state << std::endl;



}