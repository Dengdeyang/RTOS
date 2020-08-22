#ifndef __SEMAPHORE_H
#define	__SEMAPHORE_H

#include "stm32f10x.h"
#include "RTOS.h"

#define Success 1
#define Fail 0

#define Semaphore_Block   1
#define Semaphore_Unblock   0

#define MAX_DELAY  0xFFFFFFFF

//#define Bin_Sema  1
//#define Mutex_Sema  2
typedef enum  
{
	Binary_Semaphore = 1,
	Mutex_Semaphore,
	Count_Semaphore,
}Semaphore_Type;

//队列节点
struct Queue_node
{
	u32 data;
	struct Queue_node *next;
};
typedef struct Queue_node Queue_point;

//队列定义
typedef struct
{
	Queue_point *head;
	Queue_point *tail;
	u32    queue_size;
}Queue;

typedef Queue * Queue_Handle;
char push(Queue *queue_data,u32 push_data);
char pull(Queue *queue_data,u32 *pull_data);
Queue * Creat_queue(void);

typedef struct
{
	char Semaphore;
	Task_Handle task_x;
	Task_Handle task_block_list[TASK_NUM];
}Semaphore_Handle;

char Semaphore_Take(Task_Handle Task_x,Semaphore_Type type,Semaphore_Handle *Semaphore,u32 delay_ms);
void Semaphore_Give(Semaphore_Type type,Semaphore_Handle *Semaphore);
#endif 
