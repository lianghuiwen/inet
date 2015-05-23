#include "tcpserver.h"
#include <stdio.h>
#include "logging.h"
#include "inetaddress.h"
#include "channel.h"
#include "eventloop.h"

using namespace xxbc;

namespace inet {

ChannelMap g_channel_map;

TcpServer::TcpServer(const InetAddress& addr) : 
    index_(0), listensock_(TcpSocket::CreateNew()), listenaddr_(addr), loops_() {}

TcpServer::~TcpServer() {
    for (uint32_t i=0; i<loops_.size(); ++i) {
        delete loops_[i];
    }
    listensock_.Close();
}

void TcpServer::Start() {
    if (!listensock_.Valid()) {
        LOG(FATAL) << "socket create error!" << " errno: " << errno;
        return;
    }
    if (listensock_.Bind(listenaddr_) == -1) {
        LOG(FATAL) << "socket bind error!" << " errno: " << errno;
        return;
    }

    if (listensock_.Listen() == -1) {
        LOG(FATAL) << "socket listen error!" << " errno: " << errno;
        return;
    }

    LOG(INFO) << "start listen " << listenaddr_.Port();

    // one thread per loop
    const int loop_count = 5;
    InitEventLoop(loop_count);

    int peerfd;
    InetAddress peeraddr;
    for (;;) {
        peerfd = listensock_.Accept(&peeraddr);
        if (peerfd >= 0) {
            LOG(INFO) << peeraddr.Ip() << ":" << peeraddr.Port() << " connected: " << peerfd;
            // check thread
            if (!loops_[index_]->Alive()) {
                delete loops_[index_];
                loops_[index_] = new EventLoop();
                loops_[index_]->Start();
            }
            // round-robin assign
            loops_[index_]->Assign(peerfd);
            index_ = (++index_) % loops_.size();
        } else {
            switch (errno)
            {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                LOG(ERROR) << "socket accept fail!" << " errno: " << errno;
                break;
            default:
                LOG(FATAL) << "socket accept fail!" << " errno: " << errno;
                exit(0);
            }
        }
    }
}

void TcpServer::InitEventLoop(int count) {
    loops_.reserve(count);
    for (int i=0; i<count; ++i) {
        EventLoop* loop = new EventLoop();
        loop->Start();
        loops_.push_back(loop);
    }
}

} // namespace inet
