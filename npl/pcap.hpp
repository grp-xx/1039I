#ifndef _PCAP_HPP_
#define _PCAP_HPP_
#include <cstddef>
#include <functional>
#include <pcap/pcap.h>
#include <sys/types.h>
#include <system_error>
#include <utility>



enum capture {live, offline};

// typedef void (*pcap_handler)(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes);

namespace npl::pcap {

template <capture mode>
class reader {
private:
    pcap_t* m_handle = nullptr;

public:
    reader() requires (mode == live) //default ctor
    {
        char errbuf[PCAP_ERRBUF_SIZE];
        pcap_t* out;
        if ( (m_handle = pcap_open_live("any", 64, 1, 100, errbuf)) == nullptr ) 
        {
            throw std::system_error(errno, std::generic_category(),"Can't create pcap reader: " + std::string(errbuf));
        }
    }

    explicit reader(std::string device, int snaplen = 64, int promisc = 1, int to_ms = 100) requires (mode == live)
    {
        char errbuf[PCAP_ERRBUF_SIZE];
        pcap_t* out;
        if ( (m_handle = pcap_open_live(device.c_str(), snaplen, promisc, to_ms, errbuf)) == nullptr ) 
        {
            throw std::system_error(errno, std::generic_category(),"Can't open pcap reader: " + std::string(errbuf));
        }
    }

    explicit reader(std::string filename)   requires (mode == offline)
    {
        char errbuf[PCAP_ERRBUF_SIZE];
        pcap_t* out;
        if ( (m_handle = pcap_open_offline(filename.c_str(), errbuf)) == nullptr ) 
        {
            throw std::system_error(errno, std::generic_category(),"Can't open pcap reader: " + std::string(errbuf));
        }


    }
    // Copy constructor and copy assignment operator deleted
    reader(const reader& ) = delete;
    reader& operator=(const reader& ) = delete;

    // Move constructor
    reader(reader&& rhs) 
    : m_handle(rhs.m_handle)
    {
        rhs.m_handle = nullptr;
    }

    // Move operator assignment
    reader& operator=(reader&& rhs)
    {
        if (this != &rhs)
        {
            this->close();      
            m_handle = rhs.m_handle;
            rhs.m_handle = nullptr;
        }
        return *this;
    }

    // Destructor
    ~reader() 
    {
        this->close();
    }

    // Close socket
    void close()
    {
        if (m_handle != nullptr)
        {
            pcap_close(m_handle);
            m_handle = nullptr;
        }
    }

    int datalink() const
    {
        return pcap_datalink(m_handle);
    }

    // Get packets over the pcap reader
    std::pair<const u_char*, struct pcap_pkthdr> next() const
    {
        struct pcap_pkthdr hdr;
        return std::make_pair(pcap_next(m_handle, &hdr),hdr);
    }


//    template<typename Fun, typename T> 
//    int loop(Fun handler, T& user, int cnt = -1) const
//    {
//        int out;
//        if ( (out = pcap_loop(m_handle, cnt, handler, reinterpret_cast<u_char*>(&user)) ) == -1 )
//            throw std::runtime_error("pcap_loop!");
//        return out;
//    }
//    template<typename Fun, typename T> 
//    int dispatch(Fun handler, T& user, int cnt = -1) const
//    {
//        int out;
//        if ( (out = pcap_loop(m_handle, cnt, handler, reinterpret_cast<u_char*>(&user)) ) == -1 )
//            throw std::runtime_error("pcap_loop!");
//        return out;
//    }

//  Slightly more traditional signature
//
    template<typename T>
    int loop(pcap_handler callback, T& user, int cnt = -1) const
    {
        int out;
        if ( (out = pcap_loop(m_handle, cnt, callback, reinterpret_cast<u_char*>(&user)) ) == -1 )
            throw std::runtime_error("pcap_loop!");
        return out;
    }
    template<typename T>
    int dispatch(pcap_handler callback, T& user, int cnt = -1) const
    {
        int out;
        if ( (out = pcap_dispatch(m_handle, cnt, callback, reinterpret_cast<u_char*>(&user)) ) == -1 )
            throw std::runtime_error("pcap_loop!");
        return out;
    }
//
// Even more traditional...
   int loop(pcap_handler callback, u_char* user, int cnt = -1) const
   {
       int out;
       if ( (out = pcap_loop(m_handle, cnt, callback, user) ) == -1 )
           throw std::runtime_error("pcap_loop!");
       return out;
   }

   int dispatch(pcap_handler callback, u_char* user, int cnt = -1) const
   {
       int out;
       if ( (out = pcap_dispatch(m_handle, cnt, callback, user) ) == -1 )
           throw std::runtime_error("pcap_loop!");
       return out;
   }



};

};   // end of namespace npl::pcap



#endif