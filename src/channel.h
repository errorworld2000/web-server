#pragma once

#include <functional>
#include <memory>

class Channel : public std::enable_shared_from_this<Channel> {
 public:
  Channel(int fd) : fd_(fd), events_(0) {}
  int GetFd() { return fd_; }
  void SetReadHandler(std::function<void()> headler) {
    read_handler_ = std::move(headler);
  }
  void SetWriteHandler(std::function<void()> handler) {
    write_handler_ = std::move(handler);
  }
  void SetErrorHandler(std::function<void()> handler) {
    error_handler_ = std::move(handler);
  }
  void SetConnHandler(std::function<void()> handler) {
    conn_handler_ = std::move(handler);
  }
  void SetEvents(uint32_t ev) { events_ = ev; }
  void SetRevents(uint32_t ev) { revents_ = ev; }
  uint32_t GetEvents() { return events_; }

  void OnEvents();

 private:
  int fd_;
  uint32_t events_;
  uint32_t revents_;
  std::function<void()> read_handler_;
  std::function<void()> write_handler_;
  std::function<void()> error_handler_;
  std::function<void()> conn_handler_;
  void OnRead() {
    if (read_handler_) read_handler_();
  }
  void OnWrite() {
    if (write_handler_) write_handler_();
  }
  void OnError() {
    if (error_handler_) error_handler_();
  }
  void OnConn() {
    if (conn_handler_) conn_handler_();
  }
};