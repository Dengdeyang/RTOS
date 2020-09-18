/********************************************************************************
  * @file    semaphore.c
  * @author  邓得洋
  * @version V0.1
  * @date    2020-08-03
  * @brief   
  * @atteration   
  ********************************************************************************/
#include "semaphore.h"   
#include "stdlib.h"
#include "string.h"
#include "UART.h"
#include "RTOS.h"


//----------------------------------入队-------------------------------------//
/**
 * @brief  动态内存申请方式增加单项链表长度，实现入队操作
 * @param  
			queue_data：     指向一条队列的指针
			push_data：      需要压入该队列的数据
 * @retval 入队成功或失败标志                      
 */
static char push(Queue *queue_data,u32 push_data)
{
	char push_action_result;
	
	if(queue_data->queue_size==0)
	{
		queue_data->tail->data = push_data;
		queue_data->queue_size++;
		push_action_result = Success;
	}
	else
	{
		Queue_point *p = (Queue_point *)malloc(sizeof(Queue_point));
		if(NULL == p) 
		{
			printf("push fail!\r\n");
			push_action_result = Fail;
		}
		else 
		{
			queue_data->tail->next = p;
			queue_data->tail = p;
			queue_data->tail->data = push_data;
			queue_data->tail->next = NULL;
			queue_data->queue_size++;
			
			push_action_result = Success;
		}
	}
	return push_action_result;
}


//----------------------------------出队-------------------------------------//
/**
 * @brief  动态内存释放方式减小单项链表长度，实现出队操作
 * @param  
			queue_data：     指向一条队列的指针
			*pull_data：     队列弹出数据存放变量地址
 * @retval 出队成功或失败标志                      
 */
static char pull(Queue *queue_data,u32 *pull_data)
{
    Queue_point *p;
	char pull_action_result;
	
	if(!queue_data->queue_size) 
	{
		pull_action_result = Fail;
	}
	else if(queue_data->queue_size == 1) 
	{
		*pull_data = queue_data->head->data;
		queue_data->queue_size--;
		pull_action_result = Success;
	}
	else 
	{
		*pull_data = queue_data->head->data;
		p = queue_data->head;
		queue_data->head = queue_data->head->next;
		free(p);
		queue_data->queue_size--;
		pull_action_result = Success;
	}
	return pull_action_result;
}


//-------------------------消息队列创建与初始化-------------------------------------//
Queue_Handle Creat_queue(void)
{
	Queue *queue_x = (Queue *)malloc(sizeof(Queue));
	
	queue_x->head = (Queue_point *)malloc(sizeof(Queue_point));
	queue_x->tail = queue_x->head;
	queue_x->queue_size = 0;

	queue_x->head->data = 0;
    queue_x->head->next = NULL;

	if((queue_x==NULL)||(queue_x->head==NULL))   return NULL;
	else                                         return  queue_x;
}

//--------------------------------发送消息队列--------------------------------------------//
char QueueSend(Queue *queue_data,u32 push_data,Task_Handle Task_x,u32 delay_ms)
{
	u16 i;
	char queue_flag;
	if(push(queue_data,push_data)==Fail)
	{
		if(delay_ms == 0)  queue_flag = Fail;
		else
		{
			queue_data->task_block_list[Task_x] = Semaphore_Block;
			if(delay_ms == MAX_DELAY) 
			{
				Task_List[Task_x].task_statue = TASK_BLOCK;
				Task_List[Task_x].task_pend_statue = TASK_BLOCK;
				while(Task_List[Task_x].task_statue == TASK_BLOCK);
			}
			else 
			{
				RTOS_Delay(Task_x,delay_ms);
				queue_data->task_block_list[Task_x] = Semaphore_Unblock;
			}
			queue_flag = push(queue_data,push_data);
		}
	}
	
	for(i=TASK_NUM-1;i>0;i--)
	{
		if(queue_data->task_block_list[i]==Semaphore_Block)  
		{
			queue_data->task_block_list[i] = Semaphore_Unblock;
			Task_List[i].task_statue = TASK_READY;
			Task_List[i].task_pend_statue = TASK_READY;
			return queue_flag;
		}
	}
	return queue_flag;
}

//--------------------------------接收消息队列--------------------------------------------//
char QueueReceive(Queue *queue_data,u32 *pull_data,Task_Handle Task_x,u32 delay_ms)
{
	if(queue_data->queue_size > 0)
	{
		pull(queue_data,pull_data);
		return Success;
	}
	else
	{
		if(delay_ms == 0)  return Fail;
		else
		{
			queue_data->task_block_list[Task_x] = Semaphore_Block;
			if(delay_ms == MAX_DELAY) 
			{
				Task_List[Task_x].task_statue = TASK_BLOCK;
				Task_List[Task_x].task_pend_statue = TASK_BLOCK;
				while(Task_List[Task_x].task_statue == TASK_BLOCK);
				pull(queue_data,pull_data);
				return Success;
			}
			else 
			{
				RTOS_Delay(Task_x,delay_ms);
				queue_data->task_block_list[Task_x] = Semaphore_Unblock;
				if(queue_data->queue_size > 0) 
				{
				    pull(queue_data,pull_data);
					return Success;
				}
				else  return Fail;
			}
		}
	}
}

//-------------------------二值+互斥+计数信号量Take函数-------------------------------------//
/**
 * @brief  信号量获取函数，可根据需要设置信号量的种类(二值、互斥、计数)，并设置阻塞延时
 * @param  
			Task_x：     调用该函数的任务id
			type：       操作信号量的种类
            *Semaphore： 信号量地址
            delay_ms：   阻塞延时值
 * @retval 信号量获取成功或失败标志                      
 */
char Semaphore_Take(Task_Handle Task_x,Semaphore_Type type,Semaphore_Handle *Semaphore,u32 delay_ms)
{
	if(Semaphore->Semaphore)  
	{
		if(type == Count_Semaphore)  Semaphore->Semaphore -= 1;
		else 						 Semaphore->Semaphore = 0;
		return Success;
	}
	else                  
	{
		if(delay_ms == MAX_DELAY) 
		{
			Semaphore->task_block_list[Task_x] = Semaphore_Block;
			Task_List[Task_x].task_statue = TASK_BLOCK;
			Task_List[Task_x].task_pend_statue = TASK_BLOCK;
			while(Task_List[Task_x].task_statue == TASK_BLOCK);
			
			if((type == Count_Semaphore)&&(Semaphore->Semaphore))    Semaphore->Semaphore -= 1;
		    else 						                             Semaphore->Semaphore = 0;
			return Success;
		}
		else if(delay_ms == 0)  return Fail;
		else 
		{
			Semaphore->task_block_list[Task_x] = Semaphore_Block;
			RTOS_Delay(Task_x,delay_ms);
			Semaphore->task_block_list[Task_x] = Semaphore_Unblock;
			if(Semaphore->Semaphore) 
			{
				if(type == Count_Semaphore)  Semaphore->Semaphore -= 1;
				else 						 Semaphore->Semaphore = 0;
				return Success;
			}
			else  return Fail;

		}
	}
}


//-------------------------二值+互斥+计数信号量Give函数-------------------------------------//
/**
 * @brief  信号量释放函数，无阻赛
 * @param  
			type：       操作信号量的种类
            *Semaphore： 信号量地址
 * @retval  NO                      
 */
void Semaphore_Give(Semaphore_Type type,Semaphore_Handle *Semaphore)
{
	u16 i;
	if(type == Count_Semaphore)  Semaphore->Semaphore += 1;
	else 						 Semaphore->Semaphore = 1;
	
	for(i=TASK_NUM-1;i>0;i--)
	{
		if(Semaphore->task_block_list[i]==Semaphore_Block)  
		{
			Semaphore->task_block_list[i] = Semaphore_Unblock;
			Task_List[i].task_statue = TASK_READY;
			Task_List[i].task_pend_statue = TASK_READY;
			
			if(type !=Binary_Semaphore) return;
		}
	}
}

/*********************************************END OF FILE**********************/
