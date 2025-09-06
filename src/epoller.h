#pragma once
#include <memory>
#include <sys/epoll.h>
#include <vector>

#include "channel.h"

class Epoller {
 public:
  explicit Epoller(int max_event = 8096);
  ~Epoller();

  bool AddFd(int fd, uint32_t events);
  bool ModFd(int fd, uint32_t events);
  bool DelFd(int fd);
  int WaitEvents(int timeout);
  int GetEventFd(size_t i) const;
  uint32_t GetEvents(size_t i) const;

 private:
  int epoller_fd_;
  std::vector<epoll_event> events_;
};