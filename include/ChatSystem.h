#pragma once
#include "config.h"
#include<unistd.h>
#include<stdint.h>
typedef struct{
	int id;
	char username[15];
	char passwd[15];
}User;

typedef struct{
	int id;
	char groupname[15];
	// 变长数组
	int users[0];
}Group;

// 存储groups
// register_users
// 

// 用户消息类型
typedef enum {
    REGISTER_REQUEST=0,       //注册
	LOGIN_REQUEST,          // 登录消息
    GROUP_CHAT_REQUEST,           // 群聊消息
    GROUP_CREATE_REQUEST,   // 创建群组消息  
	USER_CHAT_REQUEST,    // 发给一个人的消息
	USER_FILE_SEND_REQUEST //发送文件的请求。
}MessageTpye;

// 用户消息
// 记录用户的id
typedef struct{
	MessageTpye type;
	uint32_t message_id; //客户端消息的id

	uint32_t user_id; //消息的发起者 如果是注册的话，不需要user_id
	uint32_t target_id; //如果是GROUP_CHAT target_id就是group_id
	// GROUP_CHAT target_id就是user_id 给其他用户发送消息
	// 超出部分存储在message中
    // 否则就是消息 text
	// 如果message==short_message 那么就是消息可以<1024的大小，否则
	// malloc一定大小的消息。
	// 我们默认消息都是小于1024个字节的
	// 如果是创建群聊的话，那么需要传输所有要创建的用户以及 group_id 使用\0分割
	// 发送的就是消息
	char short_message[MAX_MESSAGE_LEN];
}ClientChatMessage;

// 记录他的user_name
// 根据user_name找到id

typedef struct{
	char user_name[25];
	char passwd[25];
}ClientVerifyMessage;


// 使用线程池处理每次的消息
// 接收数据并且解析数据 创建一个响应 RESPONSE 消息
// 并且要把这些消息发送出去 
// 两个epoll对象，一个监听连接和读取数据

// 一个监听写对象 使用边沿触发write和send不冲突。

// 需要维护一个发送队列 需要考虑消息发不完的情况
// 单线程的监听socket
// 使用线程池进行写 每次写的时候传入socket描述符
// 不同的描述符之间不干涉 用户id 谁发来的信息
// 创建线程池，直接写就可以了

// 客户端需要发送文件
// 发送文件的请求
// 文件名
// 文件内容
// 客户端发送文件的请求结构体
// 使用线程池处理

// 一个消息，阻塞在读取这里
// 首先发送文件头
// 然后发送文件内容
typedef struct {
    uint32_t len;
	MessageTpye type;
    uint32_t message_id;

	uint32_t user_id; //消息的发起者 如果是注册的话，不需要user_id
	uint32_t target_id; //如果是GROUP_CHAT target_id就是group_id
	uint32_t fileSize;
    char filename[MAX_FILENAME_LENGTH];
    // 每次接收一部分文件数据
	// 写入文件中
    // char content[MAX_FILE_LEN];
}ClientFileMessage;


// 用户消息类型
typedef enum {

    REGISTER_RESPONSE,       //是否注册成功
	LOGIN_RESPONSE,          // 登录消息
    GROUP_CHAT_RESPONSE,           // 群聊消息给 群组里的用户发消息
	USER_CHAT_RESPONSE,    // 发给一个人的消息

	GROUP_CREATE_RESPONSE,   // 创建群组消息  

	USER_FILE_SEND_RESPONSE, //发送文件的响应


	UPDATE_USER_STATE, //更新用户状态 在线或者不在线

}ServerMessageTpye;

// 如果是聊天的消息的话，直接转发


// 表示是否注册成功
// 表示是否登录成功
typedef struct ServerVerifyMessage{
	uint32_t len;
	MessageTpye type;

	// 告诉对方你的user_id.
	// 不管是登录还是注册
	uint32_t user_id;

	uint32_t response_code; 
	char response_message[512]; 
	// 你已经登录成功
}ServerVerifyMessage;

typedef struct UpdateUserInfo
{
	uint32_t len;
	MessageTpye type;

	uint32_t user_id;

    char username[15];
	uint32_t status;  //用户状态 0 创建了用户 1. 用户在线  2. 用户离线退出
	/* data */
}UpdateUserInfo;


typedef struct ChatEvent
{
	/* data */
	int epoll_fd;
	int fd;
}ChatEvent;

typedef struct RecvBuf
{
	/* data */
	int fd;
	int buf_len;
	// 已经接收的长度
	int recved_len;
	// 包的长度
	int pack_len;

	char* bigger_buf;
	char buf[1024];
	struct RecvBuf* next;
}RecvBuf;

typedef struct SendBuf
{
	int fd;
	int buf_len;
	char* buf;
	struct SendBuf* next;
}SendBuf;


// 根据fd去
// 告诉其他的用户
// 一起创建一个群组
// 向其他用户发送消息


// 登录成功的话，需要发送用户列表，以及群组消息 // 离线的消息列表

// 如果是登录成功的话，需要向其他的客户端发送 更新用户状态

// 消息发送 向用户发送消息发送成功的响应，需要向其他人转发消息
// 在离线用户中记录对应的消息。

// 文件发送 接收完成之后，向用户发送文件发送成功的响应，并且向其他的用户转发消息
// 在离线用户中记录对应的文件。

// typedef struct {


// }Server

// 发送了消息之后，就要返回响应
// 客户端负责发送消息
// 同时也要考虑接收消息

// 根据RESPONSE的类型解析消息
// 一种是消息的ACK 
// 注册成功
// 登录成功  
// 消息发送成功 message_id 告诉客户端消息被成功发送 
// 其他人的消息 

// 一个用户登录 告知其他用户
// 并且给这个用户一个 id  和其他用户的id
// 应该有一个用户列表和用户名列表
// 以及groups 用户所在的群组
