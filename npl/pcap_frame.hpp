#ifndef _PCAP_FRAME_HPP_
#define _PCAP_FRAME_HPP_
#include <cstddef>
#include <functional>
#include <pcap/pcap.h>
#include <sys/types.h>
#include <system_error>
#include <utility>
#include "headers.hpp"
#include "frame.hpp"


namespace npl::pcap {

class frame : public npl::frame {
protected:
    const struct pcap_pkthdr* m_pcap = nullptr;

public:
    frame (const pcap_pkthdr* hdr, const u_char* raw)
    : npl::frame(raw, hdr->caplen)
    , m_pcap(hdr)
    {}


    frame  (frame const& rhs) = default;
    frame& operator= (frame const &) = default;
    frame  (frame&& rhs) = default;
    frame& operator=(frame &&) = default;
    ~frame() = default;


    // Methods that return the pcap header

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

    template<hdr h>
    auto get() const {
        if constexpr ( h == hdr::ether )  return header<h>( m_ether );  
        if constexpr ( h == hdr::vlan  )  return header<h>( m_vlan  );
        if constexpr ( h == hdr::arp   )  return header<h>( m_arp   );
        if constexpr ( h == hdr::ipv4  )  return header<h>( m_ipv4  );
        if constexpr ( h == hdr::icmp  )  return header<h>( m_icmp  );
        if constexpr ( h == hdr::udp   )  return header<h>( m_udp   );
        if constexpr ( h == hdr::tcp   )  return header<h>( m_tcp   );
        if constexpr ( h == hdr::pcap  )  return header<h>( m_pcap  );  
    }

    // Boolean methods to check if the frame carries the protocol p
    // Methods that check it the packet carries specific protocols..

    template <hdr h>
    bool has() const
    {
        if constexpr (h == hdr::ether )   return ( m_ether != nullptr );
        if constexpr (h == hdr::vlan  )   return ( m_vlan != nullptr  );
        if constexpr (h == hdr::arp   )   return ( m_arp != nullptr   );
        if constexpr (h == hdr::ipv4  )   return ( m_ipv4 != nullptr  );
        if constexpr (h == hdr::icmp  )   return ( m_icmp != nullptr  );
        if constexpr (h == hdr::udp   )   return ( m_udp != nullptr   );
        if constexpr (h == hdr::tcp   )   return ( m_tcp != nullptr   );
        if constexpr (h == hdr::pcap  )   return ( m_pcap != nullptr  );
    }

};


} // end of namespace npl::pcap



#endif