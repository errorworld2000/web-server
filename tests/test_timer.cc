#include <gtest/gtest.h>
#include <thread>
#include "timer.h"

TEST(TimerManagerTest, AddAndTick) {
    TimerManager tm;
    bool triggered = false;
    tm.AddTimer(1, 10, [&] { triggered = true; });
    EXPECT_FALSE(triggered);

    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    tm.Tick();
    EXPECT_TRUE(triggered);
}

TEST(TimerManagerTest, DelTimer) {
    TimerManager tm;
    bool triggered = false;
    tm.AddTimer(1, 20, [&] { triggered = true; });
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

TEST(TimerManagerTest, MultipleTimers) {
    TimerManager tm;
    int counter = 0;
    tm.AddTimer(1, 10, [&] { counter += 1; });
    tm.AddTimer(2, 20, [&] { counter += 10; });

    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    tm.Tick();
    EXPECT_EQ(counter, 1);  // 只有第一个触发

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    tm.Tick();
    EXPECT_EQ(counter, 11); // 第二个触发
}