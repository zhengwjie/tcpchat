
#include "threadpool.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
const int NUM = 2;
typedef struct Task
{
	void (*func)(void *arg);
	void *arg;
} Task;

struct ThreadPool
{
	//    任务队列
	Task *TaskQ;

	int queueCapacity;

	int queueSize;

	int queueFront; // 队伍头里面取数据
	int queueRear;	// 放数据

	// 两类线程
	pthread_t managerID;

	pthread_t *threadIDs;

	int minNum; // 最小线程数
	int maxNum;

	int busyNum; // 工作的线程

	int aliveNum; // 存活的线程

	int exitNum; // 要杀死的线程数量

	pthread_mutex_t mutexpool;
	// 锁定busy的变量
	pthread_mutex_t mutexBusy;

	int shutdown;
	// 定义是否需要销毁线程池

	// 空了的话阻塞工作线程
	// 条件修改了之后唤醒一个进程

	// 标记任务队列是否满了和空了
	// 是否满了
	pthread_cond_t notFull;
	// 是否空了
	pthread_cond_t notEmpty;
};

ThreadPool *threadPoolCreate(int min, int max, int queueCapacity)
{
	ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
	do
	{
		if (pool == NULL)
		{
			printf("pool malloc error");
			break;
		}
		pool->threadIDs = (pthread_t *)malloc(sizeof(pthread_t) * max);
		if (pool->threadIDs == NULL)
		{
			printf("threadIDs malloc error");
			break;
		}
		memset(pool->threadIDs, 0, sizeof(pthread_t) * max);
		pool->minNum = min;
		pool->maxNum = max;
		pool->busyNum = 0;
		pool->aliveNum = min;

		pool->exitNum = 0;

		if (pthread_mutex_init(&pool->mutexpool, NULL) != 0 ||
			pthread_mutex_init(&pool->mutexBusy, NULL) != 0 ||
			pthread_cond_init(&pool->notEmpty, NULL) != 0 ||
			pthread_cond_init(&pool->notFull, NULL) != 0)
		{
			printf("cond and mutex init error\n");
			break;
		}
		// 任务队列初始化
		pool->TaskQ = (Task *)malloc(sizeof(Task) * queueCapacity);

		if (pool->TaskQ == NULL)
		{
			printf("task queue malloc error\n");
			break;
		}
		pool->queueCapacity = queueCapacity;
		pool->queueSize = 0;
		pool->queueFront = 0;
		pool->queueRear = 0;
		pool->shutdown = 0;
		// 创建线程

		pthread_create(&pool->managerID, NULL, manager, pool);
		for (int i = 0; i < min; ++i)
		{
			pthread_create(&pool->threadIDs[i], NULL, worker, pool);
		}
		return pool;
	} while (0);

	if (pool && pool->threadIDs)
		free(pool->threadIDs);
	if (pool && pool->TaskQ)
		free(pool->TaskQ);
	if (pool)
		free(pool);

	return NULL;
}

void *worker(void *arg)
{
	ThreadPool *pool = (ThreadPool *)arg;
	while (1)
	{
		// 给互斥量上锁
		pthread_mutex_lock(&pool->mutexpool);
		// 加锁是为了防止所有线程同时执行pthread_cond_wait
		//
		while (pool->queueSize == 0 && !pool->shutdown)
		{
			// 在进入休眠的时候释放锁
			// 其他的线程也可以获取锁，然后进入休眠状态。
			// 满足条件后加锁。
			pthread_cond_wait(&pool->notEmpty, &pool->mutexpool);
			printf("I am weak\n");

			if (pool->exitNum > 0)
			{
				pool->exitNum--;
				if (pool->aliveNum > pool->minNum)
				{
					pool->aliveNum--;
					// 释放锁
					pthread_mutex_unlock(&pool->mutexpool);
					// 让工作线程自杀
					threadExit(pool);
				}
			}
			printf("hhhh");
		}
		//
		if (pool->shutdown)
		{
			pthread_mutex_unlock(&pool->mutexpool);
			threadExit(pool);
		}
		Task task;
		task.func = pool->TaskQ[pool->queueFront].func;
		task.arg = pool->TaskQ[pool->queueFront].arg;
		// 移动queue front
		pool->queueFront = (pool->queueFront + 1) % (pool->queueCapacity);
		pool->queueSize--;
        
		pthread_cond_signal(&pool->notFull);

		pthread_mutex_unlock(&pool->mutexpool);
        
		printf("thread %ld start working\n",pthread_self());

		pthread_mutex_lock(&pool->mutexBusy);

		pool->busyNum++;

		pthread_mutex_unlock(&pool->mutexBusy);

		task.func(task.arg);

		free(task.arg);
		task.arg = NULL;

		pthread_mutex_lock(&pool->mutexBusy);

		pool->busyNum--;

		pthread_mutex_unlock(&pool->mutexBusy);

		printf("thread %ld end working\n",pthread_self());
		// 或者
		// (*task.func)(task.arg);
	}
	return NULL;
}

void *manager(void *arg)
{
	ThreadPool *pool = (ThreadPool *)arg;
	while (!pool->shutdown)
	{
		/* code */
		pthread_mutex_lock(&pool->mutexpool);
		int queueSize = pool->queueSize;
		int aliveNum = pool->aliveNum;
		pthread_mutex_unlock(&pool->mutexpool);

		pthread_mutex_lock(&pool->mutexBusy);
		int busyNum = pool->busyNum;
		pthread_mutex_unlock(&pool->mutexBusy);

		// 添加线程
		if (queueSize > aliveNum && aliveNum < pool->maxNum)
		{
			int counter = 0;
			pthread_mutex_lock(&pool->mutexpool);
			for (int i = 0; i < pool->maxNum && counter < NUM && pool->aliveNum < pool->maxNum; ++i)
			{
				if (pool->threadIDs[i] == 0)
				{
					pthread_create(&pool->threadIDs[i], NULL, worker, pool);
					printf("create thread %ld .\n",pool->threadIDs[i]);
					counter++;
					pool->aliveNum++;
				}
			}
			pthread_mutex_unlock(&pool->mutexpool);
		}

		// 销毁线程
		// 忙的线程*2<存活的线程 &&存活的线程大于最小线程数
		// minNum不用保护，因为他们只在线程池创建的时候被修改
		if (busyNum * 2 < aliveNum && aliveNum > pool->minNum)
		{
			// 开始销毁
			pthread_mutex_lock(&pool->mutexpool);
			pool->exitNum = NUM;
			pthread_mutex_unlock(&pool->mutexpool);

			// 让工作线程自杀
			for (int i = 0; i < NUM; ++i)
			{
				pthread_cond_signal(&pool->notEmpty);
			}
		}
		sleep(3);
	}
	return NULL;
}
void threadExit(ThreadPool *pool)
{
	pthread_t tid = pthread_self();

	for (int i = 0; i < pool->maxNum; ++i)
	{
		if (pool->threadIDs[i] == tid)
		{
			pool->threadIDs[i] = 0;
			printf("threadExit() called, %ld exiting...\n", tid);
			break;
			;
		}
	}
	pthread_exit(NULL);
}

void threadPoolAdd(ThreadPool* pool, void(*func)(void*),void* arg){

	pthread_mutex_lock(&pool->mutexpool);

	while (pool->queueSize==pool->queueCapacity&&!pool->shutdown)
	{
		/* code */
		pthread_cond_wait(&pool->notFull,&pool->mutexpool);
	}
	if(pool->shutdown){
		pthread_mutex_unlock(&pool->mutexpool);
		return;
	}

	pool->TaskQ[pool->queueRear].func=func;
	pool->TaskQ[pool->queueRear].arg=arg;

	pool->queueRear=(pool->queueRear+1)%(pool->queueCapacity);
	pool->queueSize++;

	pthread_cond_signal(&pool->notEmpty);

    pthread_mutex_unlock(&pool->mutexpool);

}

int threadPoolBusyNum(ThreadPool* pool){
	pthread_mutex_lock(&pool->mutexBusy);
	int busyNum=pool->busyNum;
	pthread_mutex_unlock(&pool->mutexBusy);

	return busyNum;
}

int threadPoolAliveyNum(ThreadPool* pool){
	pthread_mutex_lock(&pool->mutexpool);
	int aliveNum=pool->aliveNum;
	pthread_mutex_unlock(&pool->mutexpool);
	return aliveNum;
}

int threadPoolDestory(ThreadPool* pool){
	if(pool==NULL){
		return -1;
	}
	// 关闭线程池
	pool->shutdown=1;
	pthread_join(pool->managerID,NULL);
	
	// 唤醒所有活着的线程
	for(int i=0;i<pool->aliveNum;++i){
		pthread_cond_signal(&pool->notEmpty);
	}

	for(int i=0;i<pool->maxNum;++i){
		if(pool->threadIDs[i]==0){
			continue;
		}
		pthread_join(pool->threadIDs[i],NULL);
	}
	// 释放堆内存
	if(pool->TaskQ){
		free(pool->TaskQ);
	}
	if(pool->threadIDs){
		free(pool->threadIDs);
	}
	pthread_mutex_destroy(&pool->mutexBusy);
	pthread_mutex_destroy(&pool->mutexpool);

    pthread_cond_destroy(&pool->notEmpty);
	pthread_cond_destroy(&pool->notFull);

	free(pool);

	return 0;
}


