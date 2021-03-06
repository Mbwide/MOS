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
  * @file    mos_task.h
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

#ifndef _MOS_TASK_H
#define _MOS_TASK_H

#include "mos_typedef.h"
#include "mos_list.h"
#include "mos_misc.h"
#include "mos_user_config.h"

/* 任务入口函数定义 */
typedef void (*task_entry_fun)(void);

/* 任务状态 */
typedef enum TASK_STATE
{
    TASK_RUN = 1, /* 运行态 */
    TASK_READY,   /* 就绪态 */
    TASK_BLOCK,   /* 阻塞态 */
    TASK_SUSPEND  /* 挂起态 */
} TASK_STATE_T;

/* 任务控制块定义 */
typedef struct mos_task_control_block
{
    volatile void   *stack_pointer;                       /* 任务栈指针sp */
    char            task_name[MOS_CONFIG_TASK_NAMELEN];   /* 任务名字 */
    task_entry_fun  task_entry;                           /* 任务入口函数 */
    mos_uint32_t    task_param;                           /* 任务形参 */

    mos_uint32_t    stack_top;                            /* 任务栈顶地址,低地址 */
    mos_uint32_t    stack_size;                           /* 任务栈大小 */

    mos_uint16_t    task_priority;                        /* 任务优先级 */
    mos_uint16_t    task_base_priority;                   /* 任务基优先级 */
    mos_uint16_t    task_state;                           /* 任务状态*/

    mos_uint32_t    task_tick_wake;                       /* 任务唤醒时间*/
    mos_uint32_t    task_tick_wake_over;                  /* 任务唤醒时间溢出标志位*/

    mos_list_t      task_list;                            /* 任务所处调度链表*/
#if (MOS_CONFIG_USE_SHELL == YES)
    mos_list_t      task_shell_list;                      /* 任务所处shell列表*/
#endif
    mos_list_t      task_ipc_list;                        /* 任务所处任务间通信链表*/
} mos_tcb_t;

/* Public Fun-----------------------------------------------------------------*/
#if MOS_CONFIG_USE_DYNAMIC_HEAP
/* 动态任务创建 */
mos_err_t mos_task_create( mos_tcb_t *  const task_tcb,    /* 任务控制块指针 */
                           task_entry_fun 	  task_code,   /* 任务入口 */
                           const char * const task_name,   /* 任务名称，字符串形式 */
                           const mos_uint8_t  task_pri,    /* 任务优先级 */
                           const mos_uint32_t stack_size); /* 任务栈大小，单位为字 */
#else
/* 静态任务创建 */
mos_err_t mos_task_create(mos_tcb_t *  const task_tcb,	    /* 任务控制块指针 */
                          task_entry_fun 	   task_code,	/* 任务入口 */
                          //const char * const task_name,	/* 任务名称，字符串形式 */
                          const mos_uint8_t  task_pri,	    /* 任务优先级 */
                          //const mos_uint32_t parameter,	/* 任务形参 */
                          const mos_uint32_t stack_size,	/* 任务栈大小，单位为字 */
                          const mos_uint32_t stack_start);  /* 任务栈起始地址 */
#endif

/* 任务调度器初始化 */
mos_err_t mos_task_scheduler_init(void);
/* 第一次开启任务调度 */
mos_err_t mos_task_scheduler_start(void);
/* 任务调度 */
mos_err_t mos_task_scheduler(void);
/* 任务选择,即更新当前运行任务 */
void mos_task_switch_context(void);

/* 将任务挂起 */
void mos_task_suspend(mos_tcb_t * to_suspend_task);
/* 将任务恢复 */
void mos_task_resume(mos_tcb_t * to_suspend_task);
/* 任务延时 */
void mos_task_delay(const mos_uint32_t tick);
/* 获取当前任务控制块 */
mos_tcb_t * mos_task_get_cur_tcb(void);
/* 获取任务调度标志 0 为开启 */
mos_ubase_t mos_task_get_scheduler_flag(void);

/* 将任务结点插入到挂起列表 */
void mos_task_insert_suspend_list(mos_tcb_t * to_suspend_task);
/* 将任务结点插入到就绪列表 */
void mos_task_insert_ready_table_list(mos_tcb_t *to_ready_task);
/* 将任务结点从就绪列表删除 */
void mos_task_remove_ready_table_list(mos_tcb_t *mos_tcb);
/* 将任务结点插入延时列表 */
void mos_task_insert_delay_list(mos_list_t *delay_list, mos_list_t *task_node, mos_tick_t mos_task_wake_tick);
/* 将任务结点从延时列表删除 */
void mos_task_remove_delay_list(mos_tcb_t * to_remove_delay_tcb);

#endif /* _MOS_TASK_H */

