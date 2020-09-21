/********************************************************************************
  * @file    RTOS.c
  * @author  邓得洋
  * @version V0.1
  * @date    2020-08-03
  * @brief   RTOS内核开发，包含：
  *				1)任务创建函数
  *                             2)临界段代码保护函数
  *				3)任务挂起与恢复函数
  *                             4)RTOS心跳滴答定时器中断服务函数(优先级判决+抢占式任务调度)
  *                             5)RTOS延时函数(释放CPU使用权)
  *                             6)RTOS初始化函数
  * @atteration   启动文件.s中，堆区必须定义较大，在任务创建时，满足任务栈较大或任务数量较多时malloc申请
  ********************************************************************************/
#include "bsp_led.h"   
#include "UART.h"
#include "RTOS.h"
#include "stdlib.h"
#include "string.h"
#include "soft_timer.h" 

//---------------------RTOS 任务列表，系统栈及其他中间全局变量定义区----------------------------//
Task_Unit Task_List[TASK_NUM]={0};
volatile uint32_t PSP_array[TASK_NUM]; 

volatile uint32_t SysTick_count = 0;
volatile uint32_t current_task_id = 0;
volatile uint32_t next_task_id = 1;
void Idle_task(void);
int __svc(0x00) SVC(void);


//---------------------------------RTOS 任务创建函数-----------------------------------------//
/**
  * @brief  动态申请任务栈内存，并对任务列表中对应的任务结构体变量进行初始化。
  * @param  u16 *task_id:              任务id变量地址
			char *task_name:           任务名称，字符串
			u8 task_statue:            任务状态
			u16 task_priority:         任务优先级
			u32 task_delay_ms:         任务延时值
			u16 task_stack_size_words: 任务栈大小(单位:32bit word)
			void *task_function:       任务函数地址
  * @retval 任务创建成功或失败标志                       
  */
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
	
	configASSERT(task_id_index < TASK_NUM);
	
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

//---------------------------------------临界段代码保护-------------------------------------//
//PRIMASK寄存器只能在特权级下访问，故通过触发SVC中断，进入特权级，再设置PRIMASK

//SVC中断服务函数
void SVC_Handler(void)
{
	__set_CONTROL(0x02);//ARM在Handler模式，设置CONTROL[0]=0,进入特权级，线程模式
}

//进入临界段代码保护
void taskENTER_CRITICAL(void)
{
	SVC();//触发SVC中断，ARM进入特权级，Handler模式
	__set_PRIMASK(0x01);//ARM进入特权级，线程模式，置PRIMASK 1bit寄存器1，屏蔽除NMI，硬fault以外的所有异常
}

//退出临界段代码保护
void taskEXIT_CRITICAL(void)
{
	__set_PRIMASK(0x00);//ARM在特权级，线程模式，复位PRIMASK 1bit寄存器，允许中断正常响应
	__set_CONTROL(0x03);//ARM在特权级，线程模式，设置CONTROL[0]=1,进入用户级，线程模式
}


//------------------------------RTOS任务的挂起与恢复----------------------------------------//
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

//------------------------滴答定时器中断服务函数--------------------------------------------//
void SysTick_Handler(void)
{
	int i;
	char Task_Flag = 1;
	
	SysTick_count++;
	
	for(i=0;i<Soft_Timer_NUM;i++)
	{
		if((Soft_Timer_List[i].system_user_tick_count == SysTick_count)&&(Soft_Timer_List[i].start_timer_flag == start_timer)) 
		{
			Soft_Timer_List[i].system_user_tick_count = SysTick_count + Soft_Timer_List[i].user_tick_count;
			Soft_Timer_List[i].timer_statue = TASK_READY;
			Release_Task(timer_guard_task_id);
		}
	}
	
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

//-------------------------------------任务延时函数-------------------------------------------//
void RTOS_Delay(u16 task_id,uint32_t delay_ms)
{
	Task_List[task_id].task_statue = TASK_BLOCK;
	Task_List[task_id].task_delay_ms = SysTick_count + delay_ms;
	
	while(Task_List[task_id].task_statue == TASK_BLOCK);
}


//---------------------------------操作系统初始化-----------------------------------------------//	
void RTOS_Init(void)
{
	u16 i,j;
	u16 priority_temp=0;
	Task_Unit task_temp;
	
	//-------------------创建软件定时器守护任务-------------------------//
	for(i=0;i<TASK_NUM;i++)
	{
		if(Task_List[i].task_priority >= priority_temp) priority_temp = Task_List[i].task_priority;
	}
	Task_Create(&timer_guard_task_id,"timer_guard_task",TASK_BLOCK,priority_temp+1,0,100,soft_timer_guard_task);
	
	//-------------------------------------------------------------//
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
	__ISB();  //指令同步隔离。它会清洗流水线，以保证所有它前面的指令都执行完毕之后，才执行它后面的指令。
	Idle_task();
}

//---------------------------------PendSV中断服务函数,任务上下文切换------------------------------//	

__ASM void PendSV_Handler(void)//有硬件fault
{
        MRS R0 , PSP //把PSP值读到R0
	STMDB R0!,{R4 - R11}//R4~R11中的数据依次存入PSP所指地址处，每次存入前R0减小4byte，
	LDR R1,=__cpp(&current_task_id);//把C语言中全局变量current_task_id的地址存入R1，汇编调用C变量需用cpp函数
	LDR R2,[R1]//将current_task_id数值存入R2中
	LDR R3, =__cpp(&PSP_array);//把C语言中全局变量PSP_array的地址存入R1，汇编调用C变量需用cpp函数
	STR R0,[R3,R2,LSL #2]//R0数据加载到地址=R3+R2<<2处(PSP当前地址存到PSP_array+current_task_id*2处)
	
	LDR R4,=__cpp(&next_task_id);//把C语言中全局变量next_task_id的地址存入R4，汇编调用C变量需用cpp函数
	LDR R4,[R4]//将next_task_id数值存入R4中
	STR R4,[R1]//将current_task_id = next_task_id
	LDR R0,[R3,R4,LSL #2]//将地址=R3+R2<<2处数据加载到R0，(PSP_array+next_task_id*2处数据加载到PSP处)
	LDMIA R0!,{R4 - R11}//PSP所指地址处读出数据加载到R4~R11中，每读出一次后R0增加4byte
	MSR PSP,R0  //R0所指地址加载到PSP
	BX LR      //PC返回到LR所指处，该函数调用前地址
	ALIGN 4    //下一条指令或数据对齐到4字节地址处，空位补0
}
 
/*********************************************END OF FILE**********************/
