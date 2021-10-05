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
  * @brief   �����ͨ��
  ******************************************************************************
  * @note
  *          2021-09-15 Mbwide:��ʼ�汾
  *          2021-10-05 Mbwide:���ͬ���ź����뻥���ź���
  ******************************************************************************
  */

#include "mos_ipc.h"
#include "mos_port.h"
#include "mos_task.h"
#include "mos_tick.h"
#include "mos_heap.h"

/* Private Fun----------------------------------------------------------------*/
/**
 * @brief  ����IPC���������(����)
 * @param  ��Ҫ���������
 * @param  �ź���ͷ����
 *
 * @return �������ɹ���־λ
 */
static mos_err_t mos_ipc_task_suspend(mos_tcb_t *to_suspend_task, mos_ipc_t *mos_ipc)
{
    /* �Ƿ���ַ */
    if (to_suspend_task == NULL || mos_ipc == NULL)
    {
        return MOS_ENULL;
    }

    /* ����������뵽�����б� */
    mos_task_insert_suspend_list(to_suspend_task);

    return MOS_EOK;
}

/**
 * @brief  ����IPC������ָ�
 * @param  �ź���ͷ����
 *
 * @return ����ָ��ɹ���־λ
 */
static mos_err_t mos_ipc_task_resume(mos_ipc_t *mos_ipc)
{

    mos_tcb_t * to_resume_task = NULL;

    /* ��ȡ��Ҫ�ָ���������ƿ� */
    to_resume_task = MOS_LIST_ENTRY(mos_ipc->block_list.next, mos_tcb_t, task_ipc_list);

    /* �Ƿ���ַ */
    if (to_resume_task == NULL || mos_ipc == NULL)
    {
        return MOS_ENULL;
    }

    /* ɾ������������� */
    mos_list_node_delete(&to_resume_task->task_ipc_list);

    /* ������״̬����������б� */
    mos_task_insert_ready_table_list(to_resume_task);

    return MOS_EOK;
}

/**
 * @brief  �����ź������ȼ��̳�
 * @param  �����ź������ƿ�
 *
 * @return �ź��������ɹ���־λ
 */
static mos_err_t mos_ipc_mutex_priority_inherit(mos_tcb_t *mutex_owner_tcb, mos_uint16_t mutex_take_tcb_priority)
{
    /* ������������������ȼ�����ӵ�иû��������������ȼ��̳� */
    if (mutex_take_tcb_priority < mutex_owner_tcb->task_priority)
    {
        /* �������ȼ��������²�������б� */
        mutex_owner_tcb->task_priority = mutex_take_tcb_priority;
        /* ��������Ӿ����б�ɾ�� */
        mos_task_remove_ready_table_list(mutex_owner_tcb);
        /* ����������뵽�����б� */
        mos_task_insert_ready_table_list(mutex_owner_tcb);

        return MOS_EOK;
    }

    return MOS_ERROR;
}

/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief  ����ͬ���ź���
 *
 * @return ͬ���ź���ͷ
 */
mos_sync_t * mos_ipc_sync_creat(void)
{
    mos_sync_t *to_creat_sync_ipc = NULL;
	
    to_creat_sync_ipc = (mos_sync_t *)mos_malloc(sizeof(mos_sync_t));
	
	/* �Ƿ���ַ */
    if(to_creat_sync_ipc == NULL) return to_creat_sync_ipc;

    /* ��ʼ��ͬ���ź��� */
    to_creat_sync_ipc->sync_ipc.flag = SYNC_IPC;
    mos_list_init(&(to_creat_sync_ipc->sync_ipc.block_list));
    to_creat_sync_ipc->value = 0;

    return to_creat_sync_ipc;
}

/**
 * @brief  ��ȡͬ���ź���
 * @param  ͬ���ź������ƿ�
 * @param  ����ʱ��
 *
 * @return �ź�����ȡ�ɹ���־λ
 */
mos_err_t mos_ipc_sync_take(mos_sync_t *mos_sync_ipc, mos_uint32_t to_block_tick)
{
    register mos_base_t temp;
    mos_tcb_t * cur_tcb = NULL;

    if (mos_sync_ipc == NULL) return MOS_ENULL;

    /* �ر��ж� */
    temp = mos_port_interrupt_disable_temp();

    /* ����Դ */
    if (mos_sync_ipc->value > 0)
    {
        --mos_sync_ipc->value;
        /* �����ж� */
        mos_port_interrupt_enable_temp(temp);
    }
    /* û����Դ */
    else
    {
        /* û������ʱ��*/
        if (to_block_tick == 0)
        {
            /* �����ж� */
            mos_port_interrupt_enable_temp(temp);
            return MOS_EEMPTY;
        }
        /* ������ʱ��*/
        else
        {
            /* ��ȡ��ǰ������ƿ�*/
            cur_tcb = mos_task_get_cur_tcb();

            /* ���� */
            if (to_block_tick == MOS_MAX_DELAY)
            {
                mos_ipc_task_suspend(cur_tcb, &mos_sync_ipc->sync_ipc);
            }
            /* ���� */
            else
            {
                mos_tick_delay_process(cur_tcb, to_block_tick);
            }

            /* ������IPC�������ҵ����ź�����,β�� */
            mos_list_tail_insert(&mos_sync_ipc->sync_ipc.block_list, &cur_tcb->task_ipc_list);

            /* �����ж� */
            mos_port_interrupt_enable_temp(temp);
            /* ������� */
            mos_task_scheduler();
        }
    }

    return MOS_EOK;
}

/**
 * @brief  ʩ��ͬ���ź���
 * @param  ͬ���ź������ƿ�
 *
 * @return �ź���ʩ�ųɹ���־λ
 */
mos_err_t mos_ipc_sync_give(mos_sync_t *mos_sync_ipc)
{
    register mos_base_t temp;
    mos_bool_t need_scheduler = FALSE;

    if (mos_sync_ipc == NULL) return MOS_ENULL;

    /* �ر��ж� */
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

    /* �����ж� */
    mos_port_interrupt_enable_temp(temp);

    if (need_scheduler == TRUE)
    {
        /* ������� */
        mos_task_scheduler();
    }

    return MOS_EOK;
}
/**
 * @brief  ���������ź���
 *
 * @return �ź��������ɹ���־λ
 */
mos_mutex_t * mos_ipc_mutex_creat(void)
{
	mos_mutex_t *to_creat_mutex_ipc = NULL;
	
    to_creat_mutex_ipc = (mos_mutex_t *)mos_malloc(sizeof(mos_mutex_t));
	
	/* �Ƿ���ַ */
    if(to_creat_mutex_ipc == NULL) return to_creat_mutex_ipc;

    /* ��ʼ�������ź��� */
    to_creat_mutex_ipc->mutex_ipc.flag = MUTEX_IPC;
    to_creat_mutex_ipc->mutex_owner = NULL;
    mos_list_init(&(to_creat_mutex_ipc->mutex_ipc.block_list));
    to_creat_mutex_ipc->value = 1;

    return to_creat_mutex_ipc;
}

/**
 * @brief  ��ȡ�����ź���
 * @param  �����ź������ƿ�
 * @param  ����ʱ��
 *
 * @return �ź�����ȡ�ɹ���־λ
 */
mos_err_t mos_ipc_mutex_take(mos_mutex_t *to_take_mutex_ipc, mos_uint32_t to_block_tick)
{
    register mos_base_t temp;
    mos_tcb_t * cur_tcb = NULL;

    if (to_take_mutex_ipc == NULL) return MOS_ENULL;

    /* �ر��ж� */
    temp = mos_port_interrupt_disable_temp();

    /* ��ȡ��ǰ������ƿ�*/
    cur_tcb = mos_task_get_cur_tcb();

    /* ����Դ */
    if (to_take_mutex_ipc->value > 0)
    {
        /* �޸Ļ����ź���ӵ���� */
        to_take_mutex_ipc->mutex_owner = cur_tcb;
        /* �޸Ļ����ź���ֵ */
        --to_take_mutex_ipc->value;

        /* �����ж� */
        mos_port_interrupt_enable_temp(temp);
    }
    /* û����Դ */
    else
    {
        /* û������ʱ��*/
        if (to_block_tick == 0)
        {
            /* �����ж� */
            mos_port_interrupt_enable_temp(temp);
            return MOS_EEMPTY;
        }
        /* ������ʱ��*/
        else
        {
            /* ���� */
            if (to_block_tick == MOS_MAX_DELAY)
            {
                mos_ipc_task_suspend(cur_tcb, &to_take_mutex_ipc->mutex_ipc);

            }
            /* ���� */
            else
            {
                mos_tick_delay_process(cur_tcb, to_block_tick);
            }

            /* ���ȼ��̳� */
            mos_ipc_mutex_priority_inherit(to_take_mutex_ipc->mutex_owner, cur_tcb->task_priority);

            /* ������IPC�����б�ҵ����ź�����,β�� */
            mos_list_tail_insert(&to_take_mutex_ipc->mutex_ipc.block_list, &cur_tcb->task_ipc_list);

            /* �����ж� */
            mos_port_interrupt_enable_temp(temp);
            /* ������� */
            mos_task_scheduler();
        }
    }

    return MOS_EOK;
}

/**
 * @brief  ʩ�Ż����ź���
 * @param  �����ź������ƿ�
 *
 * @return �ź���ʩ�ųɹ���־λ
 */
mos_err_t mos_ipc_mutex_give(mos_mutex_t *to_give_mutex_ipc)
{
    register mos_base_t temp;
    mos_bool_t need_scheduler = FALSE;
    mos_tcb_t * to_resume_task = NULL;

    if (to_give_mutex_ipc == NULL) return MOS_ENULL;

    /* �ر��ж� */
    temp = mos_port_interrupt_disable_temp();

    /* �ָ����ȼ� */
    if (to_give_mutex_ipc->mutex_owner->task_priority != to_give_mutex_ipc->mutex_owner->task_base_priority)
    {
        to_give_mutex_ipc->mutex_owner->task_priority = to_give_mutex_ipc->mutex_owner->task_base_priority;
        /* ��������Ӿ����б�ɾ�� */
        mos_task_remove_ready_table_list(to_give_mutex_ipc->mutex_owner);
        /* ����������뵽�����б� */
        mos_task_insert_ready_table_list(to_give_mutex_ipc->mutex_owner);

        need_scheduler = TRUE;
    }

    to_give_mutex_ipc->mutex_owner = NULL;

    if(!mos_list_is_empty(&to_give_mutex_ipc->mutex_ipc.block_list))
    {
        /* ��ȡ��Ҫ�ָ���������ƿ� */
        to_resume_task = MOS_LIST_ENTRY(to_give_mutex_ipc->mutex_ipc.block_list.next, mos_tcb_t, task_ipc_list);
        mos_ipc_task_resume(&to_give_mutex_ipc->mutex_ipc);
        to_give_mutex_ipc->mutex_owner = to_resume_task;

        need_scheduler = TRUE;
    }
    else
    {
        ++to_give_mutex_ipc->value;
    }

    /* �����ж� */
    mos_port_interrupt_enable_temp(temp);

    if (need_scheduler == TRUE)
    {
        /* ������� */
        mos_task_scheduler();
    }

    return MOS_EOK;
}

