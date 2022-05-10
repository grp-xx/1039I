#ifndef _PACKETHEADERS_HPP
#define _PACKETHEADERS_HPP

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string>
#include <sys/types.h>

enum Proto {ether, vlan, arp, ipv4, ipv6, icmp, tcp, udp};

namespace npl {

    template<Proto p>
    class pktheader {};

    template<>
    class pktheader<ipv4> {
        struct ip c_header = {0};  // c_ beacause using standard C header
    
    public:
        pktheader();

        pktheader(const u_char* ptr, ssize_t size)
        {
            c_header = {0};
            if (size >= sizeof(struct ip)) 
            {
                c_header = *reinterpret_cast<const struct ip*>(ptr);
            }
        }

        unsigned short
        version() const
        {
            return c_header.ip_v;
        }

        unsigned short
        hlen() const
        {
            return c_header.ip_hl;
        }

        unsigned short
        protocol() const
        {
            return c_header.ip_p;
        }

        std::string
        src() const
        {
            char addr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, reinterpret_cast<const void*>(&c_header.ip_src.s_addr), addr, sizeof(addr));
            return addr;
        }

        std::string
        dst() const
        {
            char addr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, reinterpret_cast<const void*>(&c_header.ip_dst.s_addr), addr, sizeof(addr));
            return addr;
        }
    };

    template<>
    class pktheader<tcp> 
    {

    };


}

#endif
