#include "event_loop.h"

#include <stdio.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "channel.h"

EventLoop::EventLoop()
    : epoller_(new Epoller()),
      quit_(false),
      timer_manager_(std::make_unique<TimerManager>()) {
  wake_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (wake_fd_ < 0) {
    perror("create wake-fd error");
    abort();
  }
  std::shared_ptr<Channel> wake_ch = std::make_shared<Channel>(wake_fd_);
  wake_ch->SetEvents(EPOLLIN | EPOLLET);
  wake_ch->SetReadHandler([this] { HandleRead(); });
  AddChannel(wake_ch);
}

EventLoop::~EventLoop() {}

void EventLoop::Loop() {
  while (!quit_) {
    active_channels_.clear();
    GetActiveChannel();
    for (auto& it : active_channels_) it->OnEvents();
    timer_manager_->Tick();
  }
}

void EventLoop::Quit() {}

void EventLoop::AddChannel(std::shared_ptr<Channel> channel, int timeout) {
  if (epoller_->AddFd(channel->GetFd(), channel->GetEvents()) < 0) {
    perror("epoll_add error!");
    return;
  }
  fd2channel_[channel->GetFd()] = channel;
  if (timeout > 0) {
    timer_manager_->AddTimer(channel->GetFd(), timeout,
                             [this, channel]() { this->DelChannel(channel); });
  }
  WakeUp();
}

void EventLoop::ModChannel(std::shared_ptr<Channel> channel) {
  if (!epoller_->ModFd(channel->GetFd(), channel->GetEvents())) {
    perror("error_mod error!");
  }
  WakeUp();
}

void EventLoop::DelChannel(std::shared_ptr<Channel> channel) {
  if (!epoller_->DelFd(channel->GetFd())) {
    perror("error_del error!");
  }
  fd2channel_.erase(channel->GetFd());
  timer_manager_->DelTimer(channel->GetFd());
  WakeUp();
}

void EventLoop::GetActiveChannel() {
  int timeout = timer_manager_->GetNextTick();
  int event_count = epoller_->WaitEvents(timeout);
  for (int i = 0; i < event_count; i++) {
    int fd = epoller_->GetEventFd(i);
    uint32_t events = epoller_->GetEvents(i);
    std::shared_ptr<Channel> channel = fd2channel_[fd];
    channel->SetRevents(events);
    active_channels_.push_back(channel);
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