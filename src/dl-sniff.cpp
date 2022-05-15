#include <iostream>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "socket.hpp"
#include "sockaddress.hpp"
#include <netinet/ip.h>
#include "pktheaders.hpp"


int main(int argc, char** argv)
{
    npl::socket<AF_PACKET, SOCK_DGRAM> sock(ntohs(ETH_P_ALL));


    for (;;)
    {
        // auto [buf, from] = sock.recvfrom(65536);

        auto buf = sock.read(65536);
        //const struct ip* ip_ptr = reinterpret_cast<const ip*>(&buf[0]);

        npl::pktheader<ipv4> ip_hdr(&buf[0],buf.size());
        std::cout << "Header length: " << ip_hdr.hlen() << " ";
        std::cout << "From " << ip_hdr.src() << " To: " << ip_hdr.dst() << std::endl;
        





    }

    sock.close();


}