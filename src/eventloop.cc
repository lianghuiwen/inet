#include "eventloop.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include "logging.h"
#include "tcpsocket.h"
#include "channel.h"

using namespace xxbc;

namespace inet {

extern ChannelMap g_channel_map;

EventLoop::EventLoop() : fdqueue_(), mempool_() {
    epollfd_ = epoll_create(1);
    if (epollfd_ < 0) {
        LOG(FATAL) << "epoll_create fail!";
    }

    int fds[2];
    if (pipe(fds) == 0) {
        notify_send_fd_ = fds[1];
        notify_recv_fd_ = fds[0];
    } else {
        LOG(FATAL) << "pipe fail!";
    }

    struct epoll_event event;
    event.data.ptr = &notify_recv_fd_;
    event.events = EPOLLIN;
    int ret = epoll_ctl(epollfd_, EPOLL_CTL_ADD, notify_recv_fd_, &event);
    if (ret < 0) {
        LOG(FATAL) << "epoll_ctl fail!"; 
    }
}

EventLoop::~EventLoop() {
    close(epollfd_);
    close(notify_send_fd_);
    close(notify_recv_fd_);
}

void EventLoop::Assign(int sockfd) {
    fdqueue_.push(sockfd);
    int count = write(notify_send_fd_, "C", 1);
    if (count <= 0) {
        LOG(FATAL) << "write to pipe fail!"; 
    }
}

void EventLoop::Stop() {
    int count = write(notify_send_fd_, "Q", 1);
    if (count <= 0) {
        LOG(FATAL) << "write to pipe fail!"; 
    }
}

void EventLoop::Run() {
    const int kMaxEvent = 256;
    struct epoll_event events[kMaxEvent];

    for (;;) {
        int num_events = epoll_wait(epollfd_, events, kMaxEvent, -1);
        if (num_events > 0) {
            for (int i=0; i<num_events; ++i) {
                // TODO error event
                if (events[i].data.ptr == &notify_recv_fd_) {
                    HandleCommand();        
                } else {
                    Channel* channel = reinterpret_cast<Channel*>(events[i].data.ptr);
                    HandleMessage(channel);
                }
            }
        } else if (num_events == 0) {
            // nothing happended;
        } else {
            LOG(FATAL) << "epoll_wait fail!";
            break;
        }
    }
}

void EventLoop::HandleMessage(Channel* channel) {
    char buf[1024];
    int count = channel->socket->Read(buf, sizeof(buf));
    if (count > 0) {
        channel->socket->Write(buf, count);
    } else if (count == 0) {
        LOG(INFO) << "socket close: " << channel->socket->Sockfd();
        epoll_ctl(epollfd_, EPOLL_CTL_DEL, channel->socket->Sockfd(), NULL);
        ReleaseChannel(channel);
    } else {
        if (errno != EAGAIN) {
            LOG(ERROR) << "socket error: " << channel->socket->Sockfd();
            epoll_ctl(epollfd_, EPOLL_CTL_DEL, channel->socket->Sockfd(), NULL);
            ReleaseChannel(channel);
        }
    }
}

void EventLoop::HandleCommand() {
    char cmd;
    int count = read(notify_recv_fd_, &cmd, 1);
    if (count > 0) {
        switch (cmd) {
        case 'C':
            ExeCommandC();
            break;
        case 'Q':
            ExeCommandQ();
            break;
        default:
            LOG(WARN) << "unknow command: " << cmd;
            break;
        }
    } else {
        LOG(FATAL) << "pipe read fail!";
    }
}

void EventLoop::ExeCommandC() {
    if (!fdqueue_.empty()) {
        int sockfd = fdqueue_.front();
        fdqueue_.pop();

        Channel* channel = new Channel();
        channel->socket = new TcpSocket(sockfd);
        channel->buffer = mempool_.Malloc();

        struct epoll_event event;
        event.data.ptr = channel;
        event.events = EPOLLIN;
        int ret = epoll_ctl(epollfd_, EPOLL_CTL_ADD, sockfd, &event);
        if (ret < 0) {
            LOG(ERROR) << "epoll_ctl fail!";
            ReleaseChannel(channel);
        }
    }
}

void EventLoop::ExeCommandQ() {

}

void EventLoop::ReleaseChannel(Channel* channel) {
    channel->socket->Close();
    delete channel->socket;
    mempool_.Free(channel->buffer);
    delete channel;
}

} // namespace inet
