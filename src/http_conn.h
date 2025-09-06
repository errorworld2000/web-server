#pragma once
#include <memory>
#include <netinet/in.h>

#include "channel.h"
#include "event_loop.h"
#include "http_request.h"
#include "http_response.h"

class HttpConn : public std::enable_shared_from_this<HttpConn> {
 public:
  static std::string src_dir;
  HttpConn(int fd, const sockaddr_in& addr, std::shared_ptr<EventLoop> loop);
  ~HttpConn();
  std::shared_ptr<Channel> GetChannel() { return channel_; }

 private:
  int fd_;
  sockaddr_in addr_;
  bool is_et_;
  std::shared_ptr<Channel> channel_;
  std::vector<char> read_buff_;
  std::vector<char> write_buff_;
  HttpRequest request_;
  HttpResponse response_;

  int read_errno_;

  int iovCnt_;
  iovec iov_[2];

  void HandleRead();
  void HandleWrite();
  void HandleError();
  void HandleConn();
  bool OnProcess();
};
