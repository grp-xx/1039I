#ifndef _SOCKET_HPP_
#define _SOCKET_HPP_

#include <sys/socket.h>
#include <unistd.h>
#include "sockaddress.hpp"
#include <vector>


namespace npl {

typedef std::vector<char> buffer;

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
        if (this != rhs)
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

    int bind(const sockaddress<F>& addr)
    {
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

    std::ptrdiff_t
    sendto(const buffer& buf, const sockaddress<F>& remote, int flags = 0) const
    {
        return ::sendto(m_sockfd, &buf[0], buf.size(), &remote.c_addr(), remote.len());
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
        int n = ::recvfrom(m_sockfd, &buf[0], buf.size(), &remote.c_addr(), &remote.len());
        return std::make_pair(buffer(buf.begin(),buf.begin()+n),remote);
    }




};



}  // End namespace npl




#endif