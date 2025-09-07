#include "http_conn.h"

#include <sys/epoll.h>
#include <sys/uio.h>
#include <unistd.h>

std::string HttpConn::src_dir = "./resources";

HttpConn::HttpConn(int fd, const sockaddr_in& addr,
                   std::shared_ptr<EventLoop> loop)
    : ProtocolHandler(fd, addr, loop), is_et_(true) {
  channel_->SetReadHandler([this] { HandleRead(); });
  channel_->SetWriteHandler([this] { HandleWrite(); });
  channel_->SetEvents(EPOLLIN | EPOLLET);
}

HttpConn::~HttpConn() {
  if (fd_ > 0) close(fd_);
}

void HttpConn::HandleRead() {
  char buf[4096];
  ssize_t n = 0;
  do {
    n = read(fd_, buf, sizeof(buf));
    if (n > 0) {
      read_buff_.insert(read_buff_.end(), buf, buf + n);
    } else if (n == 0) {
      Close();
      break;
    } else {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        break;
      else
        Error();
      break;
    }
  } while (is_et_);

  if (OnProcess()) {
    channel_->SetEvents(EPOLLOUT | EPOLLET);
    
  } else {
    channel_->SetEvents(EPOLLIN | EPOLLET);
  }
}

void HttpConn::HandleWrite() {
  do {
    ssize_t total = 0;
    for (int i = 0; i < iovCnt_; i++) {
      total += iov_[i].iov_len;
    }
    ssize_t n = writev(fd_, iov_, iovCnt_);
    if (n < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        break;
      else
        Error();
      break;
    } else if (n == 0) {
      Error();
      return;
    } else {
      if (n == total) {
        if (request_.IsKeepAlive()) {
          channel_->SetEvents(EPOLLIN | EPOLLET);
        } else {
          channel_->DelChannel();
        }
      } else if (n > iov_[0].iov_len) {
        iov_[1].iov_base = (char*)iov_[1].iov_base + (n - iov_[0].iov_len);
        iov_[1].iov_len -= (n - iov_[0].iov_len);
        if (iov_[0].iov_len != 0) {
          iov_[0].iov_len = 0;
        }
      } else {
        iov_[0].iov_base = (char*)iov_[0].iov_base + n;
        iov_[0].iov_len -= n;
        channel_->SetEvents(EPOLLOUT | EPOLLET);
      }
    }
  } while (is_et_);
}

void HttpConn::Error() { perror("error"); }

void HttpConn::Close() { channel_->DelChannel(); }

bool HttpConn::OnProcess() {
  if (!request_.Parse(read_buff_)) {
    return false;
  }

  HttpResponse resp;
  router_.Route(request_, resp, src_dir);
  resp.Serialize(write_buff_);

  iov_[0].iov_base = const_cast<char*>(write_buff_.data());
  iov_[0].iov_len = write_buff_.size();

  if (resp.GetFileLen() > 0 && resp.GetFile()) {
    iov_[1].iov_base = resp.GetFile();
    iov_[1].iov_len = resp.GetFileLen();
    iovCnt_ = 2;
  }
  return true;
}
