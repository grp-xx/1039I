#ifndef _PACKET_HPP_
#define _PACKET_HPP_

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/types.h>

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <sstream>
#include "headers.hpp"
#include "frame.hpp"


namespace npl {


template<Proto p = unspec>
class packet {};

template<>
class packet<unspec> : public frame {

public:
    packet()
    : frame()
    {}

    packet(const u_char* raw, ssize_t caplen)
    : frame(raw, caplen)
    {}

    packet  (packet const& rhs) = default;
    packet& operator= (packet const &) = default;
    packet  (packet&& rhs) = default;
    packet& operator=(packet &&) = default;
    ~packet() = default;   

};




template<>
class packet<ether> : public frame {

public:
    packet()
    : frame()
    {}

    packet(const u_char* raw, ssize_t caplen)
    : frame()
    {
        if (caplen < sizeof(ether_header)) {
            return;
        }

        m_ether = reinterpret_cast<const ether_header*>(raw);

        if ( m_ether->ether_type == htons(ETHERTYPE_ARP)) 
        {
            if ( caplen >= sizeof(arphdr) )
            {
                m_arp = reinterpret_cast<const arphdr*>(m_ether+1);
            }

        }
        if ( m_ether->ether_type == htons(ETHERTYPE_IP)) 
        {
            if ( caplen >= sizeof(ip) )
            {
                m_ipv4 = reinterpret_cast<const ip*>(m_ether+1);
            }
            
        }

        // Parse L3 headers

    if (m_ipv4 != nullptr)     // Frame carries an IP Packet
    {
        ssize_t ip_hl = (m_ipv4->ip_hl)*4;
        caplen -= ip_hl;
        const u_char* ipv4_ptr = reinterpret_cast<const u_char*>(m_ipv4);

        if (m_ipv4->ip_p == IPPROTO_ICMP)
        {
            if (caplen >= sizeof(struct icmp))
            {
                m_icmp = reinterpret_cast<const struct icmp*>(ipv4_ptr + ip_hl);
            }

        }

        if (m_ipv4->ip_p == IPPROTO_UDP)
        {
            if (caplen >= sizeof(struct udphdr))
            {
                m_udp = reinterpret_cast<const struct udphdr*>(ipv4_ptr + ip_hl);
            }
        }

        if (m_ipv4->ip_p == IPPROTO_TCP)
        {
            if (caplen >= sizeof(struct tcphdr))
            {
                m_tcp = reinterpret_cast<const struct tcphdr*>(ipv4_ptr + ip_hl);
            }
        }

     }

        // Parse L4 Header looking for DNS, DHCP, HTTP(S)





    }  // End Ctor

    packet  (packet const& rhs) = default;
    packet& operator= (packet const &) = default;
    packet  (packet&& rhs) = default;
    packet& operator=(packet &&) = default;
    ~packet() = default;   

};


template<>
class packet<vlan> : public frame {

public:
    packet()
    : frame()
    {}

    packet(const u_char* raw, ssize_t caplen)
    : frame()
    {
        if (caplen < sizeof(vlan_header)) {
            return;
        } 
        m_vlan = reinterpret_cast<const vlan_header*>(raw);        
        caplen -= sizeof(vlan_header); // bytes available at layer > 2

        if ( m_vlan->type == htons(ETHERTYPE_ARP)) 
        {
            if ( caplen >= sizeof(arphdr) )
            {
                m_arp = reinterpret_cast<const arphdr*>(m_vlan+1);
            }

        }
        if ( m_vlan->type == htons(ETHERTYPE_IP)) 
        {
            if ( caplen >= sizeof(ip) )
            {
                m_ipv4 = reinterpret_cast<const ip*>(m_vlan+1);
            }
            
        }

        // Parse L3 headers

        if (m_ipv4 != nullptr)     // Frame carries an IP Packet
        {
            ssize_t ip_hl = (m_ipv4->ip_hl)*4;
            caplen -= ip_hl;
            const u_char* ipv4_ptr = reinterpret_cast<const u_char*>(m_ipv4);

            if (m_ipv4->ip_p == IPPROTO_ICMP)
            {
                if (caplen >= sizeof(struct icmp))
                {
                    m_icmp = reinterpret_cast<const struct icmp*>(ipv4_ptr + ip_hl);
                }

            }

            if (m_ipv4->ip_p == IPPROTO_UDP)
            {
                if (caplen >= sizeof(struct udphdr))
                {
                    m_udp = reinterpret_cast<const struct udphdr*>(ipv4_ptr + ip_hl);
                }
            }

            if (m_ipv4->ip_p == IPPROTO_TCP)
            {
                if (caplen >= sizeof(struct tcphdr))
                {
                    m_tcp = reinterpret_cast<const struct tcphdr*>(ipv4_ptr + ip_hl);
                }
            }

        }

        // Parse L4 Header looking for DNS, DHCP, HTTP(S)


    } // End Ctor

    packet  (packet const& rhs) = default;
    packet& operator= (packet const &) = default;
    packet  (packet&& rhs) = default;
    packet& operator=(packet &&) = default;
    ~packet() = default;   

};


template<>
class packet<ipv4> : public frame {

public:
    packet()
    : frame()
    {}

    packet(const u_char* ptr, ssize_t caplen)  // ptr is a pointer to ipv4
    : frame(ptr, caplen)
    {
        m_ipv4 = reinterpret_cast<const ip*>(ptr);
        ssize_t ip_hl = (m_ipv4->ip_hl)*4;
        const u_char* ip4_ptr = reinterpret_cast<const u_char*>(m_ipv4);
        caplen -= ip_hl;

        if (m_ipv4->ip_p == IPPROTO_ICMP) 
        {
            if (caplen >= sizeof(icmp))
            {
                m_icmp = reinterpret_cast<const struct icmp*>(ip4_ptr+ip_hl);
            }
        }

        if (m_ipv4->ip_p == IPPROTO_UDP) 
        {
            if (caplen >= sizeof(udphdr))
            {
                m_udp = reinterpret_cast<const udphdr*>(ip4_ptr+ip_hl);
            }

        }

        if (m_ipv4->ip_p == IPPROTO_TCP) 
        {
            if (caplen >= sizeof(udphdr))
            {
                m_tcp = reinterpret_cast<const tcphdr*>(ip4_ptr+ip_hl);
            }
        }
    }

    packet  (packet const& rhs) = default;
    packet& operator= (packet const &) = default;
    packet  (packet&& rhs) = default;
    packet& operator=(packet &&) = default;
    ~packet() = default;   

};










} // End namespace npl

#endif