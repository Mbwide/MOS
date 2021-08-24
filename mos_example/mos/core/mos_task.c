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
  * @file    mos_task.c
  * @version V1.0.0
  * @date    2021-08-20
  * @brief   任务与调度
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#include "mos_task.h"
#include "mos_hw.h"
#include "mos_port.h"


/* Private -------------------------------------------------------------------*/
/* 延时列表，交替使用，用于解决时钟溢出问题 */
static mos_list_t g_mos_task_delay_list_1;
static mos_list_t g_mos_task_delay_list_2;
/* 任务延时列表指针 */
static mos_list_t * volatile g_mos_task_delay_list;
/* 任务延时溢出列表指针 */
static mos_list_t * volatile g_mos_task_delay_overflow_list;

/* 下一个任务解除阻塞时间 */
static volatile mos_uint32_t g_mos_task_next_task_unblock_tick = 0U;
/* 系统当前时基计数器 */
static mos_tick_t g_mos_task_cur_tick_count = 0;

/* 任务就绪表 */
static mos_list_t  g_mos_task_list_ready_table[MOS_CONFIG_TASK_PRIORITY_MAX];
/* 任务优先级标志*/
static volatile mos_uint32_t g_mos_task_priority_flag_32bit;

/* 空闲任务栈 */
static mos_uint8_t mos_task_idle_stack[MOS_CONFIG_TASK_IDLE_STACK_SIZE];
/* 空闲任务 任务控制块 */
static mos_tcb_t idle_tcb;


/* 初始化新创建的任务 */
static void mos_task_init_new(mos_tcb_t *  const mos_new_tcb,	/* 任务控制块指针 */
                              task_entry_fun 	   task_code,		/* 任务入口 */
                              const char * const task_name,		/* 任务名称，字符串形式 */
                              const mos_uint8_t  task_pri,		/* 任务优先级 */
                              const mos_uint32_t parameter,		/* 任务形参 */
                              const mos_uint32_t stack_size,		/* 任务栈大小，单位为字 */
                              const mos_uint32_t stack_start);
/* 空闲任务入口函数 */
static void mos_task_idle_entry(void *p_arg);
/* 初始化空闲任务 */
static void mos_task_idle_init(void);
/* 将任务插入延时列表 */
static void mos_task_insert_delay_list(mos_list_t *delay_list, mos_list_t *node, mos_tick_t mos_task_wake_tick);
/* 任务延时处理 */
static void mos_task_delay_process(const mos_uint32_t tick);
/* 系统时基计数器溢出后切换延时列表 */
static void mos_task_switch_delay_list(void);							  
/* 切换延时列表后复位下一个任务阻塞时间 */
static void mos_task_reset_next_task_unblock_time(void);


/* Public --------------------------------------------------------------------*/
/* 当前正在运行任务的任务控制块 */
mos_tcb_t * volatile g_cur_task_tcb = NULL;


/* define --------------------------------------------------------------------*/
/**
 * @brief 根据优先级标志位获取最高优先级(硬件方法)
 */
#define mos_get_highest_priority(top_priority, priority_flag_32bit) \
    top_priority = (mos_uint32_t) __clz(priority_flag_32bit)

/**
 * @brief 根据优先级标志位更改当前正在运行任务的任务控制块
 */
#define mos_task_select_highest_priority()  \
    { 	\
        mos_ubase_t	top_priority; \
        mos_get_highest_priority(top_priority, g_mos_task_priority_flag_32bit); \
        g_cur_task_tcb = MOS_LIST_ENTRY(g_mos_task_list_ready_table[top_priority].next, mos_tcb_t, task_list); \
        mos_list_node_delete(&(g_cur_task_tcb->task_list)); \
        mos_list_tail_insert(&g_mos_task_list_ready_table[top_priority], &(g_cur_task_tcb->task_list));  \
    } 	\


/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief  任务创建
 * @param  任务控制块
 * @param  任务入口函数
 * @param  任务名字
 * @param  任务优先级
 * @param  任务形参
 * @param  任务栈大小
 * @param  任务栈起始地址
 *
 * @return 任务创建错误标志
 */
mos_err_t mos_task_create(	mos_tcb_t *  const task_tcb,	/* 任务控制块指针 */
                            task_entry_fun 	   task_code,	/* 任务入口 */
                            const char * const task_name,	/* 任务名称，字符串形式 */
                            const mos_uint8_t  task_pri,	/* 任务优先级 */
                            const mos_uint32_t parameter,	/* 任务形参 */
                            const mos_uint32_t stack_size,	/* 任务栈大小，单位为字 */
                            const mos_uint32_t stack_start)	/* 任务栈起始地址 */
{
    mos_tcb_t *mos_new_tcb;
    mos_err_t ret = 0;

    if((task_tcb != NULL) && ((mos_uint32_t *)stack_start != NULL))
    {
        mos_new_tcb = task_tcb;
        /* 创建新的任务 */
        mos_task_init_new(	mos_new_tcb,	/* 任务控制块指针 */
                            task_code,		/* 任务入口 */
                            task_name,		/* 任务名称，字符串形式 */
                            task_pri,		/* 任务优先级 */
                            parameter,		/* 任务形参 */
                            stack_size,		/* 任务栈大小，单位为字 */
                            stack_start);	/* 任务栈起始地址 */

        /* 将任务插入就绪列表 */
        mos_task_insert_ready_table_list(g_mos_task_list_ready_table, mos_new_tcb);
    }
    else
    {
        ret = 0;
    }

    /* 返回任务状态*/
    return ret;
}

/**
 * @brief 任务调度器初始化
 *
 * @return 任务调度器初始化错误标志
 */
mos_err_t mos_task_scheduler_init(void)
{
    mos_ubase_t priortiy;

    /* 任务就绪表初始化 */
    for(priortiy = 0U; priortiy < MOS_CONFIG_TASK_PRIORITY_MAX; priortiy++ )
    {
        mos_list_init(&g_mos_task_list_ready_table[priortiy]);
    }

    /*任务延时列表初始化*/
    g_mos_task_delay_list = &g_mos_task_delay_list_1;
    mos_list_init(g_mos_task_delay_list);

    /*任务延时溢出列表初始化*/
    g_mos_task_delay_overflow_list = &g_mos_task_delay_list_2;
    mos_list_init(g_mos_task_delay_overflow_list);

    /* 下一个任务解除阻塞时间初始化为最大延时时间 */
    g_mos_task_next_task_unblock_tick = MOS_MAX_DELAY;

    return 0;
}

/**
 * @brief 第一次开启任务调度
 *
 * @return 开启任务调度错误标志
 */
mos_err_t mos_task_scheduler_start(void)
{
    /* 初始化空闲任务 */
    mos_task_idle_init();

    /* 获取第一个要运行的最高优先级任务 */
    mos_task_select_highest_priority();

    /* 第一次启动任务 */
    mos_port_start_first_task();

    /* 不会到这 */
    return 0;
}

/**
 * @brief 任务调度
 */
mos_err_t mos_task_scheduler(void)
{
    mos_port_task_scheduler();

    return 0;
}

/**
 * @brief 任务选择,即更新g_cur_task_tcb
 */
void mos_task_switch_context(void)
{
    /* 获取优先级最高的就绪任务的TCB，然后更新到g_cur_task_tcb */
    mos_task_select_highest_priority();
}

/**
 * @brief 任务延时
 */
void mos_task_delay(const mos_uint32_t tick)
{
    register mos_base_t temp;
    /* 失能中断 */
    temp = mos_port_entry_critical();

    /* 将任务插入到延时列表 */
    mos_task_delay_process(tick);

    /* 使能中断 */
    mos_port_exit_critical(temp);

    /* 任务切换 */
    mos_task_scheduler();
}

/**
 * @brief 将任务插入就绪列表
 */
void mos_task_insert_ready_table_list(mos_list_t *task_list_ready_table, mos_tcb_t *mos_tcb)
{
    /* 将任务插入就绪列表 */
    mos_list_head_insert(&(task_list_ready_table[mos_tcb->task_priority]), &(mos_tcb->task_list));
    /* 设置对应位优先级标志 */
    g_mos_task_priority_flag_32bit |= (1UL << (31UL - mos_tcb->task_priority));
}

/**
 * @brief 将任务从就绪列表删除
 */
void mos_task_remove_ready_table_list(mos_list_t *task_list_ready_table, mos_tcb_t *mos_tcb)
{
    /* 将任务从就绪列表中移除 */
    mos_list_node_delete(&(mos_tcb->task_list));

    /* 将任务在优先级位图中对应的位清除 */
    if (mos_list_is_empty(&(task_list_ready_table[mos_tcb->task_priority])))
    {
        g_mos_task_priority_flag_32bit &= ~(1UL << (31UL - mos_tcb->task_priority));

    }
}

/**
 * @brief 系统当前时基计数器计数增加
 */
void mos_task_tickcount_increase(void)
{
    mos_tcb_t *mos_tcb;
    g_mos_task_cur_tick_count++;

    /* 如果xConstTickCount溢出，则切换延时列表 */
    if( g_mos_task_cur_tick_count == (mos_tick_t)0U)
    {
        mos_task_switch_delay_list();
    }

    if(g_mos_task_cur_tick_count >= g_mos_task_next_task_unblock_tick)
    {
        for( ;; )
        {
            if(mos_list_is_empty(g_mos_task_delay_list))
            {
                /* 延时列表为空，设置g_mos_task_next_task_unblock_tick为可能的最大值 */
                g_mos_task_next_task_unblock_tick = MOS_MAX_DELAY;
                break;
            }
            else /* 延时列表不为空 */
            {
                mos_tcb = MOS_LIST_ENTRY(g_mos_task_delay_list->next, mos_tcb_t, task_list);

                /* 直到将延时列表中所有延时到期的任务移除才跳出for循环 */
                if(g_mos_task_cur_tick_count < mos_tcb-> task_tick_wake)
                {
                    g_mos_task_next_task_unblock_tick = g_mos_task_cur_tick_count;
                    break;
                }

                /* 将任务从延时列表移除，消除等待状态 */
                mos_list_node_delete(&mos_tcb->task_list);
                /* 重置任务控制块的task_tick_wake */
                mos_tcb->task_tick_wake = 0L;

                /* 将解除等待的任务添加到就绪列表 */
                mos_task_insert_ready_table_list(g_mos_task_list_ready_table, mos_tcb);

            }
        }
    }

    /* 系统调度 */
    mos_task_scheduler();
}


/* Private Fun----------------------------------------------------------------*/
/**
 * @brief 初始化新创建的任务
 */
static void mos_task_init_new(mos_tcb_t *  const mos_new_tcb,	/* 任务控制块指针 */
                              task_entry_fun 	   task_code,		/* 任务入口 */
                              const char * const task_name,		/* 任务名称，字符串形式 */
                              const mos_uint8_t  task_pri,		/* 任务优先级 */
                              const mos_uint32_t parameter,		/* 任务形参 */
                              const mos_uint32_t stack_size,		/* 任务栈大小，单位为字 */
                              const mos_uint32_t stack_start)		/* 任务栈起始地址 */
{
    mos_strncpy(mos_new_tcb->task_name, task_name, MOS_CONFIG_TASK_NAMELEN);

    mos_new_tcb->task_entry = task_code;
    mos_new_tcb->task_param = parameter;

    mos_new_tcb->stack_top  = stack_start;
    mos_new_tcb->stack_size = stack_size;

    mos_new_tcb->stack_pointer = mos_hw_stack_init(	task_code,
                                 parameter,
                                 stack_start + stack_size - 4);

    if (task_pri >= MOS_CONFIG_TASK_PRIORITY_MAX)
    {
        mos_new_tcb->task_priority = MOS_CONFIG_TASK_PRIORITY_MAX - 1;
    }
    else
    {
        mos_new_tcb->task_priority = task_pri;
    }

    mos_new_tcb->task_tick_wake = 0UL;
    mos_new_tcb->task_state = 0;	//临时
    mos_list_init(&mos_new_tcb->task_list);
}

/**
 * @brief 空闲任务入口函数
 */
static void mos_task_idle_entry(void *p_arg)
{
    while (1)
    {

    }
}

/**
 * @brief 初始化空闲任务
 */
static void mos_task_idle_init(void)
{
    /* 初始化idle */
    mos_task_create(&idle_tcb,
                    (task_entry_fun)mos_task_idle_entry,
                    "idle_task",
                    MOS_CONFIG_TASK_PRIORITY_MAX - 1,
                    (mos_uint32_t)NULL,
                    MOS_CONFIG_TASK_IDLE_STACK_SIZE,
                    (mos_uint32_t)mos_task_idle_stack);
}

/**
 * @brief  将任务插入延时列表
 * @param  任务延时列表
 * @param  任务自身链表
 * @param  系统唤醒该任务时间
 */
static void mos_task_insert_delay_list(mos_list_t *delay_list, mos_list_t *task_node, mos_tick_t mos_task_wake_tick)
{
    mos_list_t *iterator;
    mos_tcb_t  *mos_delay_tcb;
    iterator = delay_list;
	
	/* 按照唤醒时间task_tick_wake从小到大找到task_node插入点 */
    while (iterator->next != delay_list)
    {
        mos_delay_tcb = MOS_LIST_ENTRY(iterator->next, mos_tcb_t, task_list);

        if (mos_delay_tcb->task_tick_wake > mos_task_wake_tick)
        {
            break;
        }

        iterator = iterator->next;
    }

	/* 将任务插入延时列表 */
    mos_list_head_insert(iterator, task_node);
}

/**
 * @brief 任务延时处理
 */
static void mos_task_delay_process(const mos_uint32_t tick)
{
    mos_tick_t mos_task_wake_tick;

    /* 获取系统时基计数器g_mos_tick_cur_tick_count的值 */
    const mos_tick_t mos_cur_tick_count = g_mos_task_cur_tick_count;

    /* 将任务从就绪列表中移除 */
    mos_task_remove_ready_table_list(g_mos_task_list_ready_table, g_cur_task_tcb);

    /* 计算延时到期时，系统时基计数器mos_cur_tick_count的值是多少 */
    mos_task_wake_tick = mos_cur_tick_count + tick;

    /* 修改当前列表唤醒时间 */
    g_cur_task_tcb->task_tick_wake = mos_task_wake_tick;

    /* 溢出 */
    if(mos_task_wake_tick < mos_cur_tick_count )
    {
        mos_task_insert_delay_list(g_mos_task_delay_overflow_list, &(g_cur_task_tcb->task_list), mos_task_wake_tick);
    }
    /* 没有溢出 */
    else
    {
        mos_task_insert_delay_list(g_mos_task_delay_list, &(g_cur_task_tcb->task_list), mos_task_wake_tick);

        /* 更新下一个任务解锁时刻变量g_mos_task_next_task_unblock_tick的值 */
        if(mos_task_wake_tick < g_mos_task_next_task_unblock_tick)
        {
            g_mos_task_next_task_unblock_tick = mos_task_wake_tick;
        }
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
 * @brief 切换延时列表后复位下一个任务阻塞时间
 */
static void mos_task_reset_next_task_unblock_time(void)
{
    mos_tcb_t *mos_tcb;

	/* 新延时列表为空时设置阻塞时间为最大值 */
    if(mos_list_is_empty(g_mos_task_delay_list))
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
