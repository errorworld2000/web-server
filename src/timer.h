#pragma once
#include <chrono>
#include <functional>
#include <memory>
#include <queue>
#include <time.h>
#include <unordered_map>

using TimerNodeSharedPtr = std::shared_ptr<TimerNode>;

class TimerNode {
 public:
  explicit TimerNode(int id, int timeout);
  bool operator<(const TimerNode& t) const {
    return expires_time_ < t.expires_time_;
  }
  bool SetTimeout(int timeout) {
    expires_time_ = std::chrono::milliseconds(timeout) +
                    std::chrono::high_resolution_clock::now();
  }
  bool IsDeleted() const { return deleted_; }
  bool SetDeleted(bool value = true) { deleted_ = value; }
  bool GetId() { return id_; }

 private:
  int id_;
  std::function<void()> timeout_handler_;
  bool deleted_;
  std::chrono::high_resolution_clock::time_point expires_time_;
};

class TimerManager {
 public:
  explicit TimerManager();
  void AddTimer(int id, int timeout, std::function<void()>&& timeout_handler);
  void DelTimer(int id);
  void Tick();
  int GetNextTick();

 private:
  struct TimerCmp {
    bool operator()(const TimerNodeSharedPtr& a, const TimerNodeSharedPtr& b) {
      return *a < *b;
    };
  };
  std::priority_queue<TimerNodeSharedPtr, std::vector<TimerNodeSharedPtr>,
                      TimerCmp>
      timer_node_queue_;
  std::unordered_map<int, TimerNodeSharedPtr> timer_node_map_;
};
