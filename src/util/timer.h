#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <queue>
#include <time.h>
#include <unordered_map>

class TimerNode {
 public:
  explicit TimerNode(int id, int timeout,
                     std::function<void()> timeout_handler = nullptr)
      : id_(id), timeout_handler_(timeout_handler), deleted_(false) {
    SetTimeout(timeout);
  }
  std::chrono::high_resolution_clock::time_point GetExpiresTime() const {
    return expires_time_;
  }
  void SetTimeout(int timeout) {
    expires_time_ = std::chrono::milliseconds(timeout) +
                    std::chrono::high_resolution_clock::now();
  }
  bool IsDeleted() const { return deleted_; }
  void SetDeleted(bool value = true) { deleted_ = value; }
  bool GetId() { return id_; }
  void Handle() { timeout_handler_(); }

 private:
  int id_;
  std::function<void()> timeout_handler_;
  bool deleted_;
  std::chrono::high_resolution_clock::time_point expires_time_;
};

using TimerNodeSharedPtr = std::shared_ptr<TimerNode>;

class TimerManager {
 public:
  explicit TimerManager() {}
  void AddTimer(int id, int timeout,
                std::function<void()>&& timeout_handler = nullptr);
  void DelTimer(int id);
  void Tick();
  int GetNextTick();

 private:
  struct TimerCmp {
    bool operator()(const TimerNodeSharedPtr& a, const TimerNodeSharedPtr& b) {
      return a->GetExpiresTime() > b->GetExpiresTime();
    };
  };
  std::priority_queue<TimerNodeSharedPtr, std::vector<TimerNodeSharedPtr>,
                      TimerCmp>
      timer_node_queue_;
  std::unordered_map<int, TimerNodeSharedPtr> timer_node_map_;
};
