#pragma once
#include <memory>
#include <netinet/in.h>

#include "http_request.h"
#include "http_response.h"
#include "http_router.h"
#include "protocol_handler.h"

class HttpConn : public ProtocolHandler {
 public:
  static std::string src_dir;
  HttpConn(int fd, const sockaddr_in& addr, std::shared_ptr<EventLoop> loop);
  ~HttpConn();

 protected:
  void HandleRead() override;
  void HandleWrite() override;

 private:
 std::shared_ptr<EventLoop> loop_;
  bool is_et_;
  std::vector<char> read_buff_;
  std::vector<char> write_buff_;
  HttpRequest request_;
  HttpRouter router_;

  int iovCnt_;
  iovec iov_[2];

  void Error();
  void Close();
  bool OnProcess();
};
