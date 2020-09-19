#ifndef __SEMAPHORE_H
#define	__SEMAPHORE_H

#include "stm32f10x.h"
#include "RTOS.h"

#define Success 1
#define Fail 0
#define Semaphore_Block   1
#define Semaphore_Unblock   0
#define MAX_DELAY  0xFFFFFFFF

//信号量类型枚举
typedef enum  
{
	Binary_Semaphore = 1,
	Mutex_Semaphore,
	Count_Semaphore,
}Semaphore_Type;

//队列节点结构体
struct Queue_node
{
	volatile u32 data;
	struct Queue_node *next;
};

typedef struct Queue_node Queue_point;

//队列结构体
typedef struct
{
	Queue_point *head;
	Queue_point *tail;
	volatile u32    queue_size;
	volatile Task_Handle task_block_list[TASK_NUM];
}Queue;

//信号量结构体
typedef struct
{
	volatile char Semaphore;
	volatile Task_Handle task_block_list[TASK_NUM];
}Semaphore_Handle;

typedef Queue * Queue_Handle;

Queue_Handle Creat_queue(void);
char QueueSend(Queue *queue_data,u32 push_data,Task_Handle Task_x,u32 delay_ms);
char QueueReceive(Queue *queue_data,u32 *pull_data,Task_Handle Task_x,u32 delay_ms);
char Semaphore_Take(Task_Handle Task_x,Semaphore_Type type,Semaphore_Handle *Semaphore,u32 delay_ms);
void Semaphore_Give(Semaphore_Type type,Semaphore_Handle *Semaphore);
#endif 
