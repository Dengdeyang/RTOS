#ifndef __RTOS_H
#define	__RTOS_H

#include "stm32f10x.h"

#define PASS 1
#define FAIL 0

#define TASK_READY 1
#define TASK_BLOCK 0

#define	USER_TASK_NUM   4
#define	TASK_NUM   (USER_TASK_NUM+1)

#define SysTick_Rhythm  1000  //us
#define HW32_REG(ADDERSS) (*((volatile unsigned long *)(ADDERSS)))
#define stop_cpu __breakpoint(0)

//---------------------------------------------------------------------
typedef struct
{
	u16 *task_id_point;
	char *task_name;
	u8 task_statue;
	u8 task_pend_statue;
	u16 task_priority; 
	u32 task_delay_ms;
	unsigned int *task_stack;
	u16 task_stack_size_words;
	void *task_function;	
}Task_Unit;

void RTOS_Init(void);
void RTOS_Delay(u16 task_id,uint32_t delay_ms);
void Task_Creat_Init(void);
u8 Task_Create( 
				u16 *task_id,
				char *task_name,
				u8 task_statue,
				u16 task_priority,
				u32 task_delay_ms,
				u16 task_stack_size_words,
				void *task_function	
			  );
								
typedef u16  Task_Handle;
extern Task_Unit Task_List[TASK_NUM];	

void taskENTER_CRITICAL(void);
void taskEXIT_CRITICAL(void);
void Release_Task(Task_Handle Task_x);
void Pend_Task(Task_Handle Task_x);				
#endif 
