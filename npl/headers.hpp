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
#include <sstream>
#include "sockaddress.hpp"
#include "socket.hpp"



struct vlan_header {
    u_char    vlan_dhost[ETHER_ADDR_LEN];
    u_char    vlan_shost[ETHER_ADDR_LEN];
    u_int16_t vlan_tpid;
    u_int16_t vlan_id;
    u_int16_t vlan_type;
};


//enum Layer {phy = 1, datalink = 2, network = 3, transport = 4, application = 5};
enum Proto {unspec, ether, vlan, arp, ipv4, ipv6, icmp, tcp, udp};



namespace npl {

    template <Proto value>
    class pktheader {};


    template<>
    class pktheader<ether> {
    private:
        struct ether_header c_header = {0};

    public:
        explicit pktheader(const ether_header* ptr) 
        {
            if (ptr != nullptr)
                c_header = *ptr;
        }

        pktheader(const u_char* ptr, ssize_t size) 
        {
            if ( ( ptr == nullptr ) || ( size < sizeof(ether_header) ) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            c_header = *reinterpret_cast<const ether_header*>(ptr);
        }

        unsigned short
        ethertype() const{
            return c_header.ether_type;
        }

        std::string 
        src_mac() const
        {
            std::stringstream ss;
            ss << std::hex << static_cast<uint16_t>( c_header.ether_shost[0] );
            std::for_each(std::begin(c_header.ether_shost)+1, std::end(c_header.ether_shost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
            return ss.str();
        }

        std::string 
        dst_mac() const
        {
            std::stringstream ss;
            ss << std::hex << static_cast<uint16_t>( c_header.ether_dhost[0] );
            std::for_each(std::begin(c_header.ether_dhost)+1, std::end(c_header.ether_dhost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
            return ss.str();
        }

    };

    template<>
    class pktheader<vlan> {
    private:
        struct vlan_header c_header = {0};

    public:
        explicit pktheader(const vlan_header* ptr) 
        {
            if (ptr != nullptr)
                c_header = *ptr;
        }

        pktheader(const u_char* ptr, ssize_t size) 
        {
            if ( (ptr == nullptr) || (size < sizeof(vlan_header)) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            c_header = *reinterpret_cast<const vlan_header*>(ptr);
        }

        
        unsigned short
        type() const
        {
            return ntohs(c_header.vlan_type);
        }       

        unsigned short
        vlan_id() const{
            return ntohs(c_header.vlan_id & 0x0FFF) ;  // ricontrollare
        }

        unsigned short
        tpid() const
        {
            return ntohs(c_header.vlan_tpid);
        }

        std::string 
        src_mac() const
        {
            std::stringstream ss;
            ss << std::hex << static_cast<uint16_t>( c_header.vlan_shost[0] );
            std::for_each(std::begin(c_header.vlan_shost)+1, std::end(c_header.vlan_shost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
            return ss.str();
        }
        
        std::string 
        dst_mac() const
        {
            std::stringstream ss;
            ss << std::hex << static_cast<uint16_t>( c_header.vlan_dhost[0] );
            std::for_each(std::begin(c_header.vlan_dhost)+1, std::end(c_header.vlan_dhost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
            return ss.str();
        }

    };


    template<>
    class pktheader<arp> {
    private:
        struct arphdr c_header = {0};

    public:
        explicit pktheader(const arphdr* ptr) 
        {
            if (ptr != nullptr)
                c_header = *ptr;
        }

        pktheader(const u_char* ptr, ssize_t size) 
        {
            if ( (ptr==nullptr) || (size < sizeof(arphdr))) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            c_header = *reinterpret_cast<const arphdr*>(ptr);
        }


        unsigned short
        operation() const
        {
            return c_header.ar_op;
        }        
        
    };


    template<>
    class pktheader<ipv4> {
    private:
        struct ip c_header = {0};

    public:
        explicit pktheader(const struct ip* ptr) 
        {
            if (ptr != nullptr) 
                c_header = *ptr;
        }

        pktheader(const u_char* ptr, ssize_t size) 
        {
            if ( (ptr == nullptr) || (size < sizeof(ip)) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            c_header = *reinterpret_cast<const ip*>(ptr);
        }

        unsigned short 
        version() const{
            return static_cast<unsigned short>(c_header.ip_v);
        }

        unsigned short 
        protocol() const{
            return static_cast<unsigned short>(c_header.ip_p);
        }

        std::string 
        src() const
        {
            char buf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, reinterpret_cast<const void*>(&c_header.ip_src.s_addr), buf, sizeof(buf));
            return buf;
        }

        std::string  
        dst() const
        {
            char buf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, reinterpret_cast<const void*>(&c_header.ip_dst.s_addr), buf, sizeof(buf));
            return buf;
        }
    };



    template<>
    class pktheader<udp> {
    private:
        struct udphdr c_header = {0};

    public:
        explicit pktheader(const udphdr* ptr) 
        {
            if (ptr != nullptr)
                c_header = *ptr;
        }

        pktheader(const u_char* ptr, ssize_t size) 
        {
            if ( (ptr == nullptr) || (size < sizeof(udphdr)) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            c_header = *reinterpret_cast<const udphdr*>(ptr);
        }

        unsigned short
        srcport() const{
            return ntohs(c_header.uh_sport);
        }

        unsigned short
        dstport() const{
            return ntohs(c_header.uh_dport);
        }

        unsigned short
        length() const{
            return ntohs(c_header.uh_ulen);
        }
    
    };


    template<>
    class pktheader<tcp> {
    private:
        struct tcphdr c_header = {0};

    public:
        explicit pktheader(const tcphdr* ptr) 
        {
            if (ptr != nullptr)
                c_header = *ptr;
        }

        pktheader(const u_char* ptr, ssize_t size) 
        {
            if ( (ptr == nullptr) || (size < sizeof(tcphdr)) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            c_header = *reinterpret_cast<const tcphdr*>(ptr);
        }

        unsigned short
        srcport() const{
            return ntohs(c_header.th_sport);
        }

        unsigned short
        dstport() const{
            return ntohs(c_header.th_dport);
        }
    
    };


    template<>
    class pktheader<icmp> {
    private:
        struct icmp c_header = {0};

    public:
        explicit pktheader(const struct icmp* ptr) 
        {
            if (ptr != nullptr)
                c_header = *ptr;
        };

        pktheader(const u_char* ptr, ssize_t size) 
        {
            if ( (ptr == nullptr) || (size < sizeof(icmp)) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            c_header = *reinterpret_cast<const struct icmp*>(ptr);
        }

        unsigned short 
        type() const
        {
            return c_header.icmp_type;
        }

        unsigned short 
        code() const
        {
            return c_header.icmp_code;
        }
    };


}



#endif