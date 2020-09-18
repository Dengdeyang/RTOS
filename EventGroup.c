/********************************************************************************
  * @file    event.c
  * @author  邓得洋
  * @version V0.1
  * @date    2020-08-03
  * @brief   事件标志组用于多任务或多事件间的同步
  * @atteration   
  ********************************************************************************/
#include "event.h"   
#include "stdlib.h"
#include "string.h"
#include "UART.h"
#include "RTOS.h"
#include "semaphore.h"   

//-------------------------------------------------------------------------//
/**
 * @brief  任务等待事件标志组，可设置阻塞时间或非阻塞。
 * @param  
			Task_x：     等待该事件标志组的任务id
			event：      事件标志组变量地址
			care_bit：   任务需要关注事件标志组的bit(最大32bit)
			relate_flag：关心的bit之间是“与”关系，还是“或”关系
			action_flag：等待到事件发生后，保持事件标志，还是清除事件标志
			delay_ms：   任务读取事件标志组，事件未发生，任务需要阻塞的延时时间，若为0，不等待，若为MAX_DELAY，一直等待。
 * @retval 等待该事件标志组成功或失败标志                       
 */
Statue_Type xEventGroupWaitBits(Task_Handle Task_x,Event_Handle * event, u32 care_bit,Relate_Type relate_flag,Action_Type action_flag,u32 delay_ms) 
{
	Statue_Type statue_flag;
	Statue_Type event_check_flag;
	
	event_check_flag = (Statue_Type) ((relate_flag == and_type) ? ((event->Event & care_bit) == care_bit):((event->Event & care_bit) != 0));
	
	if(event_check_flag == Event_Success)  statue_flag = Event_Success;
	else 
	{
		if(delay_ms == 0)  return Event_Fail;
		else
		{
			event->task_block_list[Task_x] = Semaphore_Block;
			event->task_care_bit_list[Task_x] = care_bit;
			event->task_relate_type_list[Task_x] = relate_flag;
			if(delay_ms == MAX_DELAY) 
			{
				Task_List[Task_x].task_statue = TASK_BLOCK;
				Task_List[Task_x].task_pend_statue = TASK_BLOCK;
				while(Task_List[Task_x].task_statue == TASK_BLOCK);
				statue_flag = Event_Success;
			}
			else 
			{
				RTOS_Delay(Task_x,delay_ms);
				event->task_block_list[Task_x] = Semaphore_Unblock;
				event_check_flag = (Statue_Type) ((relate_flag == and_type) ? ((event->Event & care_bit) == care_bit):((event->Event & care_bit) != 0));
				
				if(event_check_flag == Event_Success)  statue_flag = Event_Success;
				else                                   statue_flag = Event_Fail;
			}
		}
	}
	if(action_flag == release_type)  event->Event &= ~care_bit;
	
	return statue_flag;
}


//-----------------------------置位事件标志组对应bit----------------------------------------//
void Set_Event_Bit(Event_Handle * event,u8 bit)
{
	u16 i;
	u32 care_bit;
	Relate_Type relate_flag;
	Statue_Type event_check_flag;

	event->Event |= (1<<bit);
	
	for(i=TASK_NUM-1;i>0;i--)
	{
		if(event->task_block_list[i]==Semaphore_Block)  
		{
			care_bit = event->task_care_bit_list[i];
			relate_flag = (Relate_Type) event->task_relate_type_list[i];
			
			event_check_flag = (Statue_Type)((relate_flag == and_type) ? ((event->Event & care_bit) == care_bit):((event->Event & care_bit) != 0));
			
			if(event_check_flag == Event_Success)
			{
				event->task_block_list[i] = Semaphore_Unblock;
				Task_List[i].task_statue = TASK_READY;
				Task_List[i].task_pend_statue = TASK_READY;
			}
		}
	}
}


//-----------------------------清除事件标志组对应bit----------------------------------------//
void Reset_Event_Bit(Event_Handle * event,u8 bit)
{
	event->Event &= ~(1<<bit); 
}
						
