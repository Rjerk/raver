# raver

基于I/O多路复用epoll和线程池的并发HTTP服务器

## 使用

```
$ make
$ ./server [port] [thread-num]
```

## 实现模型

采用 epoll + threadpool 模型来实现并发，用 `SO_REUSEPORT` 允许多个套接字绑定在同一主机的同一个端口上，在内核层次上实现进行简单的负载均衡。

## 设计思路

ServiceManager管理IOManager和Acceptor。IOManager负责管理ThreadPool和Poller。

HTTPService注册到ServiceManager时，会注册一个自己的afterAccept回调函数给Acceptor，在accept一个连接时就进行回调，并建立一个HTTPConnection，HTTPConnection建立Channel注册读写回调函数，把该Channel作为数据传递给epoll并关注监听套接字上的EPOLL事件。

当EPOLL事件发生时，在IOManager中处理到来的事件，并的得到对应的Channel去处理IO操作，Channel会将读写任务放在ThreadPool的任务队列中，等待线程池中的线程进行处理。

线程使用HTTPConnction::doRead函数读HTTPRequest，用HTTPParser解析它，然后生成对应的HTTPResponse，并发送响应，然后关闭该连接。

Poller封装了epoll，采用level trigger。

HTTPConnction使用非阻塞IO，避免阻塞在读写上，最大限度地复用thread-of-control。它拥有自己的input和output Buffer，因为TCP是一个无边界的字节流协议，服务器必须处理接收不完整的情况。当只写了一部分数据时，去注册EPOLLOUT事件，一旦socket可写就立即发送数据;当读数据不完整时会等数据读完再处理，不会触发不间断的EPOLLIN事件，造成 busy-loop。Buffer参考了[muduo](https://github.com/chenshuo/muduo) 网络库的实现。

## TODO

- [x] 用`std::unique_ptr`管理资源，解决存在的内存泄露问题
- [ ] 支持动态内容
- [x] JSON 配置服务器
- [ ] 增加文件缓存，解决因文件过大造成的错误
- [x] 完善日志记录
- [ ] 增加认证
