#ifndef INET_CHANNEL_H_
#define INET_CHANNEL_H_

#include <map>
#include <queue>
#include <string>
#include "mutexlock.h"
#include "mempool.h"
#include "tcpsocket.h"

using namespace std;
using namespace xxbc;

namespace inet {

struct Channel {
    string id;
    TcpSocket* socket;
    InetAddress* addr;
    Chunk* buffer;
};


class ChannelMap {
public:
    ChannelMap() : mutex_(), channels_() { }

    void Insert(Channel* channel) {
        MutexLockGuard lock(mutex_);
        channels_[channel->id] = channel;
    }

    void Erase(Channel* channel) {
        MutexLockGuard lock(mutex_);
        channels_.erase(channel->id);
    }

    Channel* Get(const string& id) {
        MutexLockGuard lock(mutex_);
        map<string, Channel*>::iterator it = channels_.find(id);
        if (it != channels_.end()) {
            return it->second;
        }
        return NULL;
    }

    // Channel* GetAndLock(const string& id) {
    //     MutexLockGuard lock(mutex_);
    //     map<string, Channel*>::iterator it = channels_.find(id);
    //     if (it != channels_.end()) {
    //         it->second->Lock();
    //         return it->second;
    //     }
    //     return NULL;
    // }

private:
    Mutex mutex_;
    map<string, Channel*> channels_;
};


class ChannelQueue {
public:
    struct ChannelNode {
        Channel* channel;
        ChannelNode* next;
    };

    ChannelQueue() : mutex_(), head_(NULL), tail_(NULL) { }

    void Push(Channel* channel) {
        ChannelNode* node = new ChannelNode();
        node->channel = channel;
        node->next = NULL;

        MutexLockGuard lock(mutex_);
        if (tail_) {
            tail_ = tail_->next = node;
        } else {
            tail_ = head_ = node;
        }
    }

    Channel* Pop() {
        Channel* channel = NULL;
        MutexLockGuard lock(mutex_);
        if (head_) {
            ChannelNode* node = head_;
            head_ = head_->next;

            channel = node->channel;
            delete node;
        }
        return channel;
    }

private:
    Mutex mutex_;
    ChannelNode* head_;
    ChannelNode* tail_;
};

} // namespace inet

#endif
