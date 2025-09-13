#pragma once

#include <bits/std_thread.h>
#include <memory>
#include <mutex>

#include "util/timer.h"

#include "epoller.h"

class Channel;

class EventLoop : public std::enable_shared_from_this<EventLoop> {
 public:
  explicit EventLoop();
  ~EventLoop();
  void Loop();
  void Quit();
  void AddChannel(std::shared_ptr<Channel> channel, int timeout,
                  std::function<void()> cb);
  void ModChannel(std::shared_ptr<Channel> channel);
  void DelChannel(std::shared_ptr<Channel> channel);
  void GetActiveChannel();

 private:
  int wake_fd_;
  bool quit_;
  std::mutex mutex_;
  std::unique_ptr<Epoller> epoller_;
  std::unique_ptr<TimerManager> timer_manager_;
  std::vector<std::shared_ptr<Channel>> active_channels_;
  std::unordered_map<int, std::shared_ptr<Channel>> fd2channel_;

  std::thread::id thread_id_;
  std::vector<std::function<void()>> pending_functors_;
  bool calling_pending_functors_;

  void WakeUp();
  void HandleRead();
  void DoPendingFunctors();
  void RunInLoop(std::function<void()>&& cb);
  void QueueInLoop(std::function<void()>&& cb);
  bool IsInLoopThread();
};