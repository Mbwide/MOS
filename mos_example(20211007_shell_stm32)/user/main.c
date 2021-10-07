/**
  ******************************************************************************
  * @file    main.c
  * @author  Mbwide
  * @version V1.0
  * @date    2021-08-01
  * @brief   主函数
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */
#include "mos_init.h"
#include "mos_task.h"
#include "mos_port.h"
#include "mos_ipc.h"
#include "mos.h"

#include "bsp_sys.h"
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_timer.h"

#define LED0 PAout(8)
#define LED1 PDout(2)

/* 标志位 */
mos_uint8_t flag1;
mos_uint8_t flag2;
mos_uint8_t flag3;

/* 任务控制块 */
mos_tcb_t task1_tcb;
mos_tcb_t task2_tcb;
mos_tcb_t task3_tcb;

/* 同步信号量 */
mos_sync_t *sync_1;
/* 互斥信号量 */
mos_mutex_t *mutex_1;

/* 软件延时，模拟任务运行 */
void delay (uint32_t count)
{
    for(; count != 0; count--);
}

/* 定时器3中断服务函数 */
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)   //溢出中断
    {
        //delay(5000);
        mos_base_t temp = mos_port_interrupt_disable_temp();
        //mos_printf("TIM3输出.......\r\n");
        mos_port_interrupt_enable_temp(temp);
    }

    TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //清除中断标志位
}

/* 定时器5中断服务函数 */
void TIM5_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM5, TIM_IT_Update) == SET)   //溢出中断
    {
        mos_ipc_sync_give(sync_1);
        mos_base_t temp = mos_port_interrupt_disable_temp();
        //mos_printf("TIM555输出.......\r\n");
        mos_port_interrupt_enable_temp(temp);
    }

    TIM_ClearITPendingBit(TIM5, TIM_IT_Update); //清除中断标志位
}

////中断测试任务函数
//void task2_entry(void)
//{
//	static u32 total_num=0;
//    while(1)
//    {
//		total_num+=1;
//		if(total_num%5 == 0)
//		{
//			printf("关闭中断.............\r\n");
//			mos_base_t temp = mos_port_interrupt_disable_temp();
//			delay(50000000);						//延时5s
//			printf("打开中断.............\r\n");//打开中断
//			mos_port_interrupt_enable_temp(temp);
//		}
//        LED1=~LED1;
//        mos_task_delay(100);
//    }
//}

/* 任务1 */
void task1_entry(void)
{
    for ( ;; )
    {
        led2_on();
        mos_task_delay(200);
        mos_ipc_mutex_take(mutex_1, MOS_MAX_DELAY);
        //mos_printf("task1_start\r\n");
        delay(5000000);
        //mos_printf("task1_end\r\n");
        mos_ipc_mutex_give(mutex_1);
        led2_off();
        mos_task_delay(200);
    }
}

/* 任务2 */
void task2_entry(void)
{
    while(1)
    {
        LED1 = ~LED1;
        //mos_ipc_sync_take(sync_1, 20000);
        mos_ipc_mutex_take(mutex_1, MOS_MAX_DELAY);
        //mos_printf("task2_start\r\n");
        delay(5000000);
        //mos_printf("task2_end\r\n");
        mos_ipc_mutex_give(mutex_1);

        mos_task_delay(100);
    }
}

/* 任务3 */
void task3_entry(void)
{
    for ( ;; )
    {
        flag3 = 1;
        mos_task_delay(1000);
        flag3 = 0;
        mos_task_delay(1000);
    }
}

/**
  * @brief  主函数
  *
  * @note
  */
int main()
{
    /* 设置系统中断优先级分组4 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    mos_port_entry_critical();
    mos_init_core();

    /* 初始化LED */
    led_init();

    TIM3_Int_Init(30000 - 1, 7200 - 1);		//初始化定时器3，定时器周期1S
    TIM5_Int_Init(30000 - 1, 7200 - 1);		//初始化定时器5，定时器周期1S

    sync_1 = mos_ipc_sync_creat();
    mutex_1 = mos_ipc_mutex_creat();

    mos_task_create(&task1_tcb,
                    task1_entry,
                    "TASK_1",
                    1UL,
                    512UL);

    mos_task_create(&task2_tcb,
                    task2_entry,
                    "TASK_2",
                    2UL,
                    512UL);

    mos_task_create(&task3_tcb,
                    task3_entry,
                    "TASK_3",
                    3UL,
                    512UL);

    mos_task_scheduler_start();

    mos_port_exit_critical();

    while(1)
    {
        /* 不会运行到这 */
    }
}

