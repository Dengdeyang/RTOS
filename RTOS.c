/********************************************************************************
  * @file    RTOS.c
  * @author  ddy
  * @version V0.1
  * @date    2020-05-05
  * @brief   RTOS内核开发
  * @atteration   堆区必须定义较大，满足malloc申请
  ********************************************************************************/
#include "bsp_led.h"   
#include "UART.h"
#include "RTOS.h"
#include "stdlib.h"
#include "string.h"

void Idle_task(void);
int __svc(0x00) SVC(void);
//-------------------------------------------------------------------------------
Task_Unit Task_List[TASK_NUM]={0};
uint32_t PSP_array[TASK_NUM]; 

uint32_t SysTick_count = 0;
uint32_t current_task_id = 0;
uint32_t next_task_id = 1;

//--------------------------------------------------------------------------------
u8 Task_Create( 
				u16 *task_id,
				char *task_name,
				u8 task_statue,
				u16 task_priority,
				u32 task_delay_ms,
				u16 task_stack_size_words,
				void *task_function	
			  )
{
	static u16 task_id_index=0;
	
	Task_List[task_id_index].task_id_point = task_id;
	Task_List[task_id_index].task_name = task_name;
	Task_List[task_id_index].task_statue = task_statue;
	Task_List[task_id_index].task_pend_statue = TASK_READY;
	Task_List[task_id_index].task_priority = task_priority;
	Task_List[task_id_index].task_delay_ms = task_delay_ms;
	Task_List[task_id_index].task_stack = (unsigned int *) calloc(task_stack_size_words,sizeof(unsigned int));
	Task_List[task_id_index].task_stack_size_words = task_stack_size_words;
	Task_List[task_id_index].task_function = task_function;
	if(Task_List[task_id_index].task_stack != NULL) 
	{
		task_id_index++;
		return PASS;
	}
	else
	{
		printf("Not enough memory for task stack!\r\n");
		return FAIL;
	}
}

//------------------------临界段代码保护-------------------------------------
void SVC_Handler(void)
{
	__set_CONTROL(0x02);
}

void taskENTER_CRITICAL(void)
{
	SVC();
	__set_PRIMASK(0x01);
}

void taskEXIT_CRITICAL(void)
{
	__set_PRIMASK(0x00);
	__set_CONTROL(0x03);
}

//------------------------任务的挂起与恢复-------------------------------------
void Pend_Task(Task_Handle Task_x)
{
	Task_List[Task_x].task_pend_statue = TASK_BLOCK;
	Task_List[Task_x].task_statue = TASK_BLOCK;
}

void Release_Task(Task_Handle Task_x)
{
	Task_List[Task_x].task_pend_statue = TASK_READY;
	Task_List[Task_x].task_statue = TASK_READY;
}

//------------------------滴答定时器中断服务函数-------------------------------
void SysTick_Handler(void)
{
	int i;
	char Task_Flag = 1;
	
	SysTick_count++;
	for(i=TASK_NUM-1;i>=0;i--)
	{
		if((Task_List[i].task_delay_ms == SysTick_count)&&(Task_List[i].task_pend_statue)) 
		{
			Task_List[i].task_statue = TASK_READY;
		}
		if((Task_List[i].task_statue)&Task_Flag)   
		{
			next_task_id = i;
			Task_Flag=0;
		}
	}
	if(current_task_id != next_task_id)
	{
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
	}
	else return;
}

//-----------------------任务延时函数-------------------------//
void RTOS_Delay(u16 task_id,uint32_t delay_ms)
{
	Task_List[task_id].task_statue = TASK_BLOCK;
	Task_List[task_id].task_delay_ms = SysTick_count + delay_ms;
	
	while(Task_List[task_id].task_statue == TASK_BLOCK);
}

//---------------------------------操作系统初始化-----------------------------------------------	
void RTOS_Init(void)
{
	u16 i,j;
	Task_Unit task_temp;
	
	SCB->CCR = SCB_CCR_STKALIGN;//使能双字栈对齐

	//-----------------根据优先级对任务列表进行排序------------------//
	for(i=0;i<TASK_NUM;i++)
	{
		for(j=i+1;j<TASK_NUM;j++)
		{
			if(Task_List[i].task_priority > Task_List[j].task_priority) 
			{
				task_temp = Task_List[j];
				Task_List[j] = Task_List[i];
				Task_List[i] = task_temp;
			}
		}
	}
	//--------------初始化任务ID和关联任务列表与系统栈---------------//
	for(i=0;i<TASK_NUM;i++)
	{
		*(Task_List[i].task_id_point) = i;
		
		PSP_array[i] = ((unsigned int) Task_List[i].task_stack)+(Task_List[i].task_stack_size_words * sizeof(unsigned int))-16*4;
		HW32_REG((PSP_array[i] + (14<<2))) = (unsigned long) Task_List[i].task_function;
		HW32_REG((PSP_array[i] + (15<<2))) = 0x01000000;
	}
	//---------------初始化操作系统-------------------------------//
	current_task_id = 0;
	__set_PSP((PSP_array[current_task_id] + 16*4));
	NVIC_SetPriority(PendSV_IRQn,0xFF);
	SysTick_Config(SysTick_Rhythm*(SystemCoreClock/1000000));
	__set_CONTROL(0x03);
	__ISB();
	Idle_task();
}


__ASM void PendSV_Handler(void)//有硬件fault
{
    MRS R0 , PSP //把PSP值读到R0
	STMDB R0!,{R4 - R11}//R4~R11中的数据依次存入PSP所指地址处，每存一次R0更新地址，记录PSP当前值
	LDR R1,=__cpp(&current_task_id);//把C语言中全局变量current_task_id的地址存入R1，汇编调用C变量需用cpp函数
	LDR R2,[R1]//将current_task_id数值存入R2中
	LDR R3, =__cpp(&PSP_array);//把C语言中全局变量PSP_array的地址存入R1，汇编调用C变量需用cpp函数
	STR R0,[R3,R2,LSL #2]//R0数据加载到地址=R3+R2<<2处(PSP当前地址存到PSP_array+current_task_id*2处)
	
	LDR R4,=__cpp(&next_task_id);//把C语言中全局变量next_task_id的地址存入R4，汇编调用C变量需用cpp函数
	LDR R4,[R4]//将next_task_id数值存入R4中
	STR R4,[R1]//将current_task_id = next_task_id
	LDR R0,[R3,R4,LSL #2]//将地址=R3+R2<<2处数据加载到R0，(PSP_array+next_task_id+2处数据加载到PSP处)
	LDMIA R0!,{R4 - R11}//PSP所指地址处读出数据加载到R4~R11中，每存一次R0更新地址，记录PSP当前值
	MSR PSP,R0  //R0所指地址加载到PSP
	BX LR      //PC返回到LR所指处，该函数调用前地址
	ALIGN 4    //下一条指令或数据对齐到4字节地址处，空位补0
}
/*********************************************END OF FILE**********************/
