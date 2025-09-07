#include <gtest/gtest.h>
#include <thread>

#include "timer.h"

TEST(TimerNodeTest, Constructor) {
  TimerNode timer(1, 1000);
  EXPECT_EQ(timer.GetId(), 1);
  EXPECT_FALSE(timer.IsDeleted());
}

TEST(TimerNodeTest, Comparison) {
  TimerNode timer1(1, 100);
  TimerNode timer2(2, 200);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_TRUE(timer1.GetExpiresTime() < timer2.GetExpiresTime());
}

TEST(TimerNodeTest, Deletion) {
  TimerNode timer(1, 1000);
  timer.SetDeleted();
  EXPECT_TRUE(timer.IsDeleted());
}

TEST(TimerManagerTest, AddAndTick) {
  TimerManager tm;
  bool triggered = false;
  tm.AddTimer(1, 10, [&]() { triggered = true; });
  EXPECT_FALSE(triggered);
  std::this_thread::sleep_for(std::chrono::milliseconds(15));
  tm.Tick();
  EXPECT_TRUE(triggered);
}

TEST(TimerManagerTest, DelTimer) {
  TimerManager tm;
  bool triggered = false;
  tm.AddTimer(1, 20, [&]() { triggered = true; });
  tm.DelTimer(1);
  std::this_thread::sleep_for(std::chrono::milliseconds(25));
  tm.Tick();
  EXPECT_FALSE(triggered);
}

TEST(TimerManagerTest, GetNextTick) {
  TimerManager tm;
  tm.AddTimer(1, 100, nullptr);
  tm.AddTimer(2, 50, nullptr);
  int next_tick = tm.GetNextTick();
  EXPECT_GE(next_tick, 40);
  EXPECT_LE(next_tick, 50);
}
