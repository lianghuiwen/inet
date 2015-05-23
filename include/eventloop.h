#ifndef INET_EVENTLOOP_H_
#define INET_EVENTLOOP_H_

#include <sys/epoll.h>
#include <queue>
#include "thread.h"
#include "mempool.h"

using namespace std;
using namespace xxbc;

namespace inet {

class Channel;

class EventLoop : public Thread {
public:
    EventLoop();
    virtual ~EventLoop();
    
    void Run();
    void Stop();
    void Assign(int sockfd);

private:
    void ReleaseChannel(Channel* channel);
    void HandleMessage(Channel* channel);
    void HandleCommand();
    void ExeCommandC();
    void ExeCommandQ();

private:
    int epollfd_;
    int notify_send_fd_;
    int notify_recv_fd_;
    queue<int> fdqueue_;
    MemoryPool mempool_;
};

} // namespace inet

#endif
