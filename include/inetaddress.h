#ifndef INET_INETADDRESS_H_
#define INET_INETADDRESS_H_

#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

using namespace std;

namespace inet {

class TcpSocket;

class InetAddress 
{
public:
    friend class TcpSocket;
    
    InetAddress() : addr_() {
        bzero(&addr_, sizeof(addr_));
    }

    InetAddress(struct sockaddr_in& addr) : addr_(addr) {}

    InetAddress(uint16_t port) {
        bzero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    InetAddress(const char* ip, uint16_t port) {
        bzero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        inet_pton(AF_INET, ip, &addr_.sin_addr);
    }

    const struct sockaddr_in& SockAddr() const {
        return addr_;
    }

    string Ip() const {
        char buf[32] = {0};
        inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
        return buf;
    }
    
    uint16_t Port() const {
        return ntohs(addr_.sin_port);
    }

private:
    struct sockaddr_in addr_;
};

} // namespace inet

#endif
