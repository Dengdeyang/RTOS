#ifndef __SOFT_TIMER_H
#define	__SOFT_TIMER_H

#include "stm32f10x.h"
#include "RTOS.h"

//-------------软件定时器结构体---------------------//
typedef struct
{
	u8   timer_statue;
	u8   start_timer_flag;
	u32  system_user_tick_count;
	u32  user_tick_count;
    void (*callback_function)(void);	
}Soft_Timer_Unit;

//------------软件定时器开启或关闭枚举值------------//
typedef enum  
{
	stop_timer = 0,
	start_timer,
}Timer_Switch;

typedef u16  Soft_Timer_Handle;
extern Soft_Timer_Unit Soft_Timer_List[Soft_Timer_NUM];
extern Task_Handle timer_guard_task_id;

void soft_timer_guard_task(void);
void Soft_Timer_Init(Soft_Timer_Handle *timer_id,Timer_Switch timer_switch_flag,u32 user_tick_count,void (*timer_function)(void));
void Start_Soft_Timer(Soft_Timer_Handle timer_id);
void Stop_Soft_Timer(Soft_Timer_Handle timer_id);
void Set_Soft_Timer(Soft_Timer_Handle timer_id,u32 tick_count);	
#endif 
