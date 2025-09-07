#include "server.h"

#include <assert.h>
#include <cstring>
#include <functional>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include "fcntl.h"

Server::Server(int port, int thread_nums, int timeout)
    : port_(port),
      listen_fd_(SocketBindListen(port_)),
      base_loop_(std::make_shared<EventLoop>()),
      event_loop_thread_pool_(
          std::make_unique<EventLoopThreadPool>(base_loop_, thread_nums)),
      timeout_(timeout) {
  accept_channel_ = std::make_shared<Channel>(base_loop_, listen_fd_);
  accept_channel_->SetEvents(EPOLLIN | EPOLLET);
  accept_channel_->SetReadHandler(std::bind(&Server::HandlerNewConn, this));

  base_loop_->AddChannel(accept_channel_, timeout_);
}

void Server::Start() {
  event_loop_thread_pool_->Start();
  base_loop_->Loop();
}

int Server::SocketBindListen(int port) {
  if (port < 0 || port > 65535) return -1;
  int listen_fd = -1;
  if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;
  int optval = 1;
  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval,
                 sizeof(optval)) == -1) {
    close(listen_fd);
    return -1;
  }

  sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(port);

  if (bind(listen_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
    close(listen_fd);
    return -1;
  }

  if (listen(listen_fd, 2048) == -1) {
    close(listen_fd);
    return -1;
  }
  SetFdNonBloack(listen_fd);  // Add this line
  return listen_fd;
}

void Server::HandlerNewConn() {
  sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(sockaddr_in));
  socklen_t client_addr_len = sizeof(client_addr);
  int accept_fd = -1;

  while ((accept_fd = accept(listen_fd_, (sockaddr*)&client_addr,
                             &client_addr_len)) > 0) {
    std::shared_ptr<EventLoop> loop = event_loop_thread_pool_->GetNextLoop();
    // TODO: log
    if (accept_fd > MAXFDS) {
      close(accept_fd);
      continue;
    }

    if (SetFdNonBloack(accept_fd) < 0) {
      perror("Set fd non block failed!");
      return;
    }
    SetSocketNodelay(accept_fd);
    // std::shared_ptr<Channel> channel = std::make_shared<Channel>(accept_fd);
    user_[accept_fd] = std::make_shared<HttpConn>(accept_fd, client_addr, loop);

    loop->AddChannel(user_[accept_fd]->GetChannel(), timeout_);
  }
}

int Server::SetFdNonBloack(int fd) {
  assert(fd > 0);
  return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

void Server::SetSocketNodelay(int fd) {
  int enable = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
}
