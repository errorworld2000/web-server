#include "epoller.h"

#include <assert.h>
#include <cerrno>
#include <stdio.h>
#include <unistd.h>

Epoller::Epoller(int max_event)
    : epoller_fd_(epoll_create(512)), events_(max_event) {
  assert(epoller_fd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller() { close(epoller_fd_); }

bool Epoller::AddFd(int fd, uint32_t events) {
  if (fd < 0) return false;
  epoll_event ev = {0};
  ev.data.fd = fd;
  ev.events = events;
  return 0 == epoll_ctl(epoller_fd_, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::ModFd(int fd, uint32_t events) {
  if (fd < 0) return false;
  epoll_event ev = {0};
  ev.data.fd = fd;
  ev.events = events;
  return 0 == epoll_ctl(epoller_fd_, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::DelFd(int fd) {
  if (fd < 0) return false;
  epoll_event ev = {0};
  return 0 == epoll_ctl(epoller_fd_, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::WaitEvents(int timeoutMs) {
  int event_count;
  do {
    event_count = epoll_wait(epoller_fd_, &events_[0],
                             static_cast<int>(events_.size()), timeoutMs);
  } while (event_count < 0 && errno == EINTR);
  return event_count;
}

int Epoller::GetEventFd(size_t i) const {
  assert(i < events_.size() && i >= 0);
  return events_[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t i) const {
  assert(i < events_.size() && i >= 0);
  return events_[i].events;
}