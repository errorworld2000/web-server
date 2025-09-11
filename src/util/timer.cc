#include "timer.h"

#include <chrono>
#include <queue>
#include <unordered_map>

struct TimerManager::TimerNode {
  TimerNode(int id, int version, int timeout, std::function<void()> handler)
      : id(id), version(version), handler(std::move(handler)), deleted(false) {
    expires = std::chrono::high_resolution_clock::now() +
              std::chrono::milliseconds(timeout);
  }

  int id;
  int version;
  bool deleted;
  std::function<void()> handler;
  std::chrono::high_resolution_clock::time_point expires;
};

bool TimerManager::TimerCmp::operator()(const TimerNodePtr& a,
                                        const TimerNodePtr& b) const {
  return a->expires > b->expires;
}

void TimerManager::AddTimer(int id, int timeout,
                            std::function<void()> handler) {
  int version = ++fd2version_[id];
  auto node =
      std::make_shared<TimerNode>(id, version, timeout, std::move(handler));
  timer_queue_.push(node);
}

void TimerManager::DelTimer(int id) {
  if (auto it = fd2version_.find(id); it != fd2version_.end()) {
    ++it->second;
  }
}

void TimerManager::Tick() {
  auto now = std::chrono::high_resolution_clock::now();
  while (!timer_queue_.empty()) {
    auto node = timer_queue_.top();
    if (node->expires > now) break;

    timer_queue_.pop();
    if (fd2version_[node->id] == node->version && !node->deleted) {
      if (node->handler) node->handler();
    }
  }
}

int TimerManager::GetNextTick() {
  if (timer_queue_.empty()) return -1;
  auto now = std::chrono::high_resolution_clock::now();
  auto node = timer_queue_.top();
  auto diff =
      std::chrono::duration_cast<std::chrono::milliseconds>(node->expires - now)
          .count();
  return diff > 0 ? diff : 0;
}