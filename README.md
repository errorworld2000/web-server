### epoll(工具)

### reactor(模式)
利用channel封装fd以及关心事件对应的回调函数
利用EventLoop将事件分发给对应channel，回调chennel注册的回调函数

### One Loop per Thread
每个线程一个EventPool,主线程负责accept连接

1. 先开始写channel,添加handler和handle以及events,发现要指定一个eventloop挂载

2. 先写epoller,主要对epoll的函数进行封装，接收channel注册到epoll，wait时获取事件并放到events_，接收channel时要记录才能获取到activechannel;
timermanager则是用于监听expire事件，tick时检查是否过期wan

3. 写eventloop，主要是将epoller和channel和timermanager封装，接收channel然后loop里进行waitevent操作获取事件。

4. 然后是thread，主要就是线程创建loop任务，pool再管理全部线程，但pool还是要存loop因为实际还是操作loop，thread只是复制创建。创建pool需要先传个loop初始化(不对当前线程校验)

5. 写server，主要先创建listenfd，再创建新的acceptchannel，然后生成个loop用于处理accptchannel以及生成eventloopthreadpool,给channel绑定read事件用于建立新的连接

6. read事件绑定连接时，手动绑定麻烦，创建个httpconn替我们绑定。绑定完事件后发现如果读取完了或者报错想要关闭有点麻烦，得在channel里面记录eventloop便于我们关闭监听和修改channel。

7. channel还是纯粹表单就行，用httpconn记录loop,loop和channel是比较底层一个负责结构一个负责运行，实际只要直接创建httpconn，会自动创建需要的表单并添加到到loop内运行

8. eventloop里面有点问题，需要考虑loop时和add_channel可能出现的异步问题，wait时可能会有新的channel加入，可能会延迟处理，添加wakechannel用于唤醒。

9. timer_manager之前懒删除没考虑好，替换为使用版本号判断

[ ] 收到第二次请求和断开有问题