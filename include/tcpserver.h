#ifndef INET_TCPSERVER_H_
#define INET_TCPSERVER_H_

#include <vector>
#include "tcpsocket.h"

using namespace std;

namespace inet {

class InetAddress;
class EventLoop;

class TcpServer {
public:
    TcpServer(const InetAddress& addr);
    ~TcpServer();

    void Start();

private:
    void InitEventLoop(int count);

private:
    int index_;
    TcpSocket listensock_;
    InetAddress listenaddr_;
    vector<EventLoop*> loops_;
};

} // namespace inet

#endif
