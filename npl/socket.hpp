#ifndef _SOCKET_HPP_
#define _SOCKET_HPP_
#include <net/if.h>
#ifdef __linux__
#include <linux/if_packet.h>
#endif 
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "json.hpp"
#include "sockaddress.hpp"

#include <sys/types.h>
#include <sys/uio.h>

#include <string>
#include <vector>
#include <cstdint>
#include <memory>


namespace npl {

    typedef std::vector<uint8_t> buffer;      // Simple definition of buffer as a vector of char

    template <int F, int type>   // F: Address Family, type: type of socket (check if __socket_type is needed...) 
    class socket {

    private:
        int m_sockfd;

    public:

        // Default constructor
        explicit socket(int protocol = 0)                          
        {
            // std::cout << "protocol: " << std::hex << protocol << std::endl;
            if ( (m_sockfd = ::socket(F, type, protocol)) == -1 ) {
                throw std::system_error(errno, std::generic_category(),"socket");
            }
        }

        // Copy constructor and copy assignment operator deleted
        socket(const socket& ) = delete;
        socket& operator=(const socket& ) = delete;

        // Move constructor
        socket(socket&& rhs) 
        : m_sockfd(rhs.m_sockfd)
        {
            rhs.m_sockfd = -1;
        }

        // Move operator assignment
        socket& operator=(socket&& rhs)
        {
            if (this != &rhs)
            {
                this->close();       // Note: closing a socket does not mean destroy the variable
                m_sockfd = rhs.m_sockfd;
                rhs.m_sockfd = -1;
            }
            return *this;
        }

        // Destructor
        ~socket() 
        {
            this->close();
        }

        // Close socket
        void close()
        {
            if (m_sockfd != -1)
            {
                ::close(m_sockfd);
                m_sockfd = -1;
            }
        }


        int shutdown(int how)
        {
            return ::shutdown(m_sockfd, how);
        }
        
        int bind(const sockaddress<F>& addr)
        {
            if constexpr (F == AF_UNIX)     // Evaluate at compile time 
            {
                unlink(addr.name().c_str());
            }
            int out = ::bind(m_sockfd, &addr.c_addr(),addr.len());
            if (out == -1) {
                throw std::system_error(errno, std::system_category(),"bind");
            }
            return out;
        }


        int listen(int backlog = 5)
        {
            int out = ::listen(m_sockfd, backlog);
            if (out == -1) {
                throw std::system_error(errno, std::system_category(),"listen");
            }
            return out;
        }

        int connect(const sockaddress<F> remote)
        {
            int out = ::connect(m_sockfd, &remote.c_addr(), remote.len());
            if (out == -1) {
                throw std::system_error(errno, std::system_category(),"connect");
            }
            return out;
        }
        
        std::pair< socket, sockaddress<F> > accept()
        {
            socket accepted;
            sockaddress<F> peer;
            // accepted.m_sockfd = ::accept(m_sockfd, reinterpret_cast<sockaddr *>(peer.m_addr), &peer.m_len); // Alternative (equivalent)
            accepted.m_sockfd = ::accept(m_sockfd, &peer.c_addr(), &peer.len());
            if (accepted.m_sockfd == -1) {
                throw std::system_error(errno, std::system_category(),"accept");
            } 
            
            return std::make_pair(std::move(accepted),peer);
        }
        


        // I/O methods (do not trow)
        // Connected sockets:      write(), read(), send(), recv()
        // Connectionless sockets: sendto(), recvfrom()


        std::ptrdiff_t     write(const buffer& buf) const
        {
           return ::write(m_sockfd, &buf[0], buf.size());
        }

        std::ptrdiff_t      send(const buffer& buf, int flags = 0) const
        {
           return ::send(m_sockfd, &buf[0], buf.size(), flags);
        }
        
        std::ptrdiff_t    sendto(const buffer& buf, const sockaddress<F>& remote, int flags = 0) const
        {
            return ::sendto(m_sockfd, &buf[0], buf.size(), flags, &remote.c_addr(), remote.len());
        }

        std::ptrdiff_t      read(buffer& buf) const
        {
           return ::read(m_sockfd, &buf[0], buf.size());
        }


        buffer              read(int len) const
        {
            buffer buf(len);
            std::ptrdiff_t n = ::read(m_sockfd, &buf[0], buf.size());
            return buffer(buf.begin(),buf.begin()+n);
        }

        
        std::ptrdiff_t      recv(buffer& buf, int flags = 0) const
        {
            return ::recv(m_sockfd, &buf[0], buf.size(), flags);
        }

        buffer              recv(int len, int flags = 0) const
        {
            buffer buf(len);
            std::ptrdiff_t n = ::recv(m_sockfd, &buf[0], buf.size(), flags);
            return buffer(buf.begin(),buf.begin()+n);
        }

        std::ptrdiff_t  recvfrom(buffer& buf, sockaddress<F> remote, int flags) const
        {
            return ::recvfrom(&buf[0],buf.size(), flags, &remote.c_addr(), &remote.len());
        }

        std::pair< buffer, sockaddress<F> > recvfrom(int len, int flags = 0) const 
        {
            sockaddress<F> peer;
            buffer buf(len);
            int n = ::recvfrom(m_sockfd,&buf[0],len,flags,&peer.c_addr(),&peer.len());
            return std::make_pair(buffer(buf.begin(),buf.begin()+n),peer);
        }


        // Advanced socket methods (To be reviewed and made better possibly)

        sockaddress<F> 
        getsockname() const
        {
            sockaddress<F> name;
            if (::getsockname(m_sockfd, &name.c_addr(), &name.len() ) !=0 ) {
               throw std::system_error(errno,std::generic_category(),"getsockname");
            }
            return name;
        }

        sockaddress<F>
        getpeername() const
        {
            sockaddress<F> name;
            if (::getpeername(m_sockfd, &name.c_addr(), &name.len() ) !=0 ) {
               throw std::system_error(errno,std::generic_category(),"getpeername");
            }
            return name;

        }


        int
        setsockopt(int level, int optname, const void *optval, socklen_t optlen)
        {
            int out = ::setsockopt(m_sockfd, level, optname, optval, optlen);
            if (out == -1) {
                throw std::system_error(errno,std::generic_category(),"setsockopt");
            }
            return out;
        }

        int
        getsockopt(int level, int optname, void *optval, socklen_t *optlen) const
        {
            int out = ::getsockopt(m_sockfd, level, optname, optval, optlen);
            if (out == -1) {
                throw std::system_error(errno,std::generic_category(),"getsockopt");
            }
            return out;
        }


        // Setting specific socket options

        // Enable reusing TCP/UDP ddress
        int
        set_reuseaddr() 
        {
            int reuse = 1;
            int out = ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
            if (out == -1) {
                throw std::system_error(errno,std::generic_category(),"set_reuseaddr");
            }
            return out;
        }

        // Put interface in promisc mode

        #ifdef __linux__
        
        int set_promisc(int ifindex) 
        {
            struct packet_mreq mreq = {0};
            mreq.mr_ifindex = ifindex;
            mreq.mr_type = PACKET_MR_PROMISC;
            int out = ::setsockopt(m_sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
            if (out == -1) {
                throw std::system_error(errno,std::generic_category(),"set_promisc");
            }
            return out;
        }

        int set_promisc(std::string device)
        {
            int if_index = if_nametoindex(device.c_str());
            return this->set_promisc(if_index);
        }

        // Enable fanout mode (join fanout group)
        int set_fanout(int group, int mode = PACKET_FANOUT_HASH)
        {
            int arg = ( (mode << 16) | group);
            int out = ::setsockopt(m_sockfd, SOL_PACKET, PACKET_FANOUT, &arg, sizeof(arg));
            if (out == -1) {
                throw std::system_error(errno,std::generic_category(),"set_fanout");
            }
            return out;
        }

        #endif


    };


} // end of namespace npl


#endif