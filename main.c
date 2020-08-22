#include "stm32f10x.h"
#include "bsp_led.h"
#include "UART.h"
#include "string.h"
#include "stdlib.h"
#include "RTOS.h"
#include "semaphore.h" 

//------------------------------------------------------------//
Task_Handle idle_task,Task1,Task2,Task3,Task4;

Semaphore_Handle Binary_Semaphore1;
//------------------------------------------------------------//
void BSP_Init(void)
{
	USART1_Config();
	LED_GPIO_Config();	
}
//------------------------------------------------------------//
int main(void)
{	
    BSP_Init();
	Task_Creat_Init();
	RTOS_Init();
	while(1)
	{
		stop_cpu;
	}
}
//------------------------------------------------------------------------------
void Idle_task(void)
{
	while(1)
	{
		//printf("//-------------Idle_task-----------------//\r\n");
	}
}
//------------------------------------------------------------//
void task1(void)
{
	while(1)
	{
		printf("111111111111\r\n");
		if(CMD =='1') 
		{
			CMD = 1;
			LED1_TOGGLE;
			if(Semaphore_Take(Task1,Count_Semaphore,&Binary_Semaphore1,MAX_DELAY))        printf("Task1 Binary_Semaphore_Take OK \r\n");
			else                                                         printf("Task1 Binary_Semaphore_Take Faik \r\n");
			
			printf("Task1 资源访问OK \r\n");
		}
		RTOS_Delay(Task1,100);
	}
}

//------------------------------------------------------------//
void task2(void)
{
	while(1)
	{
		printf("22222222222222222222222222222\r\n");
		if(CMD =='2') 
		{
			CMD = 2;
			LED1_TOGGLE;
			Semaphore_Give(Count_Semaphore,&Binary_Semaphore1);
		}
		RTOS_Delay(Task2,500);
	}
}

//------------------------------------------------------------//
void task3(void)
{
	while(1)
	{
		printf("3333333333333333333333333333333333333333333\r\n");
		if(CMD =='3') 
		{
			CMD = 3;
			LED1_TOGGLE;
			if(Semaphore_Take(Task3,Count_Semaphore,&Binary_Semaphore1,MAX_DELAY))        printf("Task3 Binary_Semaphore_Take OK \r\n");
			else                                                         printf("Task3 Binary_Semaphore_Take Faik \r\n");
			
			printf("Task3 解除阻塞 \r\n");

		}
		RTOS_Delay(Task3,1000);
	}
}

//------------------------------------------------------------//
void task4(void)
{
	while(1)
	{
		printf("444444444444444444444444444444444444444444444444444444444444444444444444\r\n");
		if(CMD =='4') 
		{
			CMD = 4;
			LED1_TOGGLE;
			Pend_Task(Task1);
		}
		
		if(CMD =='5') 
		{
			CMD = 5;
			LED1_TOGGLE;
			Release_Task(Task1);
		}
		RTOS_Delay(Task4,1500);
	}
}

//------------------------------------------------------------//
//空闲任务需首先定义，空闲任务函数可自由定义
void Task_Creat_Init(void)
{
	Task_Create(&idle_task,"Idle_task",TASK_READY,0,0,50,Idle_task);		
	
	//*task_id,*task_name,task_statue,task_priority,task_delay_ms,task_stack_size_words,*task_function	
	Task_Create(&Task1,"task1",TASK_READY,2,0,100,task1);//改为任务句柄
	Task_Create(&Task2,"task2",TASK_READY,6,0,100,task2);
	Task_Create(&Task3,"task3",TASK_READY,1,0,100,task3);
	Task_Create(&Task4,"task4",TASK_READY,4,0,100,task4);
}
/*********************************************END OF FILE**********************/
