#pragma once
#include <memory>

#include "epoller.h"
#include "timer.h"
class EventLoop {
 public:
  explicit EventLoop();
  ~EventLoop();
  void Loop();
  void Quit();
  void AddChannel(std::shared_ptr<Channel> channel, int timeout);
  void ModChannel(std::shared_ptr<Channel> channel, int timeout);
  void DelChannel(std::shared_ptr<Channel> channel);
  void GetActiveChannel();

 private:
  bool quit_;
  std::unique_ptr<Epoller> epoller_;
  std::unique_ptr<TimerManager> timer_manager_;
  std::vector<std::shared_ptr<Channel>> active_channels_;
};