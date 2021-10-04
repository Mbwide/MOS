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
  * @file    mos_ipc.c
  * @version V1.0.0
  * @date    2021-09-15
  * @brief   任务间通信
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#include "mos_ipc.h"
#include "mos_port.h"
#include "mos_task.h"
#include "mos_tick.h"

/* Private Fun----------------------------------------------------------------*/
/**
 * @brief  用于IPC的任务挂起(阻塞)
 * @param  将要挂起的任务
 * @param  信号量头属性
 *
 * @return 任务挂起成功标志位
 */
static mos_err_t mos_ipc_task_suspend(mos_tcb_t *to_suspend_task, mos_ipc_t *mos_ipc)
{
    /* 非法地址 */
    if (to_suspend_task == NULL || mos_ipc == NULL)
    {
        return MOS_ENULL;
    }

    /* 将任务IPC阻塞结点挂到该信号量下 */
    mos_list_head_insert(&mos_ipc->block_list, &to_suspend_task->task_ipc_list);
   
	/* 将任务结点插入到挂起列表 */
    mos_task_insert_suspend_list(to_suspend_task);

    return MOS_EOK;
}

/**
 * @brief  用于IPC的任务恢复
 * @param  将要恢复的任务
 * @param  信号量头属性
 *
 * @return 任务恢复成功标志位
 */
static mos_err_t mos_ipc_task_resume(mos_ipc_t *mos_ipc)
{

    mos_tcb_t * to_resume_task = NULL;

    /* 获取将要恢复的任务控制块 */
    to_resume_task = MOS_LIST_ENTRY(mos_ipc->block_list.next, mos_tcb_t, task_ipc_list);

    /* 非法地址 */
    if (to_resume_task == NULL || mos_ipc == NULL)
    {
        return MOS_ENULL;
    }

    /* 删除任务阻塞结点 */
    mos_list_node_delete(&to_resume_task->task_ipc_list);
	
	/* 将任务结点插入就绪列表 */
    mos_task_insert_ready_table_list(to_resume_task);
	
    return MOS_EOK;
}

/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief  创建同步信号量
 * @param  同步信号量控制块
 *
 * @return 信号量创建成功标志位
 */
mos_err_t mos_ipc_sync_creat(mos_sync_t *sync_ipc)
{

    /* 非法地址 */
    if(sync_ipc == NULL) return MOS_ENULL;

    /* 初始化同步信号量 */
    sync_ipc->sync_ipc.flag = SYNC_IPC;
    mos_list_init(&(sync_ipc->sync_ipc.block_list));
    sync_ipc->value = 0;

    return MOS_EOK;
}

/**
 * @brief  获取同步信号量
 * @param  同步信号量控制块
 * @param  阻塞时间
 *
 * @return 信号量获取成功标志位
 */
mos_err_t mos_ipc_sync_take(mos_sync_t *mos_sync_ipc, mos_uint32_t to_block_tick)
{
    register mos_base_t temp;
    mos_tcb_t * cur_tcb = NULL;

    if (mos_sync_ipc == NULL) return MOS_ENULL;

    /* 关闭中断 */
    temp = mos_port_interrupt_disable_temp();

    /* 有资源 */
    if (mos_sync_ipc->value > 0)
    {
        --mos_sync_ipc->value;
    }
    /* 没有资源 */
    else
    {
        /* 没有阻塞时间*/
        if (to_block_tick == 0)
        {
            /* 开启中断 */
            mos_port_interrupt_enable_temp(temp);
            return MOS_EEMPTY;
        }
        /* 有阻塞时间*/
        else
        {
            /* 获取当前任务控制块*/
            cur_tcb = mos_task_get_cur_tcb();

            /* 挂起 */
            if (to_block_tick == MOS_MAX_DELAY)
            {
                mos_ipc_task_suspend(cur_tcb, &mos_sync_ipc->sync_ipc);
            }
            /* 阻塞 */
            else
            {
                mos_tick_delay_process(cur_tcb, to_block_tick);
            }

            /* 将任务IPC阻塞列表挂到该信号量下,尾插 */
            mos_list_tail_insert(&mos_sync_ipc->sync_ipc.block_list, &cur_tcb->task_ipc_list);

            /* 开启中断 */
            mos_port_interrupt_enable_temp(temp);
            /* 任务调度 */
            mos_task_scheduler();
        }
    }

    return MOS_EOK;
}

/**
 * @param  同步信号量控制块
 *
 * @return 信号量施放成功标志位
 */
mos_err_t mos_ipc_sync_give(mos_sync_t *mos_sync_ipc)
{
    register mos_base_t temp;
    mos_bool_t need_scheduler = FALSE;

    if (mos_sync_ipc == NULL) return MOS_ENULL;

    /* 关闭中断 */
    temp = mos_port_interrupt_disable_temp();

    if(!mos_list_is_empty(&mos_sync_ipc->sync_ipc.block_list))
    {
        mos_ipc_task_resume(&mos_sync_ipc->sync_ipc);
        need_scheduler = TRUE;
    }
    else
    {
        ++mos_sync_ipc->value;
    }

    /* 开启中断 */
    mos_port_interrupt_enable_temp(temp);

    if (need_scheduler == TRUE)
    {
        /* 任务调度 */
        mos_task_scheduler();
    }

    return MOS_EOK;
}

