#pragma once

#include <memory>
#include <netinet/in.h>

#include "channel.h"
#include "event_loop.h"
#include "http_request.h"
#include "http_response.h"
#include "http_router.h"
#include "protocol_handler.h"

class HttpConn : public ProtocolHandler {
 public:
  HttpConn(int fd, const sockaddr_in& addr, std::shared_ptr<EventLoop> loop,
           int timeout);
  ~HttpConn();

 protected:
  void HandleRead() override;
  void HandleWrite() override;

 private:
  static std::string src_dir_;

  std::shared_ptr<Channel> channel_;
  std::shared_ptr<EventLoop> loop_;
  bool is_et_;
  std::vector<char> read_buff_;
  std::vector<char> write_buff_;
  HttpRequest request_;
  HttpResponse response_;
  HttpRouter router_;

  int iov_cnt_;
  iovec iov_[2];

  void Error();
  void Close();
  bool OnProcess();
};
