#include "event_loop.h"

#include <stdio.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "channel.h"

EventLoop::EventLoop()
    : epoller_(new Epoller()),
      quit_(false),
      timer_manager_(std::make_unique<TimerManager>()),
      thread_id_(std::this_thread::get_id()) {
  wake_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (wake_fd_ < 0) {
    perror("create wake-fd error");
    abort();
  }
  std::shared_ptr<Channel> wake_ch = std::make_shared<Channel>(wake_fd_);
  wake_ch->SetEvents(EPOLLIN | EPOLLET);
  wake_ch->SetReadHandler([this] { HandleRead(); });
  AddChannel(wake_ch, -1, [this, wake_ch] { DelChannel(wake_ch); });
}

EventLoop::~EventLoop() {}

void EventLoop::Loop() {
  while (!quit_) {
    active_channels_.clear();
    GetActiveChannel();
    for (auto& it : active_channels_) it->OnEvents();
    timer_manager_->Tick();
    DoPendingFunctors();
  }
}

void EventLoop::Quit() { quit_ = true; }

void EventLoop::AddChannel(std::shared_ptr<Channel> channel, int timeout,
                           std::function<void()> cb) {
  RunInLoop([this, channel, timeout, cb] {
    if (epoller_->AddFd(channel->GetFd(), channel->GetEvents()) < 0) {
      perror("epoll_add error!");
      return;
    }
    fd2channel_[channel->GetFd()] = channel;
    if (timeout > 0) {
      timer_manager_->AddTimer(channel->GetFd(), timeout, std::move(cb));
    }
  });
}

void EventLoop::ModChannel(std::shared_ptr<Channel> channel) {
  RunInLoop([this, channel] {
    if (!epoller_->ModFd(channel->GetFd(), channel->GetEvents())) {
      perror("error_mod error!");
    }
  });
}

void EventLoop::DelChannel(std::shared_ptr<Channel> channel) {
  RunInLoop([this, channel] {
    if (!epoller_->DelFd(channel->GetFd())) {
      perror("error_del error!");
    }
    fd2channel_.erase(channel->GetFd());
    timer_manager_->DelTimer(channel->GetFd());
  });
}

void EventLoop::GetActiveChannel() {
  int timeout = timer_manager_->GetNextTick();
  int event_count = epoller_->WaitEvents(timeout);
  for (int i = 0; i < event_count; i++) {
    int fd = epoller_->GetEventFd(i);
    uint32_t events = epoller_->GetEvents(i);

    auto it = fd2channel_.find(fd);
    if (it != fd2channel_.end()) {
      auto channel = it->second;
      channel->SetRevents(events);
      active_channels_.push_back(channel);
    }
  }
}

void EventLoop::WakeUp() {
  uint64_t one = 1;
  ssize_t n = write(wake_fd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    perror("wake-eventfd write failed");
  }
}

void EventLoop::HandleRead() {
  uint64_t cnt;
  ssize_t n = read(wake_fd_, &cnt, sizeof(cnt));
  if (n != sizeof(cnt)) {
    perror("wake-eventfd read failed");
  }
}

void EventLoop::DoPendingFunctors() {
  std::vector<std::function<void()>> functors;
  calling_pending_functors_ = true;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    functors.swap(pending_functors_);
  }
  for (auto& f : functors) f();
  calling_pending_functors_ = false;
}

void EventLoop::RunInLoop(std::function<void()>&& cb) {
  if (IsInLoopThread())
    cb();
  else
    QueueInLoop(std::move(cb));
}

void EventLoop::QueueInLoop(std::function<void()>&& cb) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pending_functors_.emplace_back(std::move(cb));
  }
  if (!IsInLoopThread() || calling_pending_functors_) WakeUp();
}

bool EventLoop::IsInLoopThread() {
  return std::this_thread::get_id() == thread_id_;
}
