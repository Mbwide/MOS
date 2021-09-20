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
#include "ARMCM4.h"
#include "mos_init.h"
#include "mos_task.h"
#include "mos_port.h"

/* ��־λ */
mos_uint8_t flag1;
mos_uint8_t flag2;
mos_uint8_t flag3;

/* ������ƿ� */
mos_tcb_t task1_tcb;
mos_tcb_t task2_tcb;
mos_tcb_t task3_tcb;

/* �����ʱ��ģ���������� */
void delay (uint32_t count)
{
    for(; count != 0; count--);
}

/* ����1 */
void task1_entry(void)
{
    for ( ;; )
    {
        flag1 = 1;
        mos_task_delay(1);
        flag1 = 0;
        mos_task_delay(1);
    }
}

/* ����2 */
void task2_entry(void)
{
    for ( ;; )
    {
        flag2 = 1;
        delay(90000);
		flag2 = 0;
		mos_task_suspend(&task2_tcb);
		flag2 = 1;
        mos_task_delay(1);
    }
}

/* ����3 */
void task3_entry(void)
{
    for ( ;; )
    {
        flag3 = 1;
		delay(90000);
		//mos_task_delay(1);
        flag3 = 0;
		mos_task_suspend(&task3_tcb);
        mos_task_delay(1);
    }
}

/**
  * @brief  ������
  *
  * @note
  */
int main()
{
    mos_port_entry_critical();
    mos_init_core();

    mos_task_create(&task1_tcb,					
                    task1_entry,
                    1UL,
                    512UL);

    mos_task_create(&task2_tcb,
                    task2_entry,
                    2UL,
                    512UL);

    mos_task_create(&task3_tcb,
                    task3_entry,
                    3UL,
                    512UL);
					
    mos_task_scheduler_start();
	
    mos_port_exit_critical();

    while(1)
    {
       /* �������е��� */
    }
}

