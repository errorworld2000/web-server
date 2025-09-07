#include "channel.h"

#include <sys/epoll.h>

#include "event_loop.h"

void Channel::SetEvents(uint32_t ev) {
  events_ = ev;
  if (auto loop = loop_.lock()) loop->ModChannel(shared_from_this());
}

void Channel::DelChannel() {
  if (auto loop = loop_.lock()) loop->DelChannel(shared_from_this());
}

void Channel::OnEvents() {
  events_ = 0;
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    events_ = 0;
    return;
  }
  if (revents_ & EPOLLERR) {
    OnError();
    events_ = 0;
    return;
  }
  if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    OnRead();
  }
  if (revents_ & EPOLLOUT) {
    OnWrite();
  }
  OnConn();
}