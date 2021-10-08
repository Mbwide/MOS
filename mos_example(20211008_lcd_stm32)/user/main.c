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
#include "bsp_delay.h"
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_timer.h"
#include "lcd.h"

/* 标志位 */
mos_uint8_t flag1;
mos_uint8_t flag2;
mos_uint8_t flag3;

/* 任务控制块 */
mos_tcb_t task1_tcb;
mos_tcb_t task2_tcb;
mos_tcb_t task3_tcb;

/* 互斥信号量 */
mos_mutex_t *mutex_1;

/* LCD刷屏时使用的颜色 */
int lcd_discolor[14] = { WHITE, BLACK, BLUE,  BRED,
                         GRED,  GBLUE, RED,   MAGENTA,
                         GREEN, CYAN,  YELLOW, BROWN,
                         BRRED, GRAY
                       };

/* 任务1 High Task */
void high_task_entry(void)
{
    mos_uint8_t max_r = 24;
    POINT_COLOR = BLACK;
    LCD_DrawRectangle(5, 184, 115, 314);
    LCD_DrawLine(5, 204, 115, 204);
    LCD_ShowString(25, 186, 110, 16, 16, "High Task");
    LCD_Fill(6, 205, 114, 313, BLUE); 	//填充区域

    while(1)
    {
		/* 获取互斥量 */
        mos_ipc_mutex_take(mutex_1, MOS_MAX_DELAY);
        led1_on();

        for (int i = 1; i < max_r; i++)
        {
            LCD_Fill(30, 229, 90, 289, BLUE);
            LCD_Draw_Circle(60, 259, i);
            mos_task_delay(50);
        }

        for (int i = max_r; i > 0; i--)
        {
            LCD_Fill(30, 229, 90, 289, BLUE);
            LCD_Draw_Circle(60, 259, i);
            mos_task_delay(50);
        }
		
        /* 施放互斥量 */
        mos_ipc_mutex_give(mutex_1);
        led1_off();
        mos_task_delay(10);
    }
}

/* 任务2 Middle Task */
void middle_task_entry(void)
{
    mos_uint8_t max_r = 24;
    POINT_COLOR = BLACK;
    LCD_DrawRectangle(125, 184, 234, 314);
    LCD_DrawLine(125, 204, 234, 204);
    LCD_ShowString(137, 186, 110, 16, 16, "Middle Task");
    LCD_Fill(126, 205, 233, 313, BRED);

    while(1)
    {
		/* 获取互斥量 */
        mos_ipc_mutex_take(mutex_1, MOS_MAX_DELAY);
        led2_on();

        for (int i = 1; i < max_r; i++)
        {
            LCD_Fill(149, 229, 209, 289, BRED);
            LCD_Draw_Circle(179, 259, i);
            mos_task_delay(50);
        }

        for (int i = max_r; i > 0; i--)
        {
            LCD_Fill(149, 229, 209, 289, BRED);
            LCD_Draw_Circle(179, 259, i);
            mos_task_delay(50);
        }
		
        /* 施放互斥量 */
        mos_ipc_mutex_give(mutex_1);
        
		led2_off();
        mos_task_delay(10);
    }
}

/* 任务3 Low Task*/
void low_task_entry(void)
{
    mos_uint8_t max_r = 24;
    POINT_COLOR = BLACK;
    LCD_DrawRectangle(65, 70, 175, 180);
    LCD_DrawLine(65, 90, 175, 90);
    LCD_ShowString(89, 71, 110, 16, 16, "Low Task");
    LCD_Fill(66, 91, 174, 179, GRED); 	//填充区域

    while(1)
    {
        for (int i = 1; i < max_r; i++)
        {
            LCD_Fill(80, 110, 154, 169, GRED); 	//填充区域
            LCD_Draw_Circle(120, 135, i);
            mos_task_delay(50);
        }

        for (int i = max_r; i > 0; i--)
        {
            LCD_Fill(80, 110, 154, 169, GRED); 	//填充区域
            LCD_Draw_Circle(120, 135, i);
            mos_task_delay(50);
        }
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

    /* 进入临界区 */
    mos_port_entry_critical();
	/* MOS内核初始化 */
    mos_init_core();

    /* 初始化LED */
    led_init();
    /* 初始化延时函数 */
    delay_init();
    /* 初始化LCD */
    LCD_Init();

    LCD_ShowString(10, 10, 200, 16, 16, "STM32F103");
    LCD_ShowString(10, 30, 200, 16, 16, "MOS Example");
    LCD_ShowString(10, 50, 200, 16, 16, "2021/10/08");

	/* 互斥量初始化 */
    mutex_1 = mos_ipc_mutex_creat();

    mos_task_create(&task1_tcb,
                    high_task_entry,
                    "High Task",
                    1UL,
                    512UL);

    mos_task_create(&task2_tcb,
                    middle_task_entry,
                    "Middle Task",
                    2UL,
                    512UL);

    mos_task_create(&task3_tcb,
                    low_task_entry,
                    "Low Task",
                    3UL,
                    512UL);

    mos_task_scheduler_start();

    mos_port_exit_critical();

    while(1)
    {
        /* 不会运行到这 */
    }
}

