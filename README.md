# Web-
此高并发web服务器采用线程池和epoll　I/O多路复用实现高并发。线程池采用半同步半反应堆模式实现
测试环境为ubuntu16.04，c++11
编译代码为g++ socket_pack.cpp decode.cpp Server.cpp -pthread -std=c++11

