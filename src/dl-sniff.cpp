#include <iostream>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "socket.hpp"
#include "sockaddress.hpp"
#include <netinet/ip.h>
#include <thread>
#include <vector>
#include "headers.hpp"


void frame_capture(std::string iface, unsigned short protocol)
{
    npl::sockaddress<AF_PACKET> device(iface, protocol);
    npl::socket<AF_PACKET, SOCK_DGRAM> sock;
    sock.bind(device);
    sock.set_promisc(iface);
    sock.set_fanout(42);

    for (;;)
    {
        auto [buf, from] = sock.recvfrom(65536);
        npl::pktheader<ipv4> ip_hdr(&buf[0],buf.size());
        std::cout << "Thread :" << std::this_thread::get_id() << " "; 
        std::cout << "From " << ip_hdr.src() << " To: " << ip_hdr.dst() << std::endl;
    }

    sock.close();

}


int main(int argc, char** argv)
{
    const int N_thread = 4;
    std::vector<std::thread> thread_pool;

    for (auto i = 1; i <= N_thread; i++) {
        std::thread t(frame_capture, "ens33", ETHERTYPE_IP);
        thread_pool.push_back(std::move(t));
    }

    for (auto &x : thread_pool) {
        x.join();
    }


}