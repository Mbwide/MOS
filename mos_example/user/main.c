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
#include "ARMCM4.h"
#include "mos_init.h"
#include "mos_task.h"
#include "mos_port.h"

ALIGN(MOS_CONFIG_ALIGN_SIZE)
mos_uint8_t task1_stack_size[512];
mos_uint8_t task2_stack_size[512];
mos_uint8_t task3_stack_size[512];
mos_uint8_t task4_stack_size[512];

mos_uint8_t flag1;
mos_uint8_t flag2;
mos_uint8_t flag3;
mos_uint8_t flag4;

mos_tcb_t task1_tcb;
mos_tcb_t task2_tcb;
mos_tcb_t task3_tcb;
mos_tcb_t task4_tcb;

/* 软件延时 */
void delay (uint32_t count)
{
    for(; count != 0; count--);
}

/* 任务1 */
void task1_entry(void *p_arg)
{
    for ( ;; )
    {
        flag1 = 1;
        delay (100);
        flag1 = 0;
        delay (100);
    }
}

/* 任务2 */
void task2_entry(void *p_arg)
{
    for ( ;; )
    {
        flag2 = 1;
        mos_task_delay(4);
        flag2 = 0;
        mos_task_delay(4);
    }
}

/* 任务3 */
void task3_entry(void *p_arg)
{
    for ( ;; )
    {
        flag3 = 1;
        mos_task_delay(4);
        flag3 = 0;
        mos_task_delay(4);
    }
}

/* 任务3 */
void task4_entry(void *p_arg)
{
    for ( ;; )
    {
        flag4 = 1;
        mos_task_delay(4);
        flag4 = 0;
        delay(150000);
    }
}
/**
  * @brief  主函数
  *
  * @note
  */
int main()
{
	//SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
    mos_base_t temp = mos_port_entry_critical();
    mos_init_core();

//    mos_task_create(&task1_tcb,
//                    (task_entry_fun)task1_entry,
//                    "task_1",
//                    1,
//                    (mos_uint32_t)NULL,
//                    512,
//                    (mos_uint32_t)task1_stack_size);

    mos_task_create(&task2_tcb,
                    (task_entry_fun)task2_entry,
                    "task_2",
                    2,
                    (mos_uint32_t)NULL,
                    512,
                    (mos_uint32_t)task2_stack_size);

    mos_task_create(&task3_tcb,
                    (task_entry_fun)task3_entry,
                    "task_3",
                    3,
                    (mos_uint32_t)NULL,
                    512,
                    (mos_uint32_t)task3_stack_size);

    mos_task_create(&task4_tcb,
                    (task_entry_fun)task4_entry,
                    "task_4",
                    1,
                    (mos_uint32_t)NULL,
                    512,
                    (mos_uint32_t)task4_stack_size);
					
    mos_task_scheduler_start();
    mos_port_exit_critical(temp);

    while(1)
    {

    }
}

