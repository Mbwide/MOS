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
  * @date    2021-10-05
  * @brief   任务间通信
  ******************************************************************************
  * @note
  *          2021-09-15 Mbwide:初始版本
  *          2021-10-05 Mbwide:添加同步信号量与互斥信号量
  ******************************************************************************
  */

#include "mos_ipc.h"
#include "mos_port.h"
#include "mos_task.h"
#include "mos_tick.h"
#include "mos_heap.h"

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

    /* 将任务结点插入到挂起列表 */
    mos_task_insert_suspend_list(to_suspend_task);

    return MOS_EOK;
}

/**
 * @brief  用于IPC的任务恢复
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

    /* 将任务状态结点插入就绪列表 */
    mos_task_insert_ready_table_list(to_resume_task);

    return MOS_EOK;
}

/**
 * @brief  互斥信号量优先级继承
 * @param  互斥信号量控制块
 *
 * @return 信号量创建成功标志位
 */
static mos_err_t mos_ipc_mutex_priority_inherit(mos_tcb_t *mutex_owner_tcb, mos_uint16_t mutex_take_tcb_priority)
{
    /* 如果被阻塞的任务优先级高于拥有该互斥量的任务，优先级继承 */
    if (mutex_take_tcb_priority < mutex_owner_tcb->task_priority)
    {
        /* 更新优先级，并重新插入就绪列表 */
        mutex_owner_tcb->task_priority = mutex_take_tcb_priority;
        /* 将任务结点从就绪列表删除 */
        mos_task_remove_ready_table_list(mutex_owner_tcb);
        /* 将任务结点插入到就绪列表 */
        mos_task_insert_ready_table_list(mutex_owner_tcb);

        return MOS_EOK;
    }

    return MOS_ERROR;
}

/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief  创建同步信号量
 *
 * @return 同步信号量头
 */
mos_sync_t * mos_ipc_sync_creat(void)
{
    mos_sync_t *to_creat_sync_ipc = NULL;
	
    to_creat_sync_ipc = (mos_sync_t *)mos_malloc(sizeof(mos_sync_t));
	
	/* 非法地址 */
    if(to_creat_sync_ipc == NULL) return to_creat_sync_ipc;

    /* 初始化同步信号量 */
    to_creat_sync_ipc->sync_ipc.flag = SYNC_IPC;
    mos_list_init(&(to_creat_sync_ipc->sync_ipc.block_list));
    to_creat_sync_ipc->value = 0;

    return to_creat_sync_ipc;
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
        /* 开启中断 */
        mos_port_interrupt_enable_temp(temp);
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

            /* 将任务IPC阻塞结点挂到该信号量下,尾插 */
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
 * @brief  施放同步信号量
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
/**
 * @brief  创建互斥信号量
 *
 * @return 信号量创建成功标志位
 */
mos_mutex_t * mos_ipc_mutex_creat(void)
{
	mos_mutex_t *to_creat_mutex_ipc = NULL;
	
    to_creat_mutex_ipc = (mos_mutex_t *)mos_malloc(sizeof(mos_mutex_t));
	
	/* 非法地址 */
    if(to_creat_mutex_ipc == NULL) return to_creat_mutex_ipc;

    /* 初始化互斥信号量 */
    to_creat_mutex_ipc->mutex_ipc.flag = MUTEX_IPC;
    to_creat_mutex_ipc->mutex_owner = NULL;
    mos_list_init(&(to_creat_mutex_ipc->mutex_ipc.block_list));
    to_creat_mutex_ipc->value = 1;

    return to_creat_mutex_ipc;
}

/**
 * @brief  获取互斥信号量
 * @param  互斥信号量控制块
 * @param  阻塞时间
 *
 * @return 信号量获取成功标志位
 */
mos_err_t mos_ipc_mutex_take(mos_mutex_t *to_take_mutex_ipc, mos_uint32_t to_block_tick)
{
    register mos_base_t temp;
    mos_tcb_t * cur_tcb = NULL;

    if (to_take_mutex_ipc == NULL) return MOS_ENULL;

    /* 关闭中断 */
    temp = mos_port_interrupt_disable_temp();

    /* 获取当前任务控制块*/
    cur_tcb = mos_task_get_cur_tcb();

    /* 有资源 */
    if (to_take_mutex_ipc->value > 0)
    {
        /* 修改互斥信号量拥有者 */
        to_take_mutex_ipc->mutex_owner = cur_tcb;
        /* 修改互斥信号量值 */
        --to_take_mutex_ipc->value;

        /* 开启中断 */
        mos_port_interrupt_enable_temp(temp);
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
            /* 挂起 */
            if (to_block_tick == MOS_MAX_DELAY)
            {
                mos_ipc_task_suspend(cur_tcb, &to_take_mutex_ipc->mutex_ipc);

            }
            /* 阻塞 */
            else
            {
                mos_tick_delay_process(cur_tcb, to_block_tick);
            }

            /* 优先级继承 */
            mos_ipc_mutex_priority_inherit(to_take_mutex_ipc->mutex_owner, cur_tcb->task_priority);

            /* 将任务IPC阻塞列表挂到该信号量下,尾插 */
            mos_list_tail_insert(&to_take_mutex_ipc->mutex_ipc.block_list, &cur_tcb->task_ipc_list);

            /* 开启中断 */
            mos_port_interrupt_enable_temp(temp);
            /* 任务调度 */
            mos_task_scheduler();
        }
    }

    return MOS_EOK;
}

/**
 * @brief  施放互斥信号量
 * @param  互斥信号量控制块
 *
 * @return 信号量施放成功标志位
 */
mos_err_t mos_ipc_mutex_give(mos_mutex_t *to_give_mutex_ipc)
{
    register mos_base_t temp;
    mos_bool_t need_scheduler = FALSE;
    mos_tcb_t * to_resume_task = NULL;

    if (to_give_mutex_ipc == NULL) return MOS_ENULL;

    /* 关闭中断 */
    temp = mos_port_interrupt_disable_temp();

    /* 恢复优先级 */
    if (to_give_mutex_ipc->mutex_owner->task_priority != to_give_mutex_ipc->mutex_owner->task_base_priority)
    {
        to_give_mutex_ipc->mutex_owner->task_priority = to_give_mutex_ipc->mutex_owner->task_base_priority;
        /* 将任务结点从就绪列表删除 */
        mos_task_remove_ready_table_list(to_give_mutex_ipc->mutex_owner);
        /* 将任务结点插入到就绪列表 */
        mos_task_insert_ready_table_list(to_give_mutex_ipc->mutex_owner);

        need_scheduler = TRUE;
    }

    to_give_mutex_ipc->mutex_owner = NULL;

    if(!mos_list_is_empty(&to_give_mutex_ipc->mutex_ipc.block_list))
    {
        /* 获取将要恢复的任务控制块 */
        to_resume_task = MOS_LIST_ENTRY(to_give_mutex_ipc->mutex_ipc.block_list.next, mos_tcb_t, task_ipc_list);
        mos_ipc_task_resume(&to_give_mutex_ipc->mutex_ipc);
        to_give_mutex_ipc->mutex_owner = to_resume_task;

        need_scheduler = TRUE;
    }
    else
    {
        ++to_give_mutex_ipc->value;
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

