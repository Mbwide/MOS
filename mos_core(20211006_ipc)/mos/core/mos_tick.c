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
  * @brief   ��ʱ��
  ******************************************************************************
  * @note
  *          2021-08-20 Mbwide:��ʼ�汾
  *          2021-10-04 Mbwide:�ṹ�Ż�
  ******************************************************************************
  */

#include "mos_tick.h"
#include "mos_sys.h"
#include "mos_task.h"
#include "mos_port.h"

/* Private -------------------------------------------------------------------*/
/* ϵͳ��ǰʱ�������� */
static mos_tick_t g_mos_task_cur_tick_count = 0;
/* ������ʱ����б�ָ�� */
static mos_list_t * volatile g_mos_task_delay_overflow_list;


/* Public --------------------------------------------------------------------*/
/* ��һ������������ʱ�� */
volatile mos_uint32_t g_mos_task_next_task_unblock_tick = 0U;

/* ������ʱ�б�ָ�� */
extern mos_list_t * volatile g_mos_task_delay_list;
/* ��������� */
extern mos_list_t  g_mos_task_list_ready_table[MOS_CONFIG_TASK_PRIORITY_MAX];
/* ��ǰ�������������������ƿ� */
extern mos_tcb_t * volatile g_cur_task_tcb;


/* Private Fun----------------------------------------------------------------*/
/**
 * @brief �л���ʱ�б��λ��һ����������ʱ��
 */
static void mos_task_reset_next_task_unblock_time(void)
{
    mos_tcb_t *mos_tcb;

    /* ����ʱ�б�Ϊ��ʱ��������ʱ��Ϊ���ֵ */
    if (mos_list_is_empty(g_mos_task_delay_list))
    {
        g_mos_task_next_task_unblock_tick = MOS_MAX_DELAY;
    }
    /* ����ʱ�б�Ϊ��ʱ��ȡ��һ����������ʱ��*/
    else
    {
        mos_tcb = MOS_LIST_ENTRY(g_mos_task_delay_list->next, mos_tcb_t, task_list);
        g_mos_task_next_task_unblock_tick = mos_tcb->task_tick_wake;
    }
}

/**
 * @brief ϵͳʱ������������л���ʱ�б�
 */
static void mos_task_switch_delay_list(void)
{
    /* ����������ʱ�б�ָ���������ʱ����б�ָ�� */
    mos_list_t *list_temp;
    list_temp = g_mos_task_delay_list;
    g_mos_task_delay_list = g_mos_task_delay_overflow_list;
    g_mos_task_delay_overflow_list = list_temp;

    /* ��������������� */
    /* g_num_of_over_flow++; */

    /* �л���ʱ�б��λ��һ����������ʱ�� */
    mos_task_reset_next_task_unblock_time();
}

/**
 * @brief  ϵͳ��ǰʱ����������������
 *
 * @return �Ƿ�ִ��������Ⱥ���
 */
static mos_bool_t mos_tick_tickcount_increase(void)
{
    mos_tcb_t *mos_tcb;
    g_mos_task_cur_tick_count++;

    /*�Ƿ����*/
    mos_bool_t switch_flag = FALSE;

    /* ���g_mos_task_cur_tick_count��������л���ʱ�б� */
    if (g_mos_task_cur_tick_count == (mos_tick_t)0U)
    {
        mos_task_switch_delay_list();
    }

    if (g_mos_task_cur_tick_count >= g_mos_task_next_task_unblock_tick)
    {
        for ( ;; )
        {
            /* ��ʱ�б�Ϊ�գ�����g_mos_task_next_task_unblock_tickΪ���ܵ����ֵ */
            if(mos_list_is_empty(g_mos_task_delay_list))
            {
                g_mos_task_next_task_unblock_tick = MOS_MAX_DELAY;
                break;
            }
            /* ��ʱ�б�Ϊ�� */
            else
            {
                mos_tcb = MOS_LIST_ENTRY(g_mos_task_delay_list->next, mos_tcb_t, task_list);

                /* ֱ������ʱ�б���������ʱ���ڵ������Ƴ�������forѭ�� */
                if(g_mos_task_cur_tick_count < mos_tcb->task_tick_wake)
                {
                    g_mos_task_next_task_unblock_tick = mos_tcb->task_tick_wake;
                    break;
                }

                /* ���������ʱ�б��Ƴ��������ȴ�״̬ */
                mos_list_node_delete(&mos_tcb->task_list);
                /* ����������ƿ��task_tick_wake */
                mos_tcb->task_tick_wake = 0L;

                /* ʹ��IPCʱ�ж��Ƿ���Ϊ�ź��������� */
#if (MOS_CONFIG_USE_IPC == YES)

                if (!mos_list_is_empty(&mos_tcb->task_ipc_list))
                {
                    mos_list_node_delete(&mos_tcb->task_ipc_list);
                }

#endif

                /* �и������ȼ�������Ҫ�����л�����ֵԽ�����ȼ�Խ�� */
                if (g_cur_task_tcb->task_priority > mos_tcb->task_priority)
                {
                    /* ϵͳ���ȱ�־ */
                    switch_flag = TRUE;
                }

                /* ������ȴ���������ӵ������б� */
                mos_task_insert_ready_table_list(mos_tcb);
            }
        }
    }

    /* ʹ��ʱ��Ƭ��ÿ�ζ�Ҫִ�е��ȳ��� */
#if (MOS_CONFIG_USE_TIME_SLICING == YES)
    {
        /* ϵͳ���ȱ�־ */
        switch_flag = TRUE;
    }
#endif

    return switch_flag;
}

/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief ϵͳʱ�ӳ�ʼ��
 */
void mos_tick_systick_init(void)
{
    /* systickӲ���ӿڳ�ʼ�� */
    mos_port_systick_init();

    /* ��һ������������ʱ���ʼ��Ϊ�����ʱʱ�� */
    g_mos_task_next_task_unblock_tick = MOS_MAX_DELAY;
}

/**
 * @brief ϵͳʱ���жϷ�����
 */
void mos_tick_systick_handler(void)
{
    /* �����ж� */
    mos_sys_interrupt_enter();

    /* ʱ������ */
    if (mos_tick_tickcount_increase() == TRUE)
    {
        /* ������� */
        mos_task_scheduler();
    }

    /* �뿪�ж� */
    mos_sys_interrupt_leave();
}

/**
 * @brief ��ȡ��ǰʱ�Ӽ���ֵ
 */
mos_tick_t mos_tick_get_cur_tick_count(void)
{
    return g_mos_task_cur_tick_count;
}

/**
 * @brief ������ʱ����
 */
void mos_tick_delay_process(mos_tcb_t *mos_tcb, const mos_uint32_t tick)
{
    mos_tick_t mos_task_wake_tick;

    /* ��ȡϵͳʱ��������g_mos_tick_cur_tick_count��ֵ */
    mos_tick_t mos_cur_tick_count = mos_tick_get_cur_tick_count();

    /* ������Ӿ����б����Ƴ� */
    mos_task_remove_ready_table_list(g_cur_task_tcb);

    /* ������ʱ����ʱ��ϵͳʱ��������mos_cur_tick_count��ֵ�Ƕ��� */
    mos_task_wake_tick = mos_cur_tick_count + tick;

    /* �޸������б���ʱ�� */
    mos_tcb->task_tick_wake = mos_task_wake_tick;

    /* �޸�����״̬Ϊ����̬ */
    mos_tcb->task_state = TASK_BLOCK;

    /* ��� */
    if (mos_task_wake_tick < mos_cur_tick_count )
    {
        mos_task_insert_delay_list(g_mos_task_delay_overflow_list, &(g_cur_task_tcb->task_list), mos_task_wake_tick);
    }
    /* û����� */
    else
    {
        mos_task_insert_delay_list(g_mos_task_delay_list, &(g_cur_task_tcb->task_list), mos_task_wake_tick);

        /* ������һ���������ʱ�̱���g_mos_task_next_task_unblock_tick��ֵ */
        if (mos_task_wake_tick < g_mos_task_next_task_unblock_tick)
        {
            g_mos_task_next_task_unblock_tick = mos_task_wake_tick;
        }
    }
}

