#include "timer.h"

void TimerManager::AddTimer(int id, int timeout,
                            std::function<void()>&& timeout_handler) {
  if (auto it = timer_node_map_.find(id); it != timer_node_map_.end()) {
    auto oldTimer = it->second;
    if (oldTimer->IsDeleted()) {
      oldTimer->SetTimeout(timeout);
      oldTimer->SetDeleted(false);
      return;
    }
  }
  auto newTimer = std::make_shared<TimerNode>(id, timeout, timeout_handler);
  timer_node_queue_.push(newTimer);
  timer_node_map_[id] = newTimer;
}

void TimerManager::DelTimer(int id) { timer_node_map_[id].SetDeleted(); }

void TimerManager::Tick() {
  while (!timer_node_queue_.empty()) {
    auto it = timer_node_queue_.top();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(
            it->expires_time_ - std::chrono::high_resolution_clock::now())
            .count() > 0) {
      break;
    }
    if (!it->IsDeleted) it->timeout_handler_();
    timer_node_queue_.pop();
    timer_node_map_.erase(it->GetId());
  }
}

int TimerManager::GetNextTick() { 
    int res=-1;
    if(!timer_node_queue_.empty()){
        res=std::chrono::duration_cast<std::chrono::milliseconds>(timer_node_queue_.top()->xepires_time_-std::chrono::high_resolution_clock::now()).count();
    }
    return res;
 }
