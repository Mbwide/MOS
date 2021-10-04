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
  * @brief   �����ͨ��
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

    /* ������IPC�������ҵ����ź����� */
    mos_list_head_insert(&mos_ipc->block_list, &to_suspend_task->task_ipc_list);
   
	/* ����������뵽�����б� */
    mos_task_insert_suspend_list(to_suspend_task);

    return MOS_EOK;
}

/**
 * @brief  ����IPC������ָ�
 * @param  ��Ҫ�ָ�������
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
	
	/* ���������������б� */
    mos_task_insert_ready_table_list(to_resume_task);
	
    return MOS_EOK;
}

/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief  ����ͬ���ź���
 * @param  ͬ���ź������ƿ�
 *
 * @return �ź��������ɹ���־λ
 */
mos_err_t mos_ipc_sync_creat(mos_sync_t *sync_ipc)
{

    /* �Ƿ���ַ */
    if(sync_ipc == NULL) return MOS_ENULL;

    /* ��ʼ��ͬ���ź��� */
    sync_ipc->sync_ipc.flag = SYNC_IPC;
    mos_list_init(&(sync_ipc->sync_ipc.block_list));
    sync_ipc->value = 0;

    return MOS_EOK;
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

            /* ������IPC�����б�ҵ����ź�����,β�� */
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

