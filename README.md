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

发送过程：开启服务器创建一个listenfd，创建一个连接到端口，listenfd收到read创建一个新的fd，fd获取到read对报文进行解析

错了，channel还是纯粹表单就行，用httpconn记录loop
