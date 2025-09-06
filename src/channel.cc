#include "channel.h"
#include<sys/epoll.h>

void Channel::DelChannel() {
  loop_->
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