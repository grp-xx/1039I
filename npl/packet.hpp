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

template<proto p> 
class packet : public frame {

public:
/////////
    packet()
    : frame()
    {};

    packet(const u_char* raw, ssize_t caplen)  requires (p == proto::unspec)
    : frame(raw, caplen)
    {};
    
    packet(const u_char* raw, ssize_t caplen)  requires (p == proto::ether)
    : frame(raw, caplen)
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

    packet(const u_char* raw, ssize_t caplen) requires (p == proto::vlan)
    : frame(raw, caplen)
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
    
    packet(const u_char* raw, ssize_t caplen)  requires (p == proto::ipv4) // raw is a pointer to ipv4
    : frame(raw, caplen)
    {
        m_ipv4 = reinterpret_cast<const ip*>(raw);
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


    // Various Methods
    
    // Methods that return the raw pointer to portions of packets 

    template <raw pp>
    const u_char* get() const
    {
        if constexpr ( pp == raw::ether) return reinterpret_cast<const u_char*>(m_ether);
        if constexpr ( pp == raw::vlan ) return reinterpret_cast<const u_char*>(m_vlan);
        if constexpr ( pp == raw::arp  ) return reinterpret_cast<const u_char*>(m_arp);
        if constexpr ( pp == raw::ipv4 ) return reinterpret_cast<const u_char*>(m_ipv4);
        if constexpr ( pp == raw::icmp ) return reinterpret_cast<const u_char*>(m_icmp);
        if constexpr ( pp == raw::udp  ) return reinterpret_cast<const u_char*>(m_udp);
        if constexpr ( pp == raw::tcp  ) return reinterpret_cast<const u_char*>(m_tcp);
    }
    
    // Methods that return C++ headers provided in file headers.hpp 

    template<hdr h>
    auto get() const {
        if constexpr (h == hdr::ether) return header<hdr::ether>(m_ether ); 
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


} // End namespace npl

#endif