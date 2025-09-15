# Web Server

---

## 📌 核心架构设计

本服务器基于 **C++** 实现，采用主流的 **Reactor** 设计模式，并结合 **One Loop per Thread** 的并发模型。

### Reactor 模式

* **Channel**
  封装 `fd`、其关心的事件以及对应的回调函数，是事件的抽象载体。

* **EventLoop**
  事件循环核心。调用 `epoll_wait` 等待事件发生，并将激活的事件分发给对应的 `Channel`，再由 `Channel` 执行注册的回调函数。

### One Loop per Thread 模型

* **主线程**
  拥有一个 `EventLoop`，专门负责监听服务器端口和接收新的客户端连接。

* **EventLoopThreadPool**
  内含多个子线程，每个子线程都拥有一个独立的 `EventLoop`。

* **连接分发**
  主线程 `accept` 新连接后，将新连接交给线程池中的某个 `EventLoop` 进行读写处理。

---

## 📝 编码过程

根据核心架构设计，从零开始一步步实现web-server。

1. **Channel**

   * 封装 `fd`、事件类型及回调函数。

2. **Epoller**

   * 封装 `epoll` 的系统调用接口。
   * 管理 `Channel` 的注册与移除，在 `wait` 后返回活跃事件。

3. **Timer**

   * 管理超时事件，`tick` 时检查并处理过期任务。

4. **EventLoop**

   * 封装 `Epoller`、`Channel` 和 `TimerManager`。
   * 循环中等待事件并调度 `Channel` 的回调函数。

5. **Thread & ThreadPool**

   * `Thread`：封装一个线程和其内部的 `EventLoop`。
   * `ThreadPool`：统一管理所有线程与 `EventLoop`，并负责调度。

6. **Server**

   * 创建 `listenfd` 与 `acceptChannel`，负责接收新连接。
   * 将新连接交给 `ThreadPool` 管理。

7. **HttpConn**

   * 封装一个 HTTP 连接，自动为 `Channel` 绑定回调。
   * 内部保存 `EventLoop`，便于关闭和修改监听。
   * 继承`ProtocolHandler`，便于切换别的连接。

8. **优化点**

   * EventLoop 可能在循环和 `AddChannel` 时产生异步问题：通过 `WakeChannel` 唤醒。
   * Timer 采用 **版本号判断** 替代懒删除。
   * EventLoop 与 Timer 涉及多线程安全问题，需要加锁。
   * Server 与 HttpConn 使用 **ET 模式**，需要 `while` 循环一次性读完。
   * EventLoop 内部添加 **pending 队列**，减少锁竞争。
   * Timer 与 HttpConn 增加回调函数，确保连接能正确关闭。
   * 通过 `perf` 与 `flamegraph` 定位性能瓶颈：

     * `ParseHeader`、`ParseLine` 开销大。
     * 使用 `find` 替代 `regex` 提升性能。

---

## 🚀 快速开始

**环境**：

* WSL2（Ubuntu 22）
* Cmake
* GCC(g++)

```bash
# 克隆仓库
git clone --recurse-submodules https://github.com/errorworld2000/web-server.git

# 进入根目录
cd ./web-server

# 运行web-server
./build/web-server
```

---

## 🔧 测试方法

```bash
# 安装Apache Benchmark(ab)
sudo apt update
sudo apt install apache2-utils

# 短连接测试
ab -n 100000 -c 300 http://127.0.0.1:8080/

# 长连接测试
ab -n 100000 -c 300 -k http://127.0.0.1:8080/

# 查找进程
pgrep web-server

# 性能分析
perf record -g -p PID
perf script -i perf.data &> perf.unfold
./third_party/FlameGraph/stackcollapse-perf.pl perf.unfold &> perf.folded
./third_party/FlameGraph/flamegraph.pl perf.folded > perf.svg
```

---

## 📑 性能测试结果

|连接类型|请求数|并发数|QPS|
|-|-|-|-|
|短连接|100000|300|3058.48|
|长连接|100000|300|26735.34|

---

## 👾 Bug

* [ ] 压测 10w 短连接时，最后部分请求未收到响应（长连接无此问题）。

---
