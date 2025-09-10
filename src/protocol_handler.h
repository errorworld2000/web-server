#pragma once

#include <memory>
#include <netinet/in.h>

class ProtocolHandler : public std::enable_shared_from_this<ProtocolHandler> {
 public:
  ProtocolHandler(int fd, const sockaddr_in& addr, int timeout)
      : fd_(fd), addr_(addr), timeout_(timeout) {}
  virtual ~ProtocolHandler() = default;

 protected:
  virtual void HandleRead() = 0;
  virtual void HandleWrite() = 0;

  int fd_;
  sockaddr_in addr_;
  int timeout_;
};
