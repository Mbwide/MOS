/**
  ******************************************************************************
  * Micro OS V1.0 - Copyright (C) 2021 Real Time Engineers Ltd.
  * All rights reserved.
  * This file is part of Micro OS.
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  ******************************************************************************
  */
/**
  ******************************************************************************
  * @file    mos_tick.c
  * @version V1.0.1
  * @date    2021-10-04
  * @brief   定时器
  ******************************************************************************
  * @note
  *          2021-08-20 Mbwide:初始版本
  *          2021-10-04 Mbwide:结构优化
  ******************************************************************************
  */

#include "mos_tick.h"
#include "mos_sys.h"
#include "mos_task.h"
#include "mos_port.h"

/* Private -------------------------------------------------------------------*/
/* 系统当前时基计数器 */
static mos_tick_t g_mos_task_cur_tick_count = 0;
/* 任务延时溢出列表指针 */
static mos_list_t * volatile g_mos_task_delay_overflow_list;


/* Public --------------------------------------------------------------------*/
/* 下一个任务解除阻塞时间 */
volatile mos_uint32_t g_mos_task_next_task_unblock_tick = 0U;

/* 任务延时列表指针 */
extern mos_list_t * volatile g_mos_task_delay_list;
/* 任务就绪表 */
extern mos_list_t  g_mos_task_list_ready_table[MOS_CONFIG_TASK_PRIORITY_MAX];
/* 当前正在运行任务的任务控制块 */
extern mos_tcb_t * volatile g_cur_task_tcb;


/* Private Fun----------------------------------------------------------------*/
/**
 * @brief 切换延时列表后复位下一个任务阻塞时间
 */
static void mos_task_reset_next_task_unblock_time(void)
{
    mos_tcb_t *mos_tcb;

    /* 新延时列表为空时设置阻塞时间为最大值 */
    if (mos_list_is_empty(g_mos_task_delay_list))
    {
        g_mos_task_next_task_unblock_tick = MOS_MAX_DELAY;
    }
    /* 新延时列表不为空时获取下一个任务阻塞时间*/
    else
    {
        mos_tcb = MOS_LIST_ENTRY(g_mos_task_delay_list->next, mos_tcb_t, task_list);
        g_mos_task_next_task_unblock_tick = mos_tcb->task_tick_wake;
    }
}

/**
 * @brief 系统时基计数器溢出切换延时列表
 */
static void mos_task_switch_delay_list(void)
{
    /* 交换任务延时列表指针和任务延时溢出列表指针 */
    mos_list_t *list_temp;
    list_temp = g_mos_task_delay_list;
    g_mos_task_delay_list = g_mos_task_delay_overflow_list;
    g_mos_task_delay_overflow_list = list_temp;

    /* 任务溢出次数计数 */
    /* g_num_of_over_flow++; */

    /* 切换延时列表后复位下一个任务阻塞时间 */
    mos_task_reset_next_task_unblock_time();
}

/**
 * @brief  系统当前时基计数器计数增加
 *
 * @return 是否执行任务调度函数
 */
static mos_bool_t mos_tick_tickcount_increase(void)
{
    mos_tcb_t *mos_tcb;
    g_mos_task_cur_tick_count++;

    /*是否调度*/
    mos_bool_t switch_flag = FALSE;

    /* 如果g_mos_task_cur_tick_count溢出，则切换延时列表 */
    if (g_mos_task_cur_tick_count == (mos_tick_t)0U)
    {
        mos_task_switch_delay_list();
    }

    if (g_mos_task_cur_tick_count >= g_mos_task_next_task_unblock_tick)
    {
        for ( ;; )
        {
            /* 延时列表为空，设置g_mos_task_next_task_unblock_tick为可能的最大值 */
            if(mos_list_is_empty(g_mos_task_delay_list))
            {
                g_mos_task_next_task_unblock_tick = MOS_MAX_DELAY;
                break;
            }
            /* 延时列表不为空 */
            else
            {
                mos_tcb = MOS_LIST_ENTRY(g_mos_task_delay_list->next, mos_tcb_t, task_list);

                /* 直到将延时列表中所有延时到期的任务移除才跳出for循环 */
                if(g_mos_task_cur_tick_count < mos_tcb->task_tick_wake)
                {
                    g_mos_task_next_task_unblock_tick = mos_tcb->task_tick_wake;
                    break;
                }

                /* 将任务从延时列表移除，消除等待状态 */
                mos_list_node_delete(&mos_tcb->task_list);
                /* 重置任务控制块的task_tick_wake */
                mos_tcb->task_tick_wake = 0L;

                /* 使用IPC时判断是否因为信号量被阻塞 */
#if (MOS_CONFIG_USE_IPC == YES)

                if (!mos_list_is_empty(&mos_tcb->task_ipc_list))
                {
                    mos_list_node_delete(&mos_tcb->task_ipc_list);
                }

#endif

                /* 有更高优先级任务，需要任务切换，数值越大，优先级越低 */
                if (g_cur_task_tcb->task_priority > mos_tcb->task_priority)
                {
                    /* 系统调度标志 */
                    switch_flag = TRUE;
                }

                /* 将解除等待的任务添加到就绪列表 */
                mos_task_insert_ready_table_list(mos_tcb);
            }
        }
    }

    /* 使用时间片，每次都要执行调度程序 */
#if (MOS_CONFIG_USE_TIME_SLICING == YES)
    {
        /* 系统调度标志 */
        switch_flag = TRUE;
    }
#endif

    return switch_flag;
}

/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief 系统时钟初始化
 */
void mos_tick_systick_init(void)
{
    /* systick硬件接口初始化 */
    mos_port_systick_init();

    /* 下一个任务解除阻塞时间初始化为最大延时时间 */
    g_mos_task_next_task_unblock_tick = MOS_MAX_DELAY;
}

/**
 * @brief 系统时钟中断服务函数
 */
void mos_tick_systick_handler(void)
{
    /* 进入中断 */
    mos_sys_interrupt_enter();

    /* 时基更新 */
    if (mos_tick_tickcount_increase() == TRUE)
    {
        /* 任务调度 */
        mos_task_scheduler();
    }

    /* 离开中断 */
    mos_sys_interrupt_leave();
}

/**
 * @brief 获取当前时钟计数值
 */
mos_tick_t mos_tick_get_cur_tick_count(void)
{
    return g_mos_task_cur_tick_count;
}

/**
 * @brief 任务延时处理
 */
void mos_tick_delay_process(mos_tcb_t *mos_tcb, const mos_uint32_t tick)
{
    mos_tick_t mos_task_wake_tick;

    /* 获取系统时基计数器g_mos_tick_cur_tick_count的值 */
    mos_tick_t mos_cur_tick_count = mos_tick_get_cur_tick_count();

    /* 将任务从就绪列表中移除 */
    mos_task_remove_ready_table_list(g_cur_task_tcb);

    /* 计算延时到期时，系统时基计数器mos_cur_tick_count的值是多少 */
    mos_task_wake_tick = mos_cur_tick_count + tick;

    /* 修改任务列表唤醒时间 */
    mos_tcb->task_tick_wake = mos_task_wake_tick;

    /* 修改任务状态为阻塞态 */
    mos_tcb->task_state = TASK_BLOCK;

    /* 溢出 */
    if (mos_task_wake_tick < mos_cur_tick_count )
    {
        mos_task_insert_delay_list(g_mos_task_delay_overflow_list, &(g_cur_task_tcb->task_list), mos_task_wake_tick);
    }
    /* 没有溢出 */
    else
    {
        mos_task_insert_delay_list(g_mos_task_delay_list, &(g_cur_task_tcb->task_list), mos_task_wake_tick);

        /* 更新下一个任务解锁时刻变量g_mos_task_next_task_unblock_tick的值 */
        if (mos_task_wake_tick < g_mos_task_next_task_unblock_tick)
        {
            g_mos_task_next_task_unblock_tick = mos_task_wake_tick;
        }
    }
}

