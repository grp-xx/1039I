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


enum class raw   {ether, vlan, arp, ipv4, ipv6, icmp, tcp, udp}; 
enum class proto {unspec, ether, vlan, arp, ipv4, ipv6, icmp, tcp, udp}; 

namespace npl {

class frame{

protected:

    const struct ether_header*          m_ether = nullptr;
    const struct vlan_header*           m_vlan = nullptr;
    const struct arphdr*                m_arp  = nullptr;
    const struct ip*                    m_ipv4 = nullptr; 
    const struct icmp*                  m_icmp = nullptr; 
    const struct tcphdr*                m_tcp  = nullptr; 
    const struct udphdr*                m_udp  =  nullptr; 
    // const u_char*                    m_app  = nullptr; 


public: 
    frame(){};
    frame (const u_char* raw, ssize_t caplen)
    {

        // Assume hardware is ethernet
        // Select Ethernet/Vlan

        if ( caplen < sizeof(ether_header) ) {
            return;
        }
        
        
        m_ether = reinterpret_cast<const ether_header*>(raw);

        if (m_ether->ether_type == htons(ETHERTYPE_VLAN))  // VLAN header detected
        {
            if (caplen <= sizeof(vlan_header)) {
                return;
            } 
            m_vlan = reinterpret_cast<const vlan_header*>(raw);
        }

        // Parse L2 header (find out which is the layer 3)


        if ( m_vlan != nullptr )      // Frame has vlan header
        {
            
            caplen -= sizeof(vlan_header);
            
            if ( m_vlan->type == htons(ETHERTYPE_ARP) ) 
            {
                if (caplen >= sizeof(arphdr))
                {
                    m_arp = reinterpret_cast<const arphdr*>(m_vlan+1);
                } 
            }

            if ( m_vlan->type == htons(ETHERTYPE_IP) )   
            {
                if (caplen >= sizeof(ip))
                {
                    m_ipv4 = reinterpret_cast<const ip*>(m_vlan+1);
                }
            }         

        }
        else    // Regular ethernet header
        {
            caplen -= sizeof(ether_header);
            if ( m_ether->ether_type == htons(ETHERTYPE_ARP) ) 
            {
                if (caplen >= sizeof(arphdr))
                {
                    m_arp = reinterpret_cast<const arphdr*>(m_ether+1);
                } 
            }

            if ( m_ether->ether_type == htons(ETHERTYPE_IP) )   
            {
                if (caplen >= sizeof(ip))
                {
                    m_ipv4 = reinterpret_cast<const ip*>(m_ether+1);
                }
            }         
        }


        // Parse L3 header
        // Basically... only IPv4

        if (m_ipv4 != nullptr) 
        {
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

        // Parse L4 header
        // dhcp, dns, ...
        }
        
    }  // End Ctor


    frame  (frame const& rhs) = default;
    frame& operator= (frame const &) = default;
    frame  (frame&& rhs) = default;
    frame& operator=(frame &&) = default;
    ~frame() = default;


    // Methods that return the raw pointer to portions of packets 

    template<raw p>
    const u_char* get() const
    {
        if constexpr ( p == raw::ether) return reinterpret_cast<const u_char*>(m_ether);
        if constexpr ( p == raw::vlan ) return reinterpret_cast<const u_char*>(m_vlan);
        if constexpr ( p == raw::arp  ) return reinterpret_cast<const u_char*>(m_arp);
        if constexpr ( p == raw::ipv4 ) return reinterpret_cast<const u_char*>(m_ipv4);
        if constexpr ( p == raw::icmp ) return reinterpret_cast<const u_char*>(m_icmp);
        if constexpr ( p == raw::udp  ) return reinterpret_cast<const u_char*>(m_udp);
        if constexpr ( p == raw::tcp  ) return reinterpret_cast<const u_char*>(m_tcp);
    }
    
    // Methods that return C++ headers provided in file headers.hpp 

    template<hdr h>
    auto get() const {
        if constexpr (h == hdr::ether) return header<hdr::ether>(m_ether );  // Controlla da qui!
        if constexpr (h == hdr::vlan)  return header<hdr::vlan>(m_vlan );
        if constexpr (h == hdr::arp)   return header<hdr::arp>(m_arp );
        if constexpr (h == hdr::ipv4)  return header<hdr::ipv4>(m_ipv4 );
        if constexpr (h == hdr::icmp)  return header<hdr::icmp>(m_icmp );
        if constexpr (h == hdr::udp)   return header<hdr::udp>(m_udp );
        if constexpr (h == hdr::tcp)   return header<hdr::tcp>(m_tcp );
    }

    // Boolean methods to check if the frame carries the protocol p
    // Methods that check it the packet carries specific protocols..

    template <hdr h>
    bool has() const
    {
        if constexpr (h == hdr::ether)  return (m_ether != nullptr);
        if constexpr (h == hdr::vlan)   return (m_vlan != nullptr);
        if constexpr (h == hdr::arp)    return (m_arp != nullptr);
        if constexpr (h == hdr::ipv4)   return (m_ipv4 != nullptr);
        if constexpr (h == hdr::icmp)   return (m_icmp != nullptr);
        if constexpr (h == hdr::udp)    return (m_udp != nullptr);
        if constexpr (h == hdr::tcp)    return (m_tcp != nullptr);
    }

};


} // end namespace npl






#endif