#pragma once

#include <memory>
#include <vector>

#include "event_loop_thread.h"
class EventLoopThreadPool {
 public:
  EventLoopThreadPool(int thread_nums);
  ~EventLoopThreadPool();

  EventLoopThreadPool(const EventLoopThreadPool&) = delete;
  EventLoopThreadPool& operator=(const EventLoopThreadPool&) = delete;

  void Start();
  std::shared_ptr<EventLoop> GetNextLoop();

 private:
  int next_;
  bool started_;
  int thraed_nums_;
  std::vector<std::shared_ptr<EventLoopThread>> threads_;
  std::vector<std::shared_ptr<EventLoop>> loops_;
};