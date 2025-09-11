#include "channel.h"

#include <sys/epoll.h>

void Channel::OnEvents() {
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    OnClose();
  }
  if (revents_ & EPOLLERR) {
    OnError();
  }
  if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    OnRead();
  }
  if (revents_ & EPOLLOUT) {
    OnWrite();
  }

  revents_ = 0;
}