#pragma once

#include <condition_variable>
#include <thread>

#include "event_loop.h"
class EventLoopThread {
 public:
  EventLoopThread();
  ~EventLoopThread();
  std::shared_ptr<EventLoop> StartLoop();
  EventLoopThread(const EventLoopThread&) = delete;
  EventLoopThread& operator=(const EventLoopThread&) = delete;

 private:
  bool started_;
  std::shared_ptr<EventLoop> loop_;
  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cond_;

  void ThreadFunction();
};