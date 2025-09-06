#pragma once

#include <memory>

#include "channel.h"
#include "event_loop.h"
#include "event_loop_thread_pool.h"
#include "http_conn.h"

static const int MAXFDS = 100000;

class Server {
 public:
  Server(int port, int thread_nums, int timeout);
  void Start();

 private:
  // std::unique_ptr<EventLoop> event_loop_;
  int timeout_;
  int port_;
  int listen_fd_;
  std::shared_ptr<Channel> accept_channel_;
  std::unique_ptr<EventLoopThreadPool> event_loop_thread_pool_;
  std::unordered_map<int, std::shared_ptr<HttpConn>> user_;

  int SocketBindListen(int port);
  void HandlerNewConn();
  int SetFdNonBloack(int fd);
  void SetSocketNodelay(int fd);
};