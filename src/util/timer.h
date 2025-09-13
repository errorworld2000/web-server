#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <queue>

class TimerManager {
 public:
  TimerManager() = default;

  void AddTimer(int fd, int timeout, std::function<void()> handler);
  void DelTimer(int fd);
  void Tick();
  int GetNextTick();

 private:
  struct TimerNode;
  using TimerNodePtr = std::shared_ptr<TimerNode>;
  struct TimerCmp {
    bool operator()(const TimerNodePtr& a, const TimerNodePtr& b) const;
  };

  std::mutex mutex_;
  std::priority_queue<TimerNodePtr, std::vector<TimerNodePtr>, TimerCmp>
      timer_queue_;
  std::unordered_map<int, int> fd2version_;
};