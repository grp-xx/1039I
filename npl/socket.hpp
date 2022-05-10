#ifndef _SOCKET_HPP_
#define _SOCKET_HPP_

#include <sys/socket.h>
#include <unistd.h>
#include "sockaddress.hpp"
#include <vector>
#include <system_error>

namespace npl {

typedef std::vector<u_char> buffer;

template<int F, int type>
class socket
{
private:
    int m_sockfd;

public:
    explicit socket(int protocol = 0) 
    { 
        if ((m_sockfd = ::socket(F, type, protocol)) == -1) {
            throw std::system_error(errno,std::generic_category(),"socket");
        } 
    }

    socket(const socket& rhs) = delete;
    socket& operator=(const socket& rhs) = delete;

    socket(socket&& rhs) noexcept 
    : m_sockfd(rhs.m_sockfd)
    {
        rhs.m_sockfd = -1;
    }

    socket& operator=(socket&& rhs) noexcept
    {
        if (this != &rhs)
        {
            this->close();
            m_sockfd = rhs.m_sockfd;
            rhs.m_sockfd = -1;
        }
        return *this;
    }

    ~socket()
    {
        this->close();
    }

// Basic socket management 

    void close()
    {
        if (m_sockfd != -1) {
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
        if constexpr (F == AF_UNIX) {
            unlink(addr.name().c_str());
        }
        int out = ::bind(m_sockfd,&addr.c_addr(),addr.len());
        if (out == -1) {
            throw std::system_error(errno,std::system_category(),"bind");
        }
        return out;
    }

    int listen(int backlog = 5) 
    {
        int out = ::listen(m_sockfd, backlog);
        if (out == -1) {
            throw std::system_error(errno,std::system_category(),"listen");
        }
        return out;
    }


    int connect(const sockaddress<F>& peer) 
    {
        int out = ::connect(m_sockfd, &peer.c_addr(), peer.len());
        if (out == -1) {
            throw std::system_error(errno,std::system_category(),"connect");
        }
        return out;
    }

    std::pair<socket,sockaddress<F>> accept()
    {
        socket accepted;
        sockaddress<F> peer;
        if ( (accepted.m_sockfd = ::accept(m_sockfd, &peer.c_addr(),&peer.len())) == -1 )
        {
            throw std::system_error(errno,std::system_category(),"accept");
        } 

        return std::make_pair(std::move(accepted),peer);

    }

    std::ptrdiff_t write(const buffer& buf) const
    {
        return ::write(m_sockfd, &buf[0], buf.size());
    }

    std::ptrdiff_t      send(const buffer& buf, int flags = 0) const
    {
       return ::send(m_sockfd, &buf[0], buf.size(), flags);
    }

    std::ptrdiff_t read(buffer& buf) const
    {
        return ::read(m_sockfd, &buf[0], buf.size());
    }

    buffer read(int len) const 
    {
        npl::buffer buf(len);
        int n = this->read(buf);
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

    std::ptrdiff_t
    sendto(const buffer& buf, const sockaddress<F>& remote, int flags = 0) const
    {
        return ::sendto(m_sockfd, &buf[0], buf.size(), flags, &remote.c_addr(), remote.len());
    }

    std::ptrdiff_t
    recvfrom(buffer& buf, sockaddress<F>& remote, int flags = 0) const
    {
        return ::recvfrom(m_sockfd, &buf[0], buf.size(), &remote.c_addr(), &remote.len());
    }

    std::pair<buffer,sockaddress<F>> 
    recvfrom(int len, int flags = 0) const
    {
        sockaddress<F> remote;
        buffer buf(len);
        int n = ::recvfrom(m_sockfd, &buf[0], buf.size(), flags, &remote.c_addr(), &remote.len());
        return std::make_pair(buffer(buf.begin(),buf.begin()+n),remote);
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



};



}  // End namespace npl




#endif