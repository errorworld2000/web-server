#include "http_conn.h"

#include <sys/epoll.h>
#include <sys/uio.h>
#include <unistd.h>

HttpConn::HttpConn(int fd, const sockaddr_in& addr,
                   std::shared_ptr<EventLoop> loop)
    : fd_(fd), addr_(addr), channel_(std::make_shared<Channel>(loop, fd)) {
  channel_->SetReadHandler([this] { HandleRead(); });
  channel_->SetWriteHandler([this] { HandleWrite(); });
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
    } else {
      if (n < 0) read_errno_ = errno;
      break;
    }
  } while (is_et_);
  if (OnProcess()) {
    channel_->SetEvents(EPOLLOUT);
  } else {
    channel_->SetEvents(EPOLLIN);
  }
}

void HttpConn::HandleWrite() {
  ssize_t n = writev(fd_, iov_, iovCnt_);
  if (n > 0) {
    ssize_t total = 0;
    for (int i = 0; i < iovCnt_; i++) {
      total += iov_[i].iov_len;
    }
    if (n == total) {
      if (request_.IsKeepAlive()) {
        channel_->SetEvents(EPOLLIN);
      } else {
        channel_->DelChannel();
      }
    } else {
      iov_[0].iov_base = (char*)iov_[0].iov_base + n;
      iov_[0].iov_len -= n;
      channel_->SetEvents(EPOLLOUT);
    }
  }
}

void HttpConn::HandleError() {}

void HttpConn::HandleConn() {}

bool HttpConn::OnProcess() {
  if (!request_.Parse(read_buff_) && request_.IsValid()) return false;
  if (request_.IsValid())
    response_.Init(src_dir, request_.Path(), request_.IsKeepAlive(), 200);
  else
    response_.Init(src_dir, request_.Path(), false, 400);

  response_.Serialize(write_buff_);

  iov_[0].iov_base = const_cast<char*>(write_buff_.data());
  iov_[0].iov_len = write_buff_.size();

  if (response_.GetFileLen() > 0 && response_.GetFile()) {
    iov_[1].iov_base = response_.GetFile();
    iov_[1].iov_len = response_.GetFileLen();
    iovCnt_ = 2;
  }
  return true;
}
