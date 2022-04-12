#ifndef _SOCKADDRESS_HPP_
#define _SOCKADDRESS_HPP_

#include <sys/socket.h> 
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>

namespace npl {

template<int F>
class sockaddress {};


template<>
class sockaddress<AF_UNIX> {
private: 
    socklen_t   m_len = sizeof(sockaddr_un);
    sockaddr_un m_addr;

public:
    sockaddress()  // Empty socket address
    {
        memset(&m_addr,0,sizeof(sockaddr_un));
        m_addr.sun_family = AF_UNIX;
    }

    explicit sockaddress(const std::string& pathname) 
    {
        memset(&m_addr,0,sizeof(sockaddr_un));
        m_addr.sun_family = AF_UNIX;
        ptrdiff_t offset = 0;
        #ifdef __linux__
        ++offset;
        #endif 
        strncpy(m_addr.sun_path+1,pathname.c_str(),sizeof(m_addr.sun_path) - 1);  

    }

    sockaddress(const sockaddress&)            = default;
    sockaddress& operator=(const sockaddress&) = default;
    sockaddress(sockaddress&&)                 = default;
    sockaddress& operator=(sockaddress&&)      = default;
    ~sockaddress()                             = default;

    const std::string 
    name() const 
    {
        return m_addr.sun_path;
    }

    const int 
    family() const
    {
        return m_addr.sun_family;
    }
       
    const socklen_t 
    len() const     // Note: const is part of the signature!
    {
        return m_len;
    }
        
    socklen_t& 
    len()
    {
        return reinterpret_cast<socklen_t &>(m_len);
    }


    const sockaddr& 
    c_addr() const
    {
        return reinterpret_cast<const sockaddr&>(m_addr);
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
    socklen_t   m_len = sizeof(sockaddr_in);
    sockaddr_in m_addr;

public:
    sockaddress() 
    {
        memset(&m_addr,0,sizeof(sockaddr_in));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port   = 0;
        m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    
    sockaddress(const sockaddr_in& addr)
    : m_len(sizeof(sockaddr_in))
    {
        memset(&m_addr, 0, sizeof(sockaddr_in)); 
        m_addr = addr;
    }

    sockaddress(const in_addr& ip, const in_port_t port) 
    {
        memset(&m_addr,0,sizeof(sockaddr_in));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port   = htons(port);
        m_addr.sin_addr   = ip;
    }

    sockaddress(const std::string& host, const in_port_t port) 
    {
        memset(&m_addr,0,sizeof(sockaddr_in));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port   = htons(port);

        if (host.empty()) {
            m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            return;
        }

//        if ( inet_pton(AF_INET, host.c_str(), &m_addr.sin_addr.s_addr) <=0 ) {
//            throw std::system_error(errno, std::generic_category(),"inet_pton error");
//        }

        addrinfo hints, *result;
        hints.ai_family = AF_INET;
        hints.ai_flags  = 0;
        hints.ai_socktype = 0;
        hints.ai_protocol = 0;

        if (getaddrinfo(host.c_str(), nullptr, &hints, &result) !=0 ) {
            throw std::system_error(errno, std::generic_category(),"getaddrinfo");
        } 

        sockaddr_in *sa = reinterpret_cast<sockaddr_in*>(result->ai_addr);
        m_addr.sin_addr = sa->sin_addr;

        freeaddrinfo(result);

    }

    sockaddress(const std::string& host, const std::string& service) 
    {
        memset(&m_addr,0,sizeof(sockaddr_in));
        m_addr.sin_family = AF_INET;

        if (host.empty()) {
            m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            return;
        }

//        if ( inet_pton(AF_INET, host.c_str(), &m_addr.sin_addr.s_addr) <=0 ) {
//            throw std::system_error(errno, std::generic_category(),"inet_pton error");
//        }

        addrinfo hints, *result;
        hints.ai_family = AF_INET;
        hints.ai_flags  = 0;
        hints.ai_socktype = 0;
        hints.ai_protocol = 0;

        if (getaddrinfo(host.c_str(), service.c_str(), &hints, &result) !=0 ) {
            throw std::system_error(errno, std::generic_category(),"getaddrinfo");
        } 

        sockaddr_in *sa = reinterpret_cast<sockaddr_in*>(result->ai_addr);
        m_addr = *sa;
        freeaddrinfo(result);

    }

        sockaddress(sockaddress const &) = default;
        sockaddress& operator=(sockaddress const &) = default;
        sockaddress(sockaddress &&) = default;
        sockaddress& operator=(sockaddress &&) = default;
        ~sockaddress() = default;



    const std::string 
    host() const
    {
        char ip[INET_ADDRSTRLEN];
        if ( inet_ntop(AF_INET, reinterpret_cast<const void*>(&m_addr.sin_addr), ip, sizeof(ip)) == nullptr) {
            throw std::system_error(errno, std::generic_category(),"inet_ntop error");
        }
        return ip;
    }

    const in_port_t
    port() const
    {
        return ntohs(m_addr.sin_port);
    }

    const int family() const
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

    const std::pair<std::string, std::string>
    getnameinfo() const 
    {
        char host[NI_MAXHOST], service[NI_MAXSERV];
        if (::getnameinfo(&this->c_addr(),this->len(), host, sizeof(host), service, sizeof(service), 0) != 0) {
            throw std::system_error(errno,std::generic_category(),"getnameinfo");
        }
        return std::make_pair(host, service);
    }


};  // end of aockadress<AF_INIT>




    template<>
    class sockaddress<AF_INET6> {
    private: 
        socklen_t    m_len = sizeof(sockaddr_in6);
        sockaddr_in6 m_addr;
    
    public:
        sockaddress() 
        {
            memset(&m_addr, 0, sizeof(sockaddr_in6)); 
            m_addr.sin6_family     = AF_INET6;
            m_addr.sin6_port       = 0;
            m_addr.sin6_flowinfo   = 0;
            m_addr.sin6_addr       = in6addr_any;
            m_addr.sin6_scope_id   = 0;
        }

        sockaddress(const sockaddr_in6& addr)
        {
            memset(&m_addr, 0, sizeof(sockaddr_in6)); 
            m_addr = addr;
        }

        sockaddress(const in6_addr& ip6, const in_port_t port) 
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


        const std::string 
        host() const        
        {
            char buf[INET6_ADDRSTRLEN];
            if ( inet_ntop(AF_INET6, reinterpret_cast<const void*>(&m_addr.sin6_addr), buf, sizeof(buf)) == nullptr) {
                throw std::system_error(errno,std::generic_category());
            } 
            return buf;
 
        }

        const in_port_t 
        port() const
        {
            return ntohs(m_addr.sin6_port);

        }

        const int family() const
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

        const std::pair<std::string, std::string>
        getnameinfo() const 
        {
            char host[NI_MAXHOST], service[NI_MAXSERV];
            if (::getnameinfo(&this->c_addr(),this->len(), host, sizeof(host), service, sizeof(service), 0) != 0) {
                throw std::system_error(errno,std::generic_category(),"getnameinfo");
            }
            return std::make_pair(host, service);
        }



    };


}  //end of namespace npl


#endif