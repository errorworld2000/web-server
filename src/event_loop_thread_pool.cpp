#include "event_loop_thread_pool.h"

#include <assert.h>
EventLoopThreadPool::EventLoopThreadPool(std::shared_ptr<EventLoop> base_loop,
                                         int thread_nums)
    : thraed_nums_(thread_nums),
      next_(0),
      base_loop_(std::move(base_loop)),
      started_(false) {
  assert(thraed_nums_ > 0);
}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::Start() {
  assert(!started_);
  started_ = true;
  for (int i = 0; i < thraed_nums_; i++) {
    std::shared_ptr<EventLoopThread> t(new EventLoopThread());
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
