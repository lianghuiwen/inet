#ifndef INET_TCPSOCKET_H_
#define INET_TCPSOCKET_H_

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include "inetaddress.h"

namespace inet {

class TcpSocket {
public:
    explicit TcpSocket(int sockfd) : sockfd_(sockfd) { }

    ~TcpSocket() {
        if (sockfd_ >= 0) { Close(); }
    }

    static int CreateNew() {
        return socket(AF_INET, SOCK_STREAM, 0);
    }

    bool Valid() {
        return (sockfd_ >= 0);
    }

    int Bind(const InetAddress& addr) {
        return bind(sockfd_, (struct sockaddr*) &addr.addr_, 
            sizeof(addr.addr_));
    }

    int Listen(uint16_t backlog = 5) {
        return listen(sockfd_, backlog);
    }

    int Connect(const InetAddress& addr) {
        return connect(sockfd_, (struct sockaddr*) &addr.addr_, 
            sizeof(addr.addr_));
    }

    int Accept(InetAddress* peeraddr) {
        if (peeraddr) {
            struct sockaddr_in& addr = peeraddr->addr_;
            size_t addrlen = sizeof(peeraddr->addr_);
            return accept(sockfd_, (sockaddr*) &addr, (socklen_t*) &addrlen);
        } else {
            return accept(sockfd_, NULL, NULL);
        }
    }

    int Read(char buf[], size_t len) {
        return read(sockfd_, buf, len);
    }

    int Write(char buf[], size_t len) {
        return write(sockfd_, buf, len);
    }

    void Shutdown() {
        shutdown(sockfd_, SHUT_WR);
    }

    void Close() {
        close(sockfd_);
        sockfd_ = -1;
    }

    int Sockfd() { return sockfd_; }

private:
    int sockfd_;
};

} // namespace inet

#endif
