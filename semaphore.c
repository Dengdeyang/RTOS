/********************************************************************************
  * @file    semaphore.c
  * @author  ddy
  * @version V0.1
  * @date    2020-08-03
  * @brief   单项链表构成队列操作
  * @atteration   
  ********************************************************************************/
#include "semaphore.h"   
#include "stdlib.h"
#include "string.h"
#include "UART.h"
#include "RTOS.h"


//-------------------------入队-------------------------------------//
char push(Queue *queue_data,u32 push_data)
{
	char push_action_result;
	taskENTER_CRITICAL();
	
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
	taskEXIT_CRITICAL();
	return push_action_result;
}


//-------------------------出队-------------------------------------//
char pull(Queue *queue_data,u32 *pull_data)
{
    Queue_point *p;
	char pull_action_result;
	taskENTER_CRITICAL();
	
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
	taskEXIT_CRITICAL();
	return pull_action_result;
}

//-------------------------消息队列创建与初始化-------------------------------------//
Queue * Creat_queue(void)
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

//-------------------------二值+互斥+计数信号量Take函数-------------------------------------//
char Semaphore_Take(Task_Handle Task_x,Semaphore_Type type,Semaphore_Handle *Semaphore,u32 delay_ms)
{
	if((*Semaphore).Semaphore)  
	{
		if(type == Count_Semaphore)  (*Semaphore).Semaphore -= 1;
		else 						 (*Semaphore).Semaphore = 0;
		return Success;
	}
	else                  
	{
		if(delay_ms == MAX_DELAY) 
		{
			(*Semaphore).task_block_list[Task_x] = Semaphore_Block;
			Task_List[Task_x].task_statue = TASK_BLOCK;
			Task_List[Task_x].task_pend_statue = TASK_BLOCK;
			while(Task_List[Task_x].task_statue == TASK_BLOCK);
			
			if((type == Count_Semaphore)&&((*Semaphore).Semaphore))    (*Semaphore).Semaphore -= 1;
		    else 						                               (*Semaphore).Semaphore = 0;
			return Success;
		}
		else if(delay_ms == 0)  return Fail;
		else 
		{
			(*Semaphore).task_block_list[Task_x] = Semaphore_Block;
			RTOS_Delay(Task_x,delay_ms);
			(*Semaphore).task_block_list[Task_x] = Semaphore_Unblock;
			if((*Semaphore).Semaphore) 
			{
				if(type == Count_Semaphore)  (*Semaphore).Semaphore -= 1;
				else 						 (*Semaphore).Semaphore = 0;
				return Success;
			}
			else  return Fail;

		}
	}
}

//-------------------------二值+互斥+计数信号量Give函数-------------------------------------//
void Semaphore_Give(Semaphore_Type type,Semaphore_Handle *Semaphore)
{
	u16 i;
	if(type == Count_Semaphore)  (*Semaphore).Semaphore += 1;
	else 						 (*Semaphore).Semaphore = 1;
	
	for(i=TASK_NUM-1;i>0;i--)
	{
		if((*Semaphore).task_block_list[i]==Semaphore_Block)  
		{
			(*Semaphore).task_block_list[i] = Semaphore_Unblock;
			Task_List[i].task_statue = TASK_READY;
			Task_List[i].task_pend_statue = TASK_READY;
			
			if(type !=Binary_Semaphore) return;
		}
	}
}

/*********************************************END OF FILE**********************/
