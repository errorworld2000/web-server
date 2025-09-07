#include "event_loop.h"

#include <stdio.h>

#include "channel.h"

EventLoop::EventLoop()
    : epoller_(new Epoller()),
      quit_(false),
      timer_manager_(std::make_unique<TimerManager>()) {}

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
}

void EventLoop::ModChannel(std::shared_ptr<Channel> channel) {
  if (!epoller_->ModFd(channel->GetFd(), channel->GetEvents())) {
    perror("error_mod error!");
  }
}

void EventLoop::DelChannel(std::shared_ptr<Channel> channel) {
  if (!epoller_->DelFd(channel->GetFd())) {
    perror("error_del error!");
  }
  fd2channel_.erase(channel->GetFd());
}

void EventLoop::GetActiveChannel() {
  int timeout = -1;
  timeout = timer_manager_->GetNextTick();
  if (timeout == -1) timeout = 1000;
  int event_count = epoller_->WaitEvents(timeout);
  for (int i = 0; i < event_count; i++) {
    int fd = epoller_->GetEventFd(i);
    uint32_t events = epoller_->GetEvents(i);
    std::shared_ptr<Channel> channel = fd2channel_[fd];
    channel->SetRevents(events);
    active_channels_.push_back(channel);
  }
}
