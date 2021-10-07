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
  * @version V1.0.1
  * @date    2021-10-07
  * @brief   任务与调度
  ******************************************************************************
  * @note
  *          2021-08-20 Mbwide:初始版本
  *          2021-10-04 Mbwide:结构优化
  *          2021-10-07 Mbwide:添加shell功能
  ******************************************************************************
  */

#include "mos_task.h"
#include "mos_hw.h"
#include "mos_port.h"
#include "mos_heap.h"
#include "mos_tick.h"

/* Private -------------------------------------------------------------------*/
/* 延时列表，交替使用，用于解决时钟溢出问题 */
static mos_list_t g_mos_task_delay_list_1;
static mos_list_t g_mos_task_delay_list_2;
/* 任务延时溢出列表指针 */
static mos_list_t * volatile g_mos_task_delay_overflow_list;
/* 任务悬起列表 */
//static mos_list_t g_mos_task_pend_list;
/* 任务优先级标志*/
static volatile mos_uint32_t g_mos_task_priority_flag_32bit;
/* 任务调度开启标志，0为开启 */
static mos_ubase_t g_scheduler_flag;

#if MOS_CONFIG_USE_DYNAMIC_HEAP
/* 空闲任务 任务控制块 */
static mos_tcb_t idle_tcb;
#else
/* 空闲任务 任务控制块 */
static mos_tcb_t idle_tcb;
/* 空闲任务栈 */
static mos_uint8_t mos_task_idle_stack[MOS_CONFIG_TASK_IDLE_STACK_SIZE];
#endif


/* Public --------------------------------------------------------------------*/
/* 当前正在运行任务的任务控制块 */
mos_tcb_t * volatile g_cur_task_tcb = NULL;
/* 任务就绪表 */
mos_list_t  g_mos_task_list_ready_table[MOS_CONFIG_TASK_PRIORITY_MAX];
/* 任务挂起列表 */
mos_list_t g_mos_task_suspend_list;
/* 任务延时列表指针 */
mos_list_t * volatile g_mos_task_delay_list;
/* 下一个任务解除阻塞时间 */
extern volatile mos_uint32_t g_mos_task_next_task_unblock_tick;

#if (MOS_CONFIG_USE_SHELL == YES)
/* shell任务列表 */
extern mos_list_t g_mos_shell_task_list;
#endif


/* define --------------------------------------------------------------------*/
/**
 * @brief 根据优先级标志位获取最高优先级(硬件方法)
 */
#define mos_get_highest_priority(top_priority, priority_flag_32bit) \
    top_priority = (mos_uint32_t) __clz(priority_flag_32bit)

/**
 * @brief 根据优先级标志位更改当前正在运行任务的任务控制块
 */
#if (MOS_CONFIG_USE_TIME_SLICING == YES)
#define mos_task_select_highest_priority()  \
    { 	\
        mos_ubase_t	top_priority; \
        mos_get_highest_priority(top_priority, g_mos_task_priority_flag_32bit); \
		if (g_cur_task_tcb->task_state == TASK_RUN) g_cur_task_tcb->task_state = TASK_READY; \
        g_cur_task_tcb = MOS_LIST_ENTRY(g_mos_task_list_ready_table[top_priority].next, mos_tcb_t, task_list); \
        mos_list_node_delete(&(g_cur_task_tcb->task_list)); \
        mos_list_tail_insert(&g_mos_task_list_ready_table[top_priority], &(g_cur_task_tcb->task_list)); \
        g_cur_task_tcb->task_state = TASK_RUN; \
	} 	\

#else
#define mos_task_select_highest_priority()  \
    { 	\
        mos_ubase_t	top_priority; \
        mos_get_highest_priority(top_priority, g_mos_task_priority_flag_32bit); \
        g_cur_task_tcb = MOS_LIST_ENTRY(g_mos_task_list_ready_table[top_priority].next, mos_tcb_t, task_list); \
        g_cur_task_tcb->task_state = TASK_RUN; \
    } 	\

#endif

/* Private Fun----------------------------------------------------------------*/
/**
 * @brief 初始化新创建的任务
 */
static void mos_task_init_new(mos_tcb_t *  const mos_new_tcb,  /* 任务控制块指针 */
                              task_entry_fun 	   task_code,  /* 任务入口 */
                              const char * const task_name,    /* 任务名称，字符串形式 */
                              const mos_uint8_t  task_pri,     /* 任务优先级 */
                              //const mos_uint32_t parameter,  /* 任务形参 */
                              const mos_uint32_t stack_size,   /* 任务栈大小，单位为字 */
                              const mos_uint32_t stack_start)  /* 任务栈起始地址 */
{
    mos_strncpy(mos_new_tcb->task_name, task_name, MOS_CONFIG_TASK_NAMELEN);

    mos_new_tcb->task_entry = task_code;
    //mos_new_tcb->task_param = parameter;

    mos_new_tcb->stack_top  = stack_start;
    mos_new_tcb->stack_size = stack_size;

    mos_new_tcb->stack_pointer = mos_hw_stack_init(	task_code,
                                 0,
                                 stack_start + stack_size - 4);

    if (task_pri >= MOS_CONFIG_TASK_PRIORITY_MAX)
    {
        mos_new_tcb->task_priority = MOS_CONFIG_TASK_PRIORITY_MAX - 1;
        mos_new_tcb->task_base_priority = MOS_CONFIG_TASK_PRIORITY_MAX - 1;
    }
    else
    {
        mos_new_tcb->task_priority = task_pri;
        mos_new_tcb->task_base_priority = task_pri;
    }

    mos_new_tcb->task_tick_wake = 0UL;
    mos_new_tcb->task_state = TASK_READY;
	
    /* 任务状态列表 */
    mos_list_init(&mos_new_tcb->task_list);
	
#if (MOS_CONFIG_USE_SHELL == YES)
	/* 任务shell列表 */
    mos_list_init(&mos_new_tcb->task_shell_list);
#endif
	
    /* IPC阻塞列表 */
    mos_list_init(&mos_new_tcb->task_ipc_list);
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
#if MOS_CONFIG_USE_DYNAMIC_HEAP
    /* 动创建idle任务 */
    mos_task_create(&idle_tcb,
                    (task_entry_fun)mos_task_idle_entry,
	                "IDLE_TASK",
                    MOS_CONFIG_TASK_PRIORITY_MAX - 1,
                    MOS_CONFIG_TASK_IDLE_STACK_SIZE);
#else
    /* 静态创建idle任务 */
    mos_task_create(&idle_tcb,
                    (task_entry_fun)mos_task_idle_entry,
                    //"idle_task",
                    MOS_CONFIG_TASK_PRIORITY_MAX - 1,
                    //(mos_uint32_t)NULL,
                    MOS_CONFIG_TASK_IDLE_STACK_SIZE,
                    (mos_uint32_t)mos_task_idle_stack);
#endif
}

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
#if (MOS_CONFIG_USE_DYNAMIC_HEAP == YES)
mos_err_t mos_task_create( mos_tcb_t *  const task_tcb,   /* 任务控制块指针 */
                           task_entry_fun 	  task_code,  /* 任务入口 */
                           const char * const task_name,  /* 任务名称，字符串形式 */
                           const mos_uint8_t  task_pri,   /* 任务优先级 */
                           const mos_uint32_t stack_size) /* 任务栈大小，单位为字 */
{
    mos_tcb_t *mos_new_tcb;
    mos_err_t ret = 0;

    mos_ubase_t *stack_start = (mos_ubase_t *)mos_malloc(stack_size);

    if ((task_tcb != NULL) && (stack_start != NULL))
    {
        mos_new_tcb = task_tcb;
        /* 创建新的任务 */
        mos_task_init_new( mos_new_tcb, /* 任务控制块指针 */
                           task_code,   /* 任务入口 */
		                   task_name,   /* 任务名称，字符串形式 */
                           task_pri,    /* 任务优先级 */
                           stack_size,  /* 任务栈大小，单位为字 */
                           (mos_uint32_t)stack_start);	/* 任务栈起始地址 */

        /* 将任务插入就绪列表 */
        mos_task_insert_ready_table_list(mos_new_tcb);

#if (MOS_CONFIG_USE_SHELL == YES)
	    /* 将任务插入shell列表 */
        mos_list_tail_insert(&g_mos_shell_task_list, &mos_new_tcb->task_shell_list);
#endif			

		
#if (MOS_CONFIG_USE_DEBUG_PRINTF == YES)
	mos_printf("%s Creat Success!\r\n", task_name);
#endif
		
    }
    else
    {
#if (MOS_CONFIG_USE_DEBUG_PRINTF == YES)
		mos_printf("%s Creat failed!\r\n", task_name);
#endif
        ret = 0;
    }

    /* 返回任务状态*/
    return ret;
}

#else
mos_err_t mos_task_create( mos_tcb_t *  const task_tcb,    /* 任务控制块指针 */
                           task_entry_fun 	   task_code,  /* 任务入口 */
                           //const char * const task_name, /* 任务名称，字符串形式 */
                           const mos_uint8_t  task_pri,    /* 任务优先级 */
                           //const mos_uint32_t parameter, /* 任务形参 */
                           const mos_uint32_t stack_size,  /* 任务栈大小，单位为字 */
                           const mos_uint32_t stack_start) /* 任务栈起始地址 */
{
    mos_tcb_t *mos_new_tcb;
    mos_err_t ret = 0;

    if ((task_tcb != NULL) && ((mos_uint32_t *)stack_start != NULL))
    {
        mos_new_tcb = task_tcb;
        /* 创建新的任务 */
        mos_task_init_new( mos_new_tcb,  /* 任务控制块指针 */
                           task_code,    /* 任务入口 */
                           //task_name,  /* 任务名称，字符串形式 */
                           task_pri,     /* 任务优先级 */
                           //parameter,  /* 任务形参 */
                           stack_size,   /* 任务栈大小，单位为字 */
                           stack_start); /* 任务栈起始地址 */

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
#endif

/**
 * @brief 任务调度器初始化
 *
 * @return 任务调度器初始化错误标志
 */
mos_err_t mos_task_scheduler_init(void)
{
    mos_ubase_t priortiy;

    /* 任务调度器开启标志位 */
    g_scheduler_flag = 0;

    /* 任务就绪表初始化 */
    for (priortiy = 0U; priortiy < MOS_CONFIG_TASK_PRIORITY_MAX; priortiy++ )
    {
        mos_list_init(&g_mos_task_list_ready_table[priortiy]);
    }

    /* 任务挂起列表初始化 */
    mos_list_init(&g_mos_task_suspend_list);

    /*任务延时列表初始化*/
    g_mos_task_delay_list = &g_mos_task_delay_list_1;
    mos_list_init(g_mos_task_delay_list);

    /*任务延时溢出列表初始化*/
    g_mos_task_delay_overflow_list = &g_mos_task_delay_list_2;
    mos_list_init(g_mos_task_delay_overflow_list);

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

#if (MOS_CONFIG_USE_DEBUG_PRINTF == YES)
	mos_printf("Scheduler Runing!\r\n");
#endif

#if (MOS_CONFIG_USE_SHELL == YES)
	mos_printf("CMD > ");
#endif	
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
 * @brief 挂起任务
 */
void mos_task_suspend(mos_tcb_t * to_suspend_task)
{
    mos_base_t temp;

    /* 进入临界区 */
    temp = mos_port_entry_critical_temp();

    /* 如果要挂起的任务是运行态 */
    if (to_suspend_task->task_state == TASK_RUN)
    {
        mos_task_insert_suspend_list(to_suspend_task);
        mos_port_task_scheduler();
    }
    /* 如果要挂起的任务是其他状态 */
    else
    {
        mos_task_insert_suspend_list(to_suspend_task);
    }

    /* 退出临界区 */
    mos_port_exit_critical_temp(temp);
}

/**
 * @brief 恢复任务
 */
void mos_task_resume(mos_tcb_t * to_resume_task)
{
    mos_base_t temp;

    /* 进入临界区 */
    temp = mos_port_entry_critical_temp();

    mos_list_node_delete(&(to_resume_task->task_list));
    mos_task_insert_ready_table_list( to_resume_task);

    /* 退出临界区 */
    mos_port_exit_critical_temp(temp);

    /* 任务调度 */
    mos_task_scheduler();

}

/**
 * @brief 任务延时
 */
void mos_task_delay(const mos_uint32_t tick)
{
    /* 失能中断 */
    mos_port_entry_critical();

    /* 将任务插入到延时列表 */
    mos_tick_delay_process(g_cur_task_tcb, tick);

    /* 使能中断 */
    mos_port_exit_critical();

    /* 任务切换 */
    mos_task_scheduler();
}

/**
 * @brief  获取当前任务控制块
 *
 * @return 当前任务控制块
 */
mos_tcb_t * mos_task_get_cur_tcb(void)
{
    return g_cur_task_tcb;
}

/**
 * @brief 将任务结点插入到挂起列表
 */
void mos_task_insert_suspend_list(mos_tcb_t * to_suspend_task)
{
    /* 如果要挂起的任务是运行态 */
    if (to_suspend_task->task_state == TASK_RUN)
    {
        mos_task_remove_ready_table_list(to_suspend_task);
        to_suspend_task->task_state = TASK_SUSPEND;
        mos_list_head_insert(&g_mos_task_suspend_list, &to_suspend_task->task_list);
    }
    /* 如果要挂起的任务是就绪态 */
    else if (to_suspend_task->task_state == TASK_READY)
    {
        mos_task_remove_ready_table_list(to_suspend_task);
        to_suspend_task->task_state = TASK_SUSPEND;
        mos_list_head_insert(&g_mos_task_suspend_list, &to_suspend_task->task_list);
    }
    /* 如果要挂起的任务是阻塞态 */
    else if (to_suspend_task->task_state == TASK_BLOCK)
    {
        /* 将任务从阻塞（延时）列表中移除 */
        mos_task_remove_delay_list(to_suspend_task);
        to_suspend_task->task_state = TASK_SUSPEND;
        mos_list_head_insert(&g_mos_task_suspend_list, &to_suspend_task->task_list);
    }

#if (MOS_CONFIG_USE_IPC == YES)

    /* 删除任务信号量阻塞结点 */
    if (!mos_list_is_empty(&to_suspend_task->task_ipc_list))
    {
        mos_list_node_delete(&to_suspend_task->task_ipc_list);
    }

#endif
}

/**
 * @brief 将任务结点插入到就绪列表
 */
void mos_task_insert_ready_table_list(mos_tcb_t *to_ready_task)
{
    /* 挂起态 */
    if(to_ready_task->task_state == TASK_SUSPEND)
    {
        mos_list_node_delete(&(to_ready_task->task_list));
    }
    /* 阻塞态 */
    else if (to_ready_task->task_state == TASK_BLOCK)
    {
        mos_task_remove_delay_list(to_ready_task);
    }

#if (MOS_CONFIG_USE_IPC == YES)

    if (!mos_list_is_empty(&to_ready_task->task_ipc_list))
    {
        mos_list_node_delete(&to_ready_task->task_ipc_list);
    }

#endif

    /* 修改任务状态为就绪态 */
    to_ready_task->task_state = TASK_READY;
    /* 将任务插入就绪列表 */
    mos_list_head_insert(&(g_mos_task_list_ready_table[to_ready_task->task_priority]), &(to_ready_task->task_list));
    /* 设置对应位优先级标志 */
    g_mos_task_priority_flag_32bit |= (1UL << (31UL - to_ready_task->task_priority));
}


/**
 * @brief 将任务结点从就绪列表删除
 */
void mos_task_remove_ready_table_list(mos_tcb_t *mos_tcb)
{
    /* 将任务从就绪列表中移除 */
    mos_list_node_delete(&(mos_tcb->task_list));

    /* 将任务在优先级位图中对应的位清除 */
#if (MOS_CONFIG_USE_TIME_SLICING == YES)

    if (mos_list_is_empty(&(g_mos_task_list_ready_table[mos_tcb->task_priority])))
    {
        g_mos_task_priority_flag_32bit &= ~(1UL << (31UL - mos_tcb->task_priority));
    }

#else
    g_mos_task_priority_flag_32bit &= ~(1UL << (31UL - mos_tcb->task_priority));
#endif
}

/**
 * @brief  将任务结点插入延时列表
 * @param  任务延时列表
 * @param  任务自身链表
 * @param  系统唤醒该任务时间
 */
void mos_task_insert_delay_list(mos_list_t *delay_list, mos_list_t *task_node, mos_tick_t mos_task_wake_tick)
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

    /* 插入列表 */
    mos_list_head_insert(iterator, task_node);
}

/**
 * @brief  将任务结点从延时列表删除
 * @param  任务延时列表
 * @param  任务自身链表
 */
void mos_task_remove_delay_list(mos_tcb_t * to_remove_delay_tcb)
{
    mos_tcb_t * delay_next_task_tcb = NULL;

    /* 将任务从延时列表移除，消除等待状态 */
    mos_list_node_delete(&to_remove_delay_tcb->task_list);
    /* 重置任务控制块的task_tick_wake */
    to_remove_delay_tcb->task_tick_wake = 0L;

    /* 延时列表为空，设置g_mos_task_next_task_unblock_tick为可能的最大值 */
    if(mos_list_is_empty(g_mos_task_delay_list))
    {
        g_mos_task_next_task_unblock_tick = MOS_MAX_DELAY;
    }
    /* 延时列表不为空 */
    else
    {
        delay_next_task_tcb = MOS_LIST_ENTRY(g_mos_task_delay_list->next, mos_tcb_t, task_list);
        g_mos_task_next_task_unblock_tick = delay_next_task_tcb->task_tick_wake;
    }
}

/**
 * @brief 关闭任务调度
 */
void mos_task_scheduler_close(void)
{
    mos_base_t temp;

    /* 进入临界区 */
    temp = mos_port_entry_critical_temp();

    /* g_scheduler_flag > 0 表示关闭任务调度器 */
    g_scheduler_flag++;

    /* 退出临界区 */
    mos_port_exit_critical_temp(temp);
}

/**
 * @brief 开启任务调度
 */
void mos_task_scheduler_resume(void)
{
    mos_tcb_t *mos_tcb;
    mos_ubase_t	top_priority;
    mos_base_t temp;

    /* 进入临界区 */
    temp = mos_port_entry_critical_temp();

    /* g_scheduler_flag > 0 表示关闭任务调度器 */
    --g_scheduler_flag;

    /* 如果关闭任务调度器期间恢复了高优先级任务，进行一次任务调度 */
    mos_get_highest_priority(top_priority, g_mos_task_priority_flag_32bit);
    mos_tcb = MOS_LIST_ENTRY(g_mos_task_list_ready_table[top_priority].next, mos_tcb_t, task_list);

    if ((mos_tcb->task_priority < g_cur_task_tcb->task_priority))
    {
        mos_port_task_scheduler();
    }

    /* 退出临界区 */
    mos_port_exit_critical_temp(temp);
}

