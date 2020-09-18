/********************************************************************************
  * @file    main.c
  * @author  邓得洋
  * @version V0.1
  * @date    2020-08-03
  * @brief   main文件完成硬件初始化，RTOS属性变量，任务函数，以及IPC通信中间件定义和初始化
  * @atteration   
  ********************************************************************************/
#include "stm32f10x.h"
#include "bsp_led.h"
#include "UART.h"
#include "string.h"
#include "stdlib.h"
#include "RTOS.h"
#include "semaphore.h" 
#include "event.h"   
#include "soft_timer.h"   

//----------------------RTOS属性变量以及IPC通信中间件定义区---------------------------//
Task_Handle idle_task,Task1,Task2,Task3,Task4,Task5,Task6,Task7,Task8;//RTOS任务id中间变量

Soft_Timer_Handle soft_timer0,soft_timer1,soft_timer2;                //软件定时器id中间变量

Semaphore_Handle Binary_Semaphore1;                                   //信号量定义(二值，计数，互斥)

Queue_Handle Queue_test;                                              //消息队列定义

Event_Handle test_event;                                              //事件标志组定义


//-------------------------板级支持包硬件外设初始化-----------------------------------//
void BSP_Init(void)
{
	USART1_Config();
	LED_GPIO_Config();	
}


//-------------------------------------main主函数-----------------------------------//
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


//------------------------------RTOS 任务函数定义区---------------------------------//

//---------------空闲任务----------------//
void Idle_task(void)
{
	while(1)
	{
		//printf("Idle_task\r\n");
	}
}

//---------------task1-----------------//
void task1(void)
{
	u32 temp;
	while(1)
	{
		printf("task1 is running\r\n");
		LED1_TOGGLE;
		if(CMD =='0') 
		{
			CMD = 0;
			
			if(xEventGroupWaitBits(Task1,&test_event, 0x0A,and_type,hold_type,3000))      printf("Task1 wait Event OK \r\n");
			else                                                                          printf("Task1 wait Event Fail \r\n");
			printf("Task1 解除阻塞 \r\n");
			printf("Task1 wait event data = %X\r\n",test_event.Event);
		}
		
		if(CMD =='1') 
		{
			CMD = 1;
			
			if(QueueReceive(Queue_test,&temp,Task1,3000))             printf("Task1 Queue message Receive OK \r\n");
			else                                                      printf("Task1 Queue message Receive Fail \r\n");
			printf("Task1 解除阻塞 \r\n");
			printf("Task1 Queue message receive data = %d\r\n",temp);
		}
		
		RTOS_Delay(Task1,500);
	}
}

//---------------task2-----------------//
void task2(void)
{
	u32 temp;
	while(1)
	{
		printf("task2 is running\r\n");
		if(CMD =='2') 
		{
			CMD = 2;
			if(xEventGroupWaitBits(Task2,&test_event, 0x0A,or_type,release_type,MAX_DELAY))      printf("Task2 wait Event OK \r\n");
			else                                                                                 printf("Task2 wait Event Fail \r\n");
			
			printf("Task2 解除阻塞 \r\n");
			printf("Task2 wait event data = %X\r\n",test_event.Event);
		}
		if(CMD =='3') 
		{
			CMD = 3;
			if(QueueReceive(Queue_test,&temp,Task2,MAX_DELAY))             printf("Task2 Queue message Receive OK \r\n");
			else                                                           printf("Task2 Queue message Receive Fail \r\n");
			printf("Task2 解除阻塞 \r\n");
			printf("Task2 Queue message receive data = %d\r\n",temp);
		}
		RTOS_Delay(Task2,1000);
	}
}

//---------------task3-----------------//
void task3(void)
{
	while(1)
	{
		printf("task3 is running\r\n");
		if(CMD =='4') 
		{
			CMD = 4;
			Set_Event_Bit(&test_event,1);
		}
		if(CMD =='5') 
		{
			CMD = 5;
			QueueSend(Queue_test,65,Task3,0);
		}
		RTOS_Delay(Task3,1500);
	}
}

//---------------task4-----------------//
void task4(void)
{
	while(1)
	{
		printf("task4 is running\r\n");
		if(CMD =='6') 
		{
			CMD = 6;
			Set_Event_Bit(&test_event,3);	
		}
		
		if(CMD =='7') 
		{
			CMD = 7;
			QueueSend(Queue_test,78,Task4,0);
		}
		RTOS_Delay(Task4,2000);
	}
}

//---------------task5-----------------//
void task5(void)
{
	while(1)
	{
		printf("task5 is running\r\n");
		if(CMD =='8') 
		{
			CMD = 8;
			Pend_Task(Task1);
		}
		if(CMD =='9') 
		{
			CMD = 9;
			if(Semaphore_Take(Task5,Binary_Semaphore,&Binary_Semaphore1,3000))            printf("Task5 Semaphore1 Take OK \r\n");
			else                                                      					  printf("Task5 Semaphore1 Take Fail \r\n");
			printf("Task5 解除阻塞 \r\n");
		}
		
		RTOS_Delay(Task5,2500);
	}
}

//---------------task6-----------------//
void task6(void)
{
	while(1)
	{
		printf("task6 is running\r\n");
		if(CMD =='a') 
		{
			CMD = 90;
			Release_Task(Task1);
		}
		if(CMD =='b') 
		{
			CMD = 91;
			Semaphore_Give(Binary_Semaphore,&Binary_Semaphore1);
		}
		RTOS_Delay(Task6,3000);
	}
}

//---------------task7-----------------//
void task7(void)
{
	while(1)
	{
		printf("task7 is running\r\n");
		if(CMD =='c') 
		{
			CMD = 92;
			if(Semaphore_Take(Task7,Binary_Semaphore,&Binary_Semaphore1,MAX_DELAY))         printf("Task7 Take Semaphore1 OK \r\n");
			else                                                                            printf("Task7 Take Semaphore1 Fail \r\n");
			printf("Task7 解除阻塞 \r\n");
		}
		if(CMD =='d') 
		{
			CMD = 93;
			Semaphore_Give(Binary_Semaphore,&Binary_Semaphore1);
		}
		RTOS_Delay(Task7,3500);
	}
}

//---------------task8-----------------//
void task8(void)
{
	while(1)
	{
		printf("task8 is running\r\n");
		switch(CMD)
		{
			case 'e':
			{
				Start_Soft_Timer(soft_timer0);
				CMD = 0;
			};break;
			
			case 'f':
			{
				Start_Soft_Timer(soft_timer1);
				CMD = 0;
			};break;
			
			case 'g':
			{
				Start_Soft_Timer(soft_timer2);
				CMD = 0;
			};break;
			
			case 'h':
			{
				Stop_Soft_Timer(soft_timer0);
				CMD = 0;
			};break;
			
			case 'i':
			{
				Stop_Soft_Timer(soft_timer1);
				CMD = 0;
			};break;
			
			case 'j':
			{
				Stop_Soft_Timer(soft_timer2);
				CMD = 0;
			};break;
			
			case 'k':
			{
				Set_Soft_Timer(soft_timer2,200);
				CMD = 0;
			};break;
			default:break;
		}
		RTOS_Delay(Task8,500);
	}
}


//------------------------------软件定时器回调函数定义区------------------------------//
void Timer0_callback(void)
{
	printf("Timer0_callback is running\r\n");
	GPIO_B12_TOGGLE
}

void Timer1_callback(void)
{
	printf("Timer1_callback is running\r\n");
	GPIO_B13_TOGGLE
}

void Timer2_callback(void)
{
	printf("Timer2_callback is running\r\n");
	GPIO_B14_TOGGLE
}


//-------------------------RTOS 消息队列、软件定时器、任务创建初始化-----------------------------------//
void Task_Creat_Init(void)
{
	Queue_test = Creat_queue();
	if(Queue_test==NULL) printf("Creat Queue Fail\r\n");
	
	//*timer_id,timer_switch_flag,user_tick_count,*timer_function
	Soft_Timer_Init(&soft_timer0,stop_timer,500,Timer0_callback);
	Soft_Timer_Init(&soft_timer1,stop_timer,1000,Timer1_callback);
	Soft_Timer_Init(&soft_timer2,stop_timer,2000,Timer2_callback);
	
	
	//*task_id,*task_name,task_statue,task_priority,task_delay_ms,task_stack_size_words,*task_function	
	Task_Create(&idle_task,"Idle_task",TASK_READY,0,0,50,Idle_task);		
	Task_Create(&Task1,"task1",TASK_READY,2,0,100,task1);
	Task_Create(&Task2,"task2",TASK_READY,6,0,100,task2);
	Task_Create(&Task3,"task3",TASK_READY,1,0,100,task3);
	Task_Create(&Task4,"task4",TASK_READY,4,0,100,task4);
	Task_Create(&Task5,"task5",TASK_READY,7,0,100,task5);
	Task_Create(&Task6,"task6",TASK_READY,8,0,100,task6);
	Task_Create(&Task7,"task7",TASK_READY,9,0,100,task7);
	Task_Create(&Task8,"task8",TASK_READY,10,0,100,task8);
}

