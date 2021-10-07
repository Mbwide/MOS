/**
  ******************************************************************************
  * @file    main.c
  * @author  Mbwide
  * @version V1.0
  * @date    2021-08-01
  * @brief   ������
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

/* ��־λ */
mos_uint8_t flag1;
mos_uint8_t flag2;
mos_uint8_t flag3;

/* ������ƿ� */
mos_tcb_t task1_tcb;
mos_tcb_t task2_tcb;
mos_tcb_t task3_tcb;

/* ͬ���ź��� */
mos_sync_t *sync_1;
/* �����ź��� */
mos_mutex_t *mutex_1;

/* �����ʱ��ģ���������� */
void delay (uint32_t count)
{
    for(; count != 0; count--);
}

/* ��ʱ��3�жϷ����� */
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)   //����ж�
    {
        //delay(5000);
        mos_base_t temp = mos_port_interrupt_disable_temp();
        //mos_printf("TIM3���.......\r\n");
        mos_port_interrupt_enable_temp(temp);
    }

    TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //����жϱ�־λ
}

/* ��ʱ��5�жϷ����� */
void TIM5_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM5, TIM_IT_Update) == SET)   //����ж�
    {
        mos_ipc_sync_give(sync_1);
        mos_base_t temp = mos_port_interrupt_disable_temp();
        //mos_printf("TIM555���.......\r\n");
        mos_port_interrupt_enable_temp(temp);
    }

    TIM_ClearITPendingBit(TIM5, TIM_IT_Update); //����жϱ�־λ
}

////�жϲ���������
//void task2_entry(void)
//{
//	static u32 total_num=0;
//    while(1)
//    {
//		total_num+=1;
//		if(total_num%5 == 0)
//		{
//			printf("�ر��ж�.............\r\n");
//			mos_base_t temp = mos_port_interrupt_disable_temp();
//			delay(50000000);						//��ʱ5s
//			printf("���ж�.............\r\n");//���ж�
//			mos_port_interrupt_enable_temp(temp);
//		}
//        LED1=~LED1;
//        mos_task_delay(100);
//    }
//}

/* ����1 */
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

/* ����2 */
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

/* ����3 */
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
  * @brief  ������
  *
  * @note
  */
int main()
{
    /* ����ϵͳ�ж����ȼ�����4 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    mos_port_entry_critical();
    mos_init_core();

    /* ��ʼ��LED */
    led_init();

    TIM3_Int_Init(30000 - 1, 7200 - 1);		//��ʼ����ʱ��3����ʱ������1S
    TIM5_Int_Init(30000 - 1, 7200 - 1);		//��ʼ����ʱ��5����ʱ������1S

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
        /* �������е��� */
    }
}

