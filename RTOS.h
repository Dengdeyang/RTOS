#ifndef __RTOS_H
#define	__RTOS_H
#include "stm32f10x.h"

//----------------操作系统裁剪--------------------
#define	USER_TASK_NUM   8     //用户指定任务数(不含空闲任务)
#define Soft_Timer_NUM  3     //用户指定软件定时器个数
#define SysTick_Rhythm  1000  //用户指定操作系统心跳时钟周期，单位us
//--------------------------------------------

//断言
#define vAssertCalled(char,int) printf("Error:%s,%d\r\n",char,int)
#define configASSERT(x) if((x)==0) vAssertCalled(__FILE__,__LINE__)
	

#define PASS 1
#define FAIL 0
#define TASK_READY 1
#define TASK_BLOCK 0
#define	TASK_NUM   (USER_TASK_NUM+2)   //system task number is 2: idle_task ,timer_guard_task
#define HW32_REG(ADDERSS) (*((volatile unsigned long *)(ADDERSS)))
#define stop_cpu __breakpoint(0)

//---------------------------------------------------------------------//
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


typedef u16  Task_Handle;
extern Task_Unit Task_List[TASK_NUM];	
extern uint32_t SysTick_count;

//---------------------------------------------------------------------//
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
void taskENTER_CRITICAL(void);
void taskEXIT_CRITICAL(void);
void Release_Task(Task_Handle Task_x);
void Pend_Task(Task_Handle Task_x);						
#endif 
