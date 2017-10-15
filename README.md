# raver

基于I/O多路复用epoll和线程池的并发HTTP服务器

## 使用

```
$ make
$ ./server
```

通过浏览器访问该服务器 ip:8888

![](https://github.com/Rjerk/learning-notes/blob/master/img/web.jpg)

## 特性

- epoll + threadpoll 实现并发
- 用 `SO_REUSEPORT` 允许多个套接字绑定在同一主机的同一个端口上，在内核层次上实现进行简单的负载均衡
- 使用基于对象编程思想
- 使用 C++ 11

## benchmark

使用[wrk](https://github.com/wg/wrk) 进行benchmark测试:

```
./wrk -c4000 -d10s -t20 -H"Get /index.html HTTP/1.0"  http://localhost:8888/

Running 10s test @ http://localhost:8888/
  20 threads and 4000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   150.78ms  252.00ms   2.00s    93.05%
    Req/Sec   158.59    289.25     4.45k    91.57%
  28030 requests in 10.10s, 81.67MB read
  Socket errors: connect 0, read 27758, write 0, timeout 44
Requests/sec:   2774.21
Transfer/sec:      8.08MB
```

## 设计思路

ServiceManager管理IOManager和Acceptor。IOManager负责管理ThreadPool和EPoller。

HTTPService注册到ServiceManager时，会注册一个自己的newConnection给Acceptor作为回调函数，在每次accept后就进行回调，建立一个HTTPConnection，HTTPConnection建立Channel注册读写回调函数，把该Channel作为数据传递给epoll并关注监听套接字上的EPOLL事件。

当EPOLL事件发生时，在IOManager中处理到来的事件，得到对应的Channel去处理IO操作，Channel会将任务放在ThreadPool的任务队列中，等待线程池中的线程进行处理。

EPoller封装了epoll，采用level trigger。

HTTPConnction使用非阻塞IO，避免阻塞在读写上，最大限度地复用thread-of-control。它拥有自己的input和output Buffer，因为TCP是一个无边界的字节流协议，服务器必须处理接收不完整的情况。当只写了一部分数据时，去注册EPOLLOUT事件，一旦socket可写就立即发送数据;当读数据不完整时会等数据读完再处理，不会触发不间断的EPOLLIN事件，造成 busy-loop。实现参考了[muduo](https://github.com/chenshuo/muduo) 网络库的实现。

## TODO

- [x] 用`std::unique_ptr`管理资源，解决存在的内存泄露问题
- [ ] 支持动态内容
- [x] JSON 配置服务器
- [x] 增加文件缓存，便于热点文件的快速访问
- [x] 完善日志记录
- [ ] 增加认证
