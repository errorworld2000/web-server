#include "event_loop_thread.h"

#include <assert.h>

EventLoopThread::EventLoopThread() : loop_(nullptr) {}

EventLoopThread::~EventLoopThread() {
  if (loop_ != nullptr) {
    loop_->Quit();
    thread_.join();
  }
}

std::shared_ptr<EventLoop> EventLoopThread::StartLoop() {
  assert(!started_);
  started_ = true;
  thread_ = std::thread(&EventLoopThread::ThreadFunction, this);
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return loop_ != nullptr; });
  }
  return loop_;
}

void EventLoopThread::ThreadFunction() {
  auto loop = std::make_shared<EventLoop>();
  {
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = loop;
    cond_.notify_one();
  }
  loop->Loop();
  {
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
  }
}
