#ifndef _SOCKADDRESS_HPP_
#define _SOCKADDRESS_HPP_
#include <cstdio>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <sstream>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <system_error>
#include <algorithm>

#ifdef __linux__
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#endif




namespace npl {

    template<int F>       // F: Address Family
    class sockaddress; 

    template<>
    class sockaddress<AF_UNIX> {
    private: 
        socklen_t   m_len;
        sockaddr_un m_addr;

    public:
        sockaddress()   // Empty socket address: to be passed to socket methods that need value-result arguments
        : m_len(sizeof(sockaddr_un))
        {
            memset(&m_addr,0,sizeof(sockaddr_un));
            m_addr.sun_family = AF_UNIX;
        }

        explicit sockaddress(const std::string& pathname)
        : m_len(sizeof(sockaddr_un)) 
        {
            memset(&m_addr,0,sizeof(sockaddr_un));
            m_addr.sun_family = AF_UNIX;
            ptrdiff_t offset = 0;
            // Optimization for Linux Abstract Addressing Namespace
            #ifdef __linux__
            ++offset;
            #endif
            // End optimization
            strncpy(m_addr.sun_path+offset,pathname.c_str(),sizeof(m_addr.sun_path)-1);
        }

        sockaddress(const sockaddress&)            = default;
        sockaddress& operator=(const sockaddress&) = default;
        sockaddress(sockaddress&&)                 = default; 
        sockaddress& operator=(sockaddress&&)      = default;
        ~sockaddress()                             = default;


        std::string 
        name() const        
        {
            //return static_cast<std::string>(m_addr.sun_path);
            return m_addr.sun_path;
        } 

        int 
        family() const
        {
            return m_addr.sun_family;
        }

        const socklen_t 
        len() const                       // Note: const is part of the signature!
        {
            return m_len;
        }

        socklen_t & 
        len()
        {
            return reinterpret_cast<socklen_t &>(m_len); 
        }
        
        const sockaddr& 
        c_addr() const                        // Note: const is part of the signature!
        {
            return reinterpret_cast<const sockaddr &>(m_addr);
        }

        sockaddr& 
        c_addr()
        {
            return reinterpret_cast<sockaddr &>(m_addr);
        }

    };
    
   

    template<>
    class sockaddress<AF_INET> {
    private: 
        socklen_t   m_len;
        sockaddr_in m_addr;
    
    public:
        sockaddress() 
        : m_len(sizeof(sockaddr_in))
        {
            memset(&m_addr, 0, sizeof(sockaddr_in)); 
            m_addr.sin_family        = AF_INET;
            m_addr.sin_port          = 0;
            m_addr.sin_addr.s_addr   = htonl(INADDR_ANY);
        }

        sockaddress(const sockaddr_in& addr)
        : m_len(sizeof(sockaddr_in))
        {
            memset(&m_addr, 0, sizeof(sockaddr_in)); 
//            m_addr.sin_family = AF_INET;
//            m_addr.sin_port = addr.sin_port;
//            m_addr.sin_addr = addr.sin_addr;
            m_addr = addr;
        }

        sockaddress(const in_addr& ip, const in_port_t port) 
        : m_len(sizeof(sockaddr_in))
        {
            memset(&m_addr, 0, sizeof(sockaddr_in)); 
            m_addr.sin_family = AF_INET;
            m_addr.sin_port = htons(port);
            m_addr.sin_addr = ip;
        }

        sockaddress(const std::string& host, const in_port_t port) 
        : m_len(sizeof(sockaddr_in))
        {
            memset(&m_addr, 0, sizeof(sockaddr_in)); 
            m_addr.sin_family = AF_INET;
            m_addr.sin_port   = htons(port);

            if (host.empty()) {
                m_addr.sin_addr.s_addr   = htonl(INADDR_ANY);
                return;
            }

//        
//            
            if ( inet_pton(AF_INET, host.c_str(), &m_addr.sin_addr.s_addr) <= 0 ) 
            {
                throw std::system_error(errno,std::generic_category(),"inet_pton error");
            }
//            
//
//            addrinfo hints, *res;
//            hints.ai_family   = AF_INET;
//            hints.ai_socktype = 0;
//            hints.ai_protocol = 0;
//            hints.ai_flags    = 0;
//
//            if (getaddrinfo(host.c_str(), nullptr, &hints, &res) !=0 ) {
//                throw std::system_error(errno,std::generic_category(),"getaddrinfo");
//            } 
//
//            sockaddr_in* sa = reinterpret_cast<sockaddr_in *>(res->ai_addr);
//            m_addr.sin_addr = sa->sin_addr;
//            freeaddrinfo(res);

        }


        sockaddress(const std::string& host, const std::string& service) 
        : m_len(sizeof(sockaddr_in))
        {
            memset(&m_addr, 0, sizeof(sockaddr_in)); 
            m_addr.sin_family = AF_INET;

            if (host.empty()) {
                m_addr.sin_addr.s_addr   = htonl(INADDR_ANY);
                return;
            }

            addrinfo hints, *res;
            hints.ai_family   = AF_INET;
            hints.ai_socktype = 0;
            hints.ai_protocol = 0;
            hints.ai_flags    = 0;

            if (getaddrinfo(host.c_str(), service.c_str(), &hints, &res) !=0 ) {
                throw std::system_error(errno,std::generic_category());
            } 

            sockaddr_in* sa = reinterpret_cast<sockaddr_in *>(res->ai_addr);
            m_addr = *sa;
            freeaddrinfo(res);

        }

        sockaddress(sockaddress const &) = default;
        sockaddress& operator=(sockaddress const &) = default;
        sockaddress(sockaddress &&) = default;
        sockaddress& operator=(sockaddress &&) = default;
        ~sockaddress() = default;

        const std::string host() const        
        {
            char buf[INET_ADDRSTRLEN];
            if ( inet_ntop(AF_INET, reinterpret_cast<const void*>(&m_addr.sin_addr.s_addr), buf, sizeof(buf)) == nullptr) {
                throw std::system_error(errno,std::generic_category());
            } 
            return buf;
 
        }

        const int 
        port() const
        {
            return ntohs(m_addr.sin_port);

        }

        int family() const
        {
            return m_addr.sin_family;
            
        }

        const socklen_t len() const                       // Note: const is part of the signature!
        {
            return m_len;
        }

        socklen_t & len()
        {
            return reinterpret_cast<socklen_t &>(m_len); 
        }
        
        const sockaddr& c_addr() const                        // Note: const is part of the signature!
        {
            return reinterpret_cast<const sockaddr &>(m_addr);
        }

        sockaddr& c_addr()
        {
            return reinterpret_cast<sockaddr &>(m_addr);
        }

        std::pair<std::string, std::string>
        getnameinfo() const 
        {
            char host[NI_MAXHOST], service[NI_MAXSERV];
            if (::getnameinfo(&this->c_addr(),this->len(), host, sizeof(host), service, sizeof(service), 0) != 0) {
                throw std::system_error(errno,std::generic_category(),"getnameinfo");
            }
            return std::make_pair(host, service);
        }


    };



    template<>
    class sockaddress<AF_INET6> {
    private: 
        socklen_t    m_len;
        sockaddr_in6 m_addr;
    
    public:
        sockaddress() 
        : m_len(sizeof(sockaddr_in6))
        {
            memset(&m_addr, 0, sizeof(sockaddr_in6)); 
            m_addr.sin6_family     = AF_INET6;
            m_addr.sin6_port       = 0;
            m_addr.sin6_flowinfo   = 0;
            m_addr.sin6_addr       = in6addr_any;
            // memcpy(&m_addr.sin6_addr,&in6addr_any,sizeof(in6_addr));
            m_addr.sin6_scope_id   = 0;
        }

        sockaddress(const sockaddr_in6& addr)
        : m_len(sizeof(sockaddr_in6))
        {
            m_addr = addr;
            memcpy(&m_addr.sin6_addr, &addr.sin6_addr.s6_addr, sizeof(in6_addr));
        }

        sockaddress(const in6_addr& ip6, const in_port_t port) 
        : m_len(sizeof(sockaddr_in6))
        {
            memset(&m_addr, 0, sizeof(sockaddr_in6)); 
            m_addr.sin6_family    = AF_INET6;
            m_addr.sin6_port      = htons(port);
            m_addr.sin6_flowinfo  = 0;
            m_addr.sin6_addr      = ip6;
            m_addr.sin6_scope_id  = 0;
        }

        sockaddress(const std::string& host, const in_port_t port) 
        : m_len(sizeof(sockaddr_in6))
        {
            memset(&m_addr, 0, sizeof(sockaddr_in6)); 
            m_addr.sin6_family = AF_INET6;
            m_addr.sin6_port   = htons(port);
            if (host.empty()) {
                m_addr.sin6_addr   = in6addr_any;
                return;
            }

//            if ( inet_pton(AF_INET6, host.c_str(), &m_addr.sin6_addr.s6_addr) <= 0 ) {
//                throw std::system_error(errno, std::generic_category());
//            }
//
            addrinfo hints, *res;
            hints.ai_family   = AF_INET6;
            hints.ai_socktype = 0;
            hints.ai_protocol = 0;
            hints.ai_flags    = 0;

            if (getaddrinfo(host.c_str(), nullptr, &hints, &res) !=0 ) {
                throw std::system_error(errno,std::generic_category());
            } 

            sockaddr_in6* sa = reinterpret_cast<sockaddr_in6 *>(res->ai_addr);
            m_addr.sin6_addr = sa->sin6_addr;
            freeaddrinfo(res);

        }

        sockaddress(const std::string& host, const std::string& service) 
        : m_len(sizeof(sockaddr_in6))
        {
            memset(&m_addr, 0, sizeof(sockaddr_in6)); 
            m_addr.sin6_family = AF_INET6;

            if (host.empty()) {
                m_addr.sin6_addr   = in6addr_any;
                return;
            }

            addrinfo hints, *res;
            hints.ai_family   = AF_INET6;
            hints.ai_socktype = 0;
            hints.ai_protocol = 0;
            hints.ai_flags    = 0;

            if (getaddrinfo(host.c_str(), service.c_str(), &hints, &res) !=0 ) {
                throw std::system_error(errno,std::generic_category());
            } 

            sockaddr_in6* sa = reinterpret_cast<sockaddr_in6 *>(res->ai_addr);
            m_addr = *sa;
            freeaddrinfo(res);

        }

        sockaddress(sockaddress const &) = default;
        sockaddress& operator=(sockaddress const &) = default;
        sockaddress(sockaddress &&) = default;
        sockaddress& operator=(sockaddress &&) = default;
        ~sockaddress() = default;


        const std::string host() const        
        {
            char buf[INET6_ADDRSTRLEN];
            if ( inet_ntop(AF_INET6, reinterpret_cast<const void*>(&m_addr.sin6_addr), buf, sizeof(buf)) == nullptr) {
                throw std::system_error(errno,std::generic_category());
            } 
            return buf;
 
        }

        const int 
        port() const
        {
            return ntohs(m_addr.sin6_port);

        }

        int family() const
        {
            return m_addr.sin6_family;
            
        }

        const socklen_t len() const                       // Note: const is part of the signature!
        {
            return m_len;
        }

        socklen_t & len()
        {
            return reinterpret_cast<socklen_t &>(m_len); 
        }
        
        const sockaddr& c_addr() const                        // Note: const is part of the signature!
        {
            return reinterpret_cast<const sockaddr &>(m_addr);
        }

        sockaddr& c_addr()
        {
            return reinterpret_cast<sockaddr &>(m_addr);
        }

        std::pair<std::string, std::string>
        getnameinfo() const 
        {
            char host[NI_MAXHOST], service[NI_MAXSERV];
            if (::getnameinfo(&this->c_addr(),this->len(), host, sizeof(host), service, sizeof(service), 0) != 0) {
                throw std::system_error(errno,std::generic_category(),"getnameinfo");
            }
            return std::make_pair(host, service);
        }



    };


    #ifdef __linux__

    template<>
    class sockaddress<AF_PACKET> {
    private: 
        socklen_t   m_len;
        sockaddr_ll m_addr;

    public:
        sockaddress()   // Empty socket address: to be passed to socket methods that need value-result arguments
        : m_len(sizeof(sockaddr_ll))
        {
            memset(&m_addr,0,sizeof(sockaddr_ll));
            m_addr.sll_family = AF_PACKET;
        }

        explicit sockaddress(int if_index, int protocol = ETH_P_ALL)
        : m_len(sizeof(sockaddr_ll)) 
        {
            memset(&m_addr,0,sizeof(sockaddr_ll));
            m_addr.sll_family   = AF_PACKET;
            m_addr.sll_ifindex  = if_index;
            m_addr.sll_protocol = htons(protocol); 
        }

        explicit sockaddress(const std::string& iface, int protocol = ETH_P_ALL)
        : m_len(sizeof(sockaddr_ll)) 
        {
            memset(&m_addr,0,sizeof(sockaddr_ll));
            m_addr.sll_family   = AF_PACKET;
            if ( (m_addr.sll_ifindex  = if_nametoindex(iface.c_str())) == 0) 
            {
                throw std::system_error(errno,std::system_category(),"Interface not available");
            }
            m_addr.sll_protocol = htons(protocol); 
        }
        
        sockaddress(const sockaddress&)            = default;
        sockaddress& operator=(const sockaddress&) = default;
        sockaddress(sockaddress&&)                 = default; 
        sockaddress& operator=(sockaddress&&)      = default;
        ~sockaddress()                             = default;


        int 
        ifindex() const        
        {
            return m_addr.sll_ifindex;
        } 

        std::string
        ifname() const        
        {
            char name[64];
            if ( m_addr.sll_ifindex == 0 ) {
                return "Any";
            }
            if ( if_indextoname(m_addr.sll_ifindex, name) == nullptr ) 
            {
                throw std::system_error(errno,std::system_category(),"Interface name error");
            }           
            return (m_addr.sll_ifindex == 0) ? "Any" : name; 
        }

        std::string 
        hw_addr() const
        {
            std::stringstream ss;
            ss << std::hex << static_cast<uint16_t>( m_addr.sll_addr[0]);
            std::for_each(std::begin(m_addr.sll_addr)+1, std::end(m_addr.sll_addr), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
            return ss.str();
        }

        unsigned short
        hw_len() const{
            return m_addr.sll_halen;
        }

        unsigned short
        hw_type() const{
            return m_addr.sll_hatype;
        }

        unsigned short
        pkt_type() const{
            return m_addr.sll_pkttype;
        }

        int 
        family() const
        {
            return m_addr.sll_family;
        }

        const socklen_t 
        len() const                       // Note: const is part of the signature!
        {
            return m_len;
        }

        socklen_t & 
        len()
        {
            return reinterpret_cast<socklen_t &>(m_len); 
        }
        
        const sockaddr& 
        c_addr() const                        // Note: const is part of the signature!
        {
            return reinterpret_cast<const sockaddr &>(m_addr);
        }

        sockaddr& 
        c_addr()
        {
            return reinterpret_cast<sockaddr &>(m_addr);
        }

    };

    #endif
    




} // end namespace npl



#endif