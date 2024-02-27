#pragma once

#include <stdio.h>
#include <sys/epoll.h>
#include "ChatSystem.h"
// #include <stdio.h>
int create_epoll(int listen_fd);

// 处理不同的事件
// 一个循环
void handle_events(int epoll_fd,int listen_fd);

void handle_event(struct epoll_event* ev,int epoll_fd,int listen_fd);

// 处理连接
// 把调用accept接口重新创建一个socket用于通信
void handle_connection(int epoll_fd,int listen_fd);

void op_event(int epoll_fd,int fd,int state,uint32_t op);

// 服务器接收消息
void handle_read(void* arg);

// 服务器发送消息
void handle_write();
// 把数据读到cur->biggerbuf里面
int read_data(RecvBuf* cur);

// 用户下线的时候
void delete(RecvBuf* buf_ptr);

int parse_data(RecvBuf* cur);
ClientVerifyMessage* parse_verify_message(char* data);

void clean_buf(RecvBuf* buf_ptr);

