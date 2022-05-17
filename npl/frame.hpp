#ifndef _FRAME_HPP_
#define _FRAME_HPP_

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

namespace npl {

class frame {
protected:
    const struct ether_header*         m_ether = nullptr;
    const struct vlan_header *          m_vlan = nullptr;
    const struct arphdr*                m_arp  = nullptr;
    const struct ip*                    m_ipv4 = nullptr; 
    const struct icmp*                  m_icmp = nullptr; 
    const struct tcphdr*                m_tcp  = nullptr; 
    const struct udphdr*                m_udp  = nullptr; 


public:
    frame()
    {};

    frame(const u_char* raw, ssize_t caplen)
    {   
        if (caplen < sizeof(ether_header)) {
            return;
        }

        m_ether = reinterpret_cast<const ether_header*>(raw);

        if (m_ether->ether_type == htons(ETHERTYPE_VLAN))  // VLAN detected
        {
            if (caplen < sizeof(vlan_header)) {
                return;
            }
            m_vlan = reinterpret_cast<const vlan_header*>(raw);
            m_ether = nullptr;
        }

        // Parse L2 header

        if ( m_vlan != nullptr)  // Frame has vlan header 
        {
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
        }
        else       // Frame is regular ethernet header
        {
            caplen -= sizeof(ether_header); // bytes available at layer > 2

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

    }


    frame  (frame const& rhs) = default;
    frame& operator= (frame const &) = default;
    frame  (frame&& rhs) = default;
    frame& operator=(frame &&) = default;
    ~frame() = default;


    // Get pointers to header
    // Methods that return the raw pointer to C headers

    template <Proto hdr>
    auto get() const;

    // Methods that return C++ headers provided in file headers.hpp 

    template<Proto p>
    auto header() const;

    // Boolean methods to check if the frame carries the protocol p
    // Methods that check it the packet carries specific protocols..

    template <Proto p>
    bool is() const;

};


// Get pointers to header
// Methods that return the raw pointer to C headers
template<>
inline auto frame::get<ether>() const
{
    return m_ether;
}

template<>
inline auto frame::get<vlan>() const
{
    return m_vlan;
}

template<>
inline auto  frame::get<arp>() const
{
    return m_arp;
}
template<>

inline auto  frame::get<ipv4>() const
{
    return m_ipv4;
}

template<>
inline auto  frame::get<icmp>() const
{
    return m_icmp;
}

template<>
inline auto  frame::get<tcp>() const
{
    return m_tcp;
}

template<>
inline auto  frame::get<udp>() const
{
    return m_udp;
}


// Methods that extract the specified header

template<>
inline auto frame::header<ether>() const
{
    return pktheader<ether>(m_ether);
}

template<>
inline auto  frame::header<vlan>() const
{
    return pktheader<vlan>(m_vlan);
}

template<>
inline auto  frame::header<arp>() const
{
    return pktheader<arp>(m_arp);
}

template<>
inline auto  frame::header<ipv4>() const
{
    return pktheader<ipv4>(m_ipv4);
}

template<>
inline auto  frame::header<icmp>() const
{
    return pktheader<icmp>(m_icmp);
}

template<>
inline auto  frame::header<tcp>() const
{
    return pktheader<tcp>(m_tcp);
}

template<>
inline auto  frame::header<udp>() const
{
    return pktheader<udp>(m_udp);
}


// Boolean methods to check if the frame carries the protocol p
// Methods that check it the packet carries specific protocols..



template<>
inline bool  frame::is<ether>() const
{
    return (m_ether != nullptr);
}

template<>
inline bool  frame::is<vlan>() const
{
    return (m_vlan != nullptr);
}

template<>
inline bool  frame::is<arp>() const
{
    return (m_arp != nullptr);
}

template<>
inline bool  frame::is<ipv4>() const
{
    return (m_ipv4 != nullptr);
}

template<>
inline bool  frame::is<icmp>() const
{
    return (m_icmp != nullptr);
}

template<>
inline bool  frame::is<tcp>() const
{
    return (m_tcp != nullptr);
}

template<>
inline bool  frame::is<udp>() const
{
    return (m_udp != nullptr);
}





} // end of namespace npl




#endif