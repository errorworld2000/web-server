#include "event_loop_thread_pool.h"

#include <assert.h>
EventLoopThreadPool::EventLoopThreadPool(int thread_nums)
    : thraed_nums_(thread_nums), next_(0), started_(false) {
  assert(thraed_nums_ > 0);
}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::Start() {
  assert(!started_);
  started_ = true;
  for (int i = 0; i < thraed_nums_; i++) {
    std::string name = "loopthread" + std::to_string(i);
    auto t = std::make_shared<EventLoopThread>(name);
    threads_.push_back(t);
    loops_.push_back(t->StartLoop());
  }
}

std::shared_ptr<EventLoop> EventLoopThreadPool::GetNextLoop() {
  assert(started_);
  std::shared_ptr<EventLoop> loop = loops_[next_];
  next_ = (next_ + 1) % thraed_nums_;
  return loop;
}
