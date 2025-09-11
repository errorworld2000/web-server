#pragma once

#include <memory>

#include "util/timer.h"

#include "epoller.h"

class Channel;

class EventLoop : public std::enable_shared_from_this<EventLoop> {
 public:
  explicit EventLoop();
  ~EventLoop();
  void Loop();
  void Quit();
  void AddChannel(std::shared_ptr<Channel> channel, int timeout = -1);
  void ModChannel(std::shared_ptr<Channel> channel);
  void DelChannel(std::shared_ptr<Channel> channel);
  void GetActiveChannel();

 private:
  int wake_fd_;
  bool quit_;
  std::unique_ptr<Epoller> epoller_;
  std::unique_ptr<TimerManager> timer_manager_;
  std::vector<std::shared_ptr<Channel>> active_channels_;
  std::unordered_map<int, std::shared_ptr<Channel>> fd2channel_;

  void WakeUp();
  void HandleRead();
};