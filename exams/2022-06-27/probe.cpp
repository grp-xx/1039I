#include <iostream>
#include <string>
#include <sys/socket.h>
#include "pcap.hpp"
#include "parse.hpp"
#include "sockaddress.hpp"
#include "socket.hpp"
#include "json.hpp"

using json = nlohmann::json;

struct collector_info {
    npl::socket<AF_INET, SOCK_DGRAM> sock;
    npl::sockaddress<AF_INET> addr;
};


struct report {
    std::string src_ip;
    std::string dst_ip;
    unsigned short src_prt;
    unsigned short dst_prt;
    unsigned int length;
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




// Callback function 
template <typename T>
void fun(u_char* user, const pcap_pkthdr* phdr, const u_char* ptr)
{
        T& collector = *reinterpret_cast<T*>(user);

        // from now on can forget pointers, just use the state variable
        npl::pcap::frame ff(phdr,ptr);

        if (ff.has<hdr::tcp>()) 
        {
            auto iphdr = ff.get<hdr::ipv4>();
            auto tcphdr = ff.get<hdr::tcp>();
            report rr;
            rr.length = iphdr.len() - iphdr.hlen() - tcphdr.hlen();
            rr.src_ip = ff.get<hdr::ipv4>().src();
            rr.dst_ip = ff.get<hdr::ipv4>().dst();
            rr.src_prt = ff.get<hdr::tcp>().srcport();
            rr.dst_prt = ff.get<hdr::tcp>().dstport();
        
            // Create JSON object report

            json j_rr = rr;

            // Serialize report and send to collector

            // std::cout << "Packet: " << j_rr.dump() << std::endl;
            std::string s_rr = j_rr.dump();

            collector.sock.sendto(npl::buffer(s_rr.begin(), s_rr.end()), collector.addr);
        }
}



int main(int argc, char** argv) 
{

    if ( argc != 4) {
        std::cout << "Usage: " << argv[0] << " [ -i <device> | -f <filename> ] <collector> " << std::endl;
        exit(1);
    }

    collector_info collector;
    collector.addr = npl::sockaddress<AF_INET>(argv[3],12000);

    if (std::string(argv[1]) == "-i") {
        npl::pcap::reader<live> probe(argv[2]);
        // std::cout << "Data Link Type: " << probe.datalink() << std::endl;
        probe.loop(fun< collector_info >, collector);
    }

    if (std::string(argv[1]) == "-f") {
        npl::pcap::reader<offline> trace(argv[2]);
        // std::cout << "Data Link: " << trace.datalink() << std::endl;
        trace.loop(fun< collector_info >, collector);
    }





    exit(0);
}

