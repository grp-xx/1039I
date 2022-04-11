#ifndef _SOCKADDRESS_HPP_
#define _SOCKADDRESS_HPP_

#include <sys/socket.h> 
#include <sys/un.h>

namespace npl {

template<int F>
class sockaddress {};


template<>
class sockaddress<AF_UNIX> {
private: 
    socklen_t   m_len;
    sockaddr_un m_addr;

public:
    sockaddress()  // Empty socket address
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

    std::string name() const 
    {
        return m_addr.sun_path;
    }

    int 
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



}  //end of namespace npl


#endif