/********************************************************************************
  * @file    soft_timer.c
  * @author  邓得洋
  * @version V0.1
  * @date    2020-08-03
  * @brief   软件定时器数量可在内存允许条件下根据需要定义，突破硬件定时器的数量限制
  * @atteration   
  ********************************************************************************/
#include "soft_timer.h"   
#include "stdlib.h"
#include "string.h"
#include "UART.h"
#include "RTOS.h"

//--------------------------------软件定时器列表定义-----------------------------------//
Soft_Timer_Unit Soft_Timer_List[Soft_Timer_NUM]={0};
Task_Handle timer_guard_task_id;


//--------------------------------软件定时器守护任务-----------------------------------//
/**
 * @brief   在所有RTOS任务中，软件定时器守护任务的优先级最高，RTOS心跳中断中判断到有软件定时器ready会触发该任务，
            在该任务中进行调用软件定时器对应的回调函数
 * @param   NO
 * @retval  NO                      
 */
void soft_timer_guard_task(void)
{
	u16 i=0;
	while(1)
	{
		for(i=0;i<Soft_Timer_NUM;i++)
		{
			if(Soft_Timer_List[i].timer_statue == TASK_READY)  
			{
				(*Soft_Timer_List[i].callback_function)();
				Soft_Timer_List[i].timer_statue = TASK_BLOCK;
			}
		}
		Pend_Task(timer_guard_task_id);
		while(Task_List[timer_guard_task_id].task_statue == TASK_BLOCK);
	}
}

//-------------------------------启动对应软件定时器-----------------------------------//
void Start_Soft_Timer(Soft_Timer_Handle timer_id)
{
	Soft_Timer_List[timer_id].system_user_tick_count = SysTick_count + Soft_Timer_List[timer_id].user_tick_count;
	Soft_Timer_List[timer_id].start_timer_flag = start_timer;
}


//-------------------------------停止对应软件定时器------------------------------------//
void Stop_Soft_Timer(Soft_Timer_Handle timer_id)
{
	Soft_Timer_List[timer_id].start_timer_flag = stop_timer;
	Soft_Timer_List[timer_id].timer_statue = TASK_BLOCK;
}


//---------------------设置对应软件定时器的定时值(RTOS心跳节拍数)------------------------//
void Set_Soft_Timer(Soft_Timer_Handle timer_id,u32 tick_count)
{
	Soft_Timer_List[timer_id].user_tick_count = tick_count;
	Soft_Timer_List[timer_id].system_user_tick_count = SysTick_count + tick_count;
}


//-------------------------------软件定时器的初始化------------------------------------//
/**
 * @brief   在所有RTOS任务中，软件定时器守护任务的优先级最高，RTOS心跳中断中判断到有软件定时器ready会触发该任务，
            在该任务中进行调用软件定时器对应的回调函数
 * @param   
		*timer_id：         软件定时器的id变量地址
		timer_switch_flag： 开启或关闭软件定时器
		user_tick_count：   用户指定软件定时器的定时值(RTOS心跳节拍数)
		timer_function      软件定时器对应回调函数
 * @retval  NO                      
 */
void Soft_Timer_Init(Soft_Timer_Handle *timer_id,Timer_Switch timer_switch_flag,u32 user_tick_count,void (*timer_function)(void))
{
	static u16 timer_id_init=0;
	configASSERT(timer_id_init < Soft_Timer_NUM);
	
	*timer_id = (timer_id_init++);
	Soft_Timer_List[*timer_id].start_timer_flag = timer_switch_flag;
	Soft_Timer_List[*timer_id].user_tick_count = user_tick_count;
	Soft_Timer_List[*timer_id].system_user_tick_count = SysTick_count + user_tick_count;
	Soft_Timer_List[*timer_id].callback_function = timer_function;
}

