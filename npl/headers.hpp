#ifndef _HEADERS_HPP_
#define _HEADERS_HPP_
#include <cstddef>
#include <iostream>
#include <net/if_arp.h>
#include <sys/types.h>
#include <vector>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <pcap/pcap.h>
#include <sstream>



struct vlan_header {
    u_char    vlan_dhost[ETHER_ADDR_LEN];
    u_char    vlan_shost[ETHER_ADDR_LEN];
    u_int16_t vlan_tpid;
    u_int16_t vlan_id;
    u_int16_t type;
};


//enum Layer {phy = 1, datalink = 2, network = 3, transport = 4, application = 5};
enum class hdr {pcap, ether, vlan, arp, ipv4, ipv6, icmp, tcp, udp};



namespace npl {

template <hdr value>
class header {};


// Specialize for pcap theader

template<>
class header<hdr::pcap> {
private:
    struct pcap_pkthdr chdr = {0};

public:
    header(){};
    explicit header(const pcap_pkthdr* ptr) 
    {
        if (ptr != nullptr)
            chdr = *ptr;
    }

    header(const u_char* ptr, ssize_t size) 
    {
        if ( ( ptr == nullptr ) || ( size < sizeof(pcap_pkthdr) ) ) {
            throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
        }
        chdr = *reinterpret_cast<const pcap_pkthdr*>(ptr);
    }

    header  (header const& rhs) = default;
    header& operator= (header const &) = default;
    header  (header&& rhs) = default;
    header& operator=(header &&) = default;
    ~header() = default;

    auto
    c_hdr() const
    {
        return chdr;
    } 

    auto
    timestamp() const{
        return chdr.ts;
    }

    unsigned short 
    caplen() const{
        return chdr.caplen;
    }

    unsigned short 
    len() const{
        return chdr.len;
    }

};

// Specialize for Ethernet header

template<>
class header<hdr::ether> {
private:
    struct ether_header chdr = {0};

public:
    header(){};
    explicit header(const ether_header* ptr) 
    {
        if (ptr != nullptr)
            chdr = *ptr;
    }

    header(const u_char* ptr, ssize_t size) 
    {
        if ( ( ptr == nullptr ) || ( size < sizeof(ether_header) ) ) {
            throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
        }
        chdr = *reinterpret_cast<const ether_header*>(ptr);
    }

    header  (header const& rhs) = default;
    header& operator= (header const &) = default;
    header  (header&& rhs) = default;
    header& operator=(header &&) = default;
    ~header() = default; 

    auto
    c_hdr() const
    {
        return chdr;
    } 
    
    unsigned short
    ethertype() const{
        return chdr.ether_type;
    }

    std::string 
    src_mac() const
    {
        std::stringstream ss;
        ss << std::hex << static_cast<uint16_t>( chdr.ether_shost[0] );
        std::for_each(std::begin(chdr.ether_shost)+1, std::end(chdr.ether_shost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
        return ss.str();
    }

    std::string 
    dst_mac() const
    {
        std::stringstream ss;
        ss << std::hex << static_cast<uint16_t>( chdr.ether_dhost[0] );
        std::for_each(std::begin(chdr.ether_dhost)+1, std::end(chdr.ether_dhost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
        return ss.str();
    }

};

// Specialize for 802.1Q (VLAN) header

template<>
class header<hdr::vlan> {
private:
    struct vlan_header chdr = {0};

public:
    header(){};
    explicit header(const vlan_header* ptr) 
    {
        if (ptr != nullptr)
            chdr = *ptr;
    }

    header(const u_char* ptr, ssize_t size) 
    {
        if ( (ptr == nullptr) || (size < sizeof(vlan_header)) ) {
            throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
        }
        chdr = *reinterpret_cast<const vlan_header*>(ptr);
    }

    header  (header const& rhs) = default;
    header& operator= (header const &) = default;
    header  (header&& rhs) = default;
    header& operator=(header &&) = default;
    ~header() = default; 
    
    auto
    c_hdr() const
    {
        return chdr;
    } 
    unsigned short
    type() const
    {
        return ntohs(chdr.type);
    }       

    unsigned short
    vlan_id() const{
        return ntohs(chdr.vlan_id & 0x0FFF) ;  // ricontrollare
    }

    unsigned short
    tpid() const
    {
        return ntohs(chdr.vlan_tpid);
    }

    std::string 
    src_mac() const
    {
        std::stringstream ss;
        ss << std::hex << static_cast<uint16_t>( chdr.vlan_shost[0] );
        std::for_each(std::begin(chdr.vlan_shost)+1, std::end(chdr.vlan_shost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
        return ss.str();
    }
    
    std::string 
    dst_mac() const
    {
        std::stringstream ss;
        ss << std::hex << static_cast<uint16_t>( chdr.vlan_dhost[0] );
        std::for_each(std::begin(chdr.vlan_dhost)+1, std::end(chdr.vlan_dhost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
        return ss.str();
    }

};


// Specialize for ARP header

template<>
class header<hdr::arp> {
private:
    struct arphdr chdr = {0};

public:
    header(){};
    explicit header(const arphdr* ptr) 
    {
        if (ptr != nullptr)
            chdr = *ptr;
    }

    header(const u_char* ptr, ssize_t size) 
    {
        if ( (ptr==nullptr) || (size < sizeof(arphdr))) {
            throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
        }
        chdr = *reinterpret_cast<const arphdr*>(ptr);
    }

    header  (header const& rhs) = default;
    header& operator= (header const &) = default;
    header  (header&& rhs) = default;
    header& operator=(header &&) = default;
    ~header() = default; 

    auto
    c_hdr() const
    {
        return chdr;
    } 
    unsigned short
    operation() const
    {
        return chdr.ar_op;
    }        
    
};


// Specialize for IPv4 header

template<>
class header<hdr::ipv4> {
private:
    struct ip chdr = {0};

public:
    header(){};
    explicit header(const struct ip* ptr) 
    {
        if (ptr != nullptr) 
            chdr = *ptr;
    }

    header(const u_char* ptr, ssize_t size) 
    {
        if ( (ptr == nullptr) || (size < sizeof(ip)) ) {
            throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
        }
        chdr = *reinterpret_cast<const ip*>(ptr);
    }

    header  (header const& rhs) = default;
    header& operator= (header const &) = default;
    header  (header&& rhs) = default;
    header& operator=(header &&) = default;
    ~header() = default; 

    auto
    c_hdr() const
    {
        return chdr;
    } 
    unsigned short 
    version() const
    {
        return static_cast<unsigned short>(chdr.ip_v);
    }

    unsigned short 
    protocol() const
    {
        return static_cast<unsigned short>(chdr.ip_p);
    }

    unsigned short 
    hlen() const
    {
        return static_cast<unsigned short>(chdr.ip_hl);
    }

    unsigned short 
    len() const
    {
        return static_cast<unsigned short>(chdr.ip_len);
    }
    
    std::string 
    src() const
    {
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, reinterpret_cast<const void*>(&chdr.ip_src.s_addr), buf, sizeof(buf));
        return buf;
    }

    std::string  
    dst() const
    {
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, reinterpret_cast<const void*>(&chdr.ip_dst.s_addr), buf, sizeof(buf));
        return buf;
    }
};


// Specialize for UDP header

template<>
class header<hdr::udp> {
private:
    struct udphdr chdr = {0};

public:
    header(){};
    explicit header(const udphdr* ptr) 
    {
        if (ptr != nullptr)
            chdr = *ptr;
    }

    header(const u_char* ptr, ssize_t size) 
    {
        if ( (ptr == nullptr) || (size < sizeof(udphdr)) ) {
            throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
        }
        chdr = *reinterpret_cast<const udphdr*>(ptr);
    }

    header  (header const& rhs) = default;
    header& operator= (header const &) = default;
    header  (header&& rhs) = default;
    header& operator=(header &&) = default;
    ~header() = default; 

    auto
    c_hdr() const
    {
        return chdr;
    } 
    unsigned short
    srcport() const{
        return ntohs(chdr.uh_sport);
    }

    unsigned short
    dstport() const{
        return ntohs(chdr.uh_dport);
    }

    unsigned short
    length() const{
        return ntohs(chdr.uh_ulen);
    }

};


// Specialize for TCP header

template<>
class header<hdr::tcp> {
private:
    struct tcphdr chdr = {0};

public:
    header(){};
    explicit header(const tcphdr* ptr) 
    {
        if (ptr != nullptr)
            chdr = *ptr;
    }

    header(const u_char* ptr, ssize_t size) 
    {
        if ( (ptr == nullptr) || (size < sizeof(tcphdr)) ) {
            throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
        }
        chdr = *reinterpret_cast<const tcphdr*>(ptr);
    }

    header  (header const& rhs) = default;
    header& operator= (header const &) = default;
    header  (header&& rhs) = default;
    header& operator=(header &&) = default;
    ~header() = default; 
    unsigned short
    srcport() const{
        return ntohs(chdr.th_sport);
    }

    auto
    c_hdr() const
    {
        return chdr;
    } 
    unsigned short
    dstport() const{
        return ntohs(chdr.th_dport);
    }

};


// Specialize for ICMP header

template<>
class header<hdr::icmp> {
private:
    struct icmp chdr = {0};

public:
    header(){};
    explicit header(const struct icmp* ptr) 
    {
        if (ptr != nullptr)
            chdr = *ptr;
    };

    header(const u_char* ptr, ssize_t size) 
    {
        if ( (ptr == nullptr) || (size < sizeof(icmp)) ) {
            throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
        }
        chdr = *reinterpret_cast<const struct icmp*>(ptr);
    }

    header  (header const& rhs) = default;
    header& operator= (header const &) = default;
    header  (header&& rhs) = default;
    header& operator=(header &&) = default;
    ~header() = default; 
    unsigned short 
    type() const
    {
        return chdr.icmp_type;
    }

    auto
    c_hdr() const
    {
        return chdr;
    } 
    unsigned short 
    code() const
    {
        return chdr.icmp_code;
    }
};


}



#endif