#pragma once

// 线程池的创建以及初始化
typedef struct ThreadPool ThreadPool;

ThreadPool* threadPoolCreate(int min,int max,int queueCapacity);

void* worker(void* arg);

void* manager(void* arg);
void threadExit(ThreadPool* pool);

void threadPoolAdd(ThreadPool* pool, void(*func)(void*),void* arg);

int threadPoolBusyNum(ThreadPool* pool);

int threadPoolAliveyNum(ThreadPool* pool);
// 销毁线程池

int threadPoolDestory(ThreadPool* pool);
// 添加工作任务的函数

// 当前线程池中工作的个数


// 当前线程池中活着的个数



