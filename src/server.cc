#include "server.h"

#include <assert.h>
#include <cerrno>
#include <climits>
#include <cstring>
#include <functional>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include "fcntl.h"

static const int MAXFDS = 101000;

Server::Server(int port, int thread_nums, int timeout)
    : port_(port),
      listen_fd_(SocketBindListen(port_)),
      base_loop_(std::make_unique<EventLoop>()),
      event_loop_thread_pool_(
          std::make_unique<EventLoopThreadPool>(thread_nums)),
      timeout_(timeout) {
  accept_channel_ = std::make_shared<Channel>(listen_fd_);
  accept_channel_->SetEvents(EPOLLIN | EPOLLET);
  accept_channel_->SetReadHandler(std::bind(&Server::HandlerNewConn, this));
  base_loop_->AddChannel(accept_channel_, INT_MAX,
                         [this] { base_loop_->DelChannel(accept_channel_); });
}

Server::~Server() {
  base_loop_->DelChannel(accept_channel_);
  base_loop_->Quit();
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
  SetFdNonBlock(listen_fd);
  return listen_fd;
}

void Server::HandlerNewConn() {
  sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  while (true) {
    int accept_fd =
        accept(listen_fd_, (sockaddr*)&client_addr, &client_addr_len);
    if (accept_fd < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      } else if (errno == EINTR) {
        continue;
      } else {
        perror("accept error");
        break;
      }
    }

    if (accept_fd > MAXFDS) {
      perror("this fd is over MAXFD!");
      close(accept_fd);
      continue;
    }

    if (SetFdNonBlock(accept_fd) < 0) {
      perror("Set fd non block failed!");
      close(accept_fd);
      continue;
    }

    SetSocketNodelay(accept_fd);
    std::shared_ptr<EventLoop> loop = event_loop_thread_pool_->GetNextLoop();
    user_[accept_fd] =
        std::make_shared<HttpConn>(accept_fd, client_addr, loop, timeout_,
                                   [this](int fd) { user_[fd] = nullptr; });
  }
}

int Server::SetFdNonBlock(int fd) {
  assert(fd > 0);
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) return -1;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::SetSocketNodelay(int fd) {
  int enable = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
}
