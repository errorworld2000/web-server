#pragma once

#include <memory>
#include <netinet/in.h>

#include "channel.h"
#include "event_loop.h"

class ProtocolHandler : public std::enable_shared_from_this<ProtocolHandler> {
 public:
  ProtocolHandler(int fd, const sockaddr_in& addr,
                  std::shared_ptr<EventLoop> loop)
      : fd_(fd), addr_(addr), channel_(std::make_shared<Channel>(loop, fd)) {}
  virtual ~ProtocolHandler() = default;

  std::shared_ptr<Channel> GetChannel() { return channel_; }

 protected:
  virtual void HandleRead() = 0;
  virtual void HandleWrite() = 0;

  int fd_;
  sockaddr_in addr_;
  std::shared_ptr<Channel> channel_;
};
