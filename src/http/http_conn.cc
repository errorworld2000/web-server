#include "http_conn.h"

#include <sys/epoll.h>
#include <sys/uio.h>
#include <unistd.h>

std::string HttpConn::src_dir_ = "../resources";

HttpConn::HttpConn(int fd, const sockaddr_in& addr,
                   std::shared_ptr<EventLoop> loop, int timeout)
    : ProtocolHandler(fd, addr, timeout),
      is_et_(true),
      loop_(loop),
      channel_(std::make_shared<Channel>(fd_)) {
  channel_->SetReadHandler([this] { HandleRead(); });
  channel_->SetWriteHandler([this] { HandleWrite(); });
  channel_->SetEvents(EPOLLIN | EPOLLET);
  loop_->AddChannel(channel_, timeout_);
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
      return;
    } else {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      } else {
        Error();
        return;
      }
    }
  } while (is_et_);

  if (OnProcess()) {
    channel_->SetEvents(EPOLLOUT | EPOLLET);
    loop_->ModChannel(channel_);

  } else {
    channel_->SetEvents(EPOLLIN | EPOLLET);
    loop_->ModChannel(channel_);
  }
}

void HttpConn::HandleWrite() {
  do {
    ssize_t total = 0;
    for (int i = 0; i < iov_cnt_; i++) {
      total += iov_[i].iov_len;
    }
    ssize_t n = writev(fd_, iov_, iov_cnt_);
    if (n < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      } else {
        Error();
        return;
      }
    } else if (n == 0) {
      Error();
      return;
    } else {
      if (n == total) {
        if (request_.IsKeepAlive()) {
          channel_->SetEvents(EPOLLIN | EPOLLET);
          loop_->ModChannel(channel_);
        } else {
          loop_->DelChannel(channel_);
        }

        return;
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

void HttpConn::Error() {
  perror("error");
  Close();
}

void HttpConn::Close() { loop_->DelChannel(channel_); }

bool HttpConn::OnProcess() {
  request_.Clear();
  response_.Clear();

  if (!request_.Parse(read_buff_)) {
    return false;
  }

  router_.Route(request_, response_, src_dir_);
  response_.Serialize(write_buff_);

  iov_[0].iov_base = const_cast<char*>(write_buff_.data());
  iov_[0].iov_len = write_buff_.size();
  iov_cnt_ = 1;

  if (response_.GetFileLen() > 0 && response_.GetFile()) {
    iov_[1].iov_base = response_.GetFile();
    iov_[1].iov_len = response_.GetFileLen();
    iov_cnt_ = 2;
  }
  return true;
}
