#ifdef __linux__

#include <iostream>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "frame.hpp"
#include "socket.hpp"
#include "sockaddress.hpp"
#include <netinet/ip.h>
#include <thread>
#include <vector>
#include "headers.hpp"
#include "frame.hpp"


void frame_capture(std::string iface, unsigned short protocol)
{
    npl::sockaddress<AF_PACKET> device(iface, protocol);
    npl::socket<AF_PACKET, SOCK_RAW> sock;
    sock.bind(device);
    sock.set_promisc(iface);
    sock.set_fanout(42);

    for (;;)
    {
        auto [buf, from] = sock.recvfrom(65536);
        npl::frame ff(&buf[0],buf.size());

        if (ff.is<ipv4>()) 
        {
            std::cout << "From: " << ff.header<ipv4>().src() << " --> To: " << ff.header<ipv4>().dst() << std::endl;
            std::cout << "Vlan ID: " << ff.header<vlan>().vlan_id() << std::endl;

        } 

    }

    sock.close();

}


int main(int argc, char** argv)
{
    const int N_thread = 4;
    std::vector<std::thread> thread_pool;

    for (auto i = 1; i <= N_thread; i++) {
        std::thread t(frame_capture, "ens33", ETH_P_ALL);
        thread_pool.push_back(std::move(t));
    }

    for (auto &x : thread_pool) {
        x.join();
    }


}

#endif