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
  * @brief   ���������
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#include "mos_task.h"
#include "mos_hw.h"
#include "mos_port.h"


/* Private -------------------------------------------------------------------*/
/* ��ʱ�б�����ʹ�ã����ڽ��ʱ��������� */
static mos_list_t g_mos_task_delay_list_1;
static mos_list_t g_mos_task_delay_list_2;
/* ������ʱ�б�ָ�� */
static mos_list_t * volatile g_mos_task_delay_list;
/* ������ʱ����б�ָ�� */
static mos_list_t * volatile g_mos_task_delay_overflow_list;

/* ��һ������������ʱ�� */
static volatile mos_uint32_t g_mos_task_next_task_unblock_tick = 0U;
/* ϵͳ��ǰʱ�������� */
static mos_tick_t g_mos_task_cur_tick_count = 0;

/* ��������� */
static mos_list_t  g_mos_task_list_ready_table[MOS_CONFIG_TASK_PRIORITY_MAX];
/* �������ȼ���־*/
static volatile mos_uint32_t g_mos_task_priority_flag_32bit;

/* ��������ջ */
static mos_uint8_t mos_task_idle_stack[MOS_CONFIG_TASK_IDLE_STACK_SIZE];
/* �������� ������ƿ� */
static mos_tcb_t idle_tcb;


/* ��ʼ���´��������� */
static void mos_task_init_new(mos_tcb_t *  const mos_new_tcb,	/* ������ƿ�ָ�� */
                              task_entry_fun 	   task_code,		/* ������� */
                              const char * const task_name,		/* �������ƣ��ַ�����ʽ */
                              const mos_uint8_t  task_pri,		/* �������ȼ� */
                              const mos_uint32_t parameter,		/* �����β� */
                              const mos_uint32_t stack_size,		/* ����ջ��С����λΪ�� */
                              const mos_uint32_t stack_start);
/* ����������ں��� */
static void mos_task_idle_entry(void *p_arg);
/* ��ʼ���������� */
static void mos_task_idle_init(void);
/* �����������ʱ�б� */
static void mos_task_insert_delay_list(mos_list_t *delay_list, mos_list_t *node, mos_tick_t mos_task_wake_tick);
/* ������ʱ���� */
static void mos_task_delay_process(const mos_uint32_t tick);
/* ϵͳʱ��������������л���ʱ�б� */
static void mos_task_switch_delay_list(void);							  
/* �л���ʱ�б��λ��һ����������ʱ�� */
static void mos_task_reset_next_task_unblock_time(void);


/* Public --------------------------------------------------------------------*/
/* ��ǰ�������������������ƿ� */
mos_tcb_t * volatile g_cur_task_tcb = NULL;


/* define --------------------------------------------------------------------*/
/**
 * @brief �������ȼ���־λ��ȡ������ȼ�(Ӳ������)
 */
#define mos_get_highest_priority(top_priority, priority_flag_32bit) \
    top_priority = (mos_uint32_t) __clz(priority_flag_32bit)

/**
 * @brief �������ȼ���־λ���ĵ�ǰ�������������������ƿ�
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
 * @brief  ���񴴽�
 * @param  ������ƿ�
 * @param  ������ں���
 * @param  ��������
 * @param  �������ȼ�
 * @param  �����β�
 * @param  ����ջ��С
 * @param  ����ջ��ʼ��ַ
 *
 * @return ���񴴽������־
 */
mos_err_t mos_task_create(	mos_tcb_t *  const task_tcb,	/* ������ƿ�ָ�� */
                            task_entry_fun 	   task_code,	/* ������� */
                            const char * const task_name,	/* �������ƣ��ַ�����ʽ */
                            const mos_uint8_t  task_pri,	/* �������ȼ� */
                            const mos_uint32_t parameter,	/* �����β� */
                            const mos_uint32_t stack_size,	/* ����ջ��С����λΪ�� */
                            const mos_uint32_t stack_start)	/* ����ջ��ʼ��ַ */
{
    mos_tcb_t *mos_new_tcb;
    mos_err_t ret = 0;

    if((task_tcb != NULL) && ((mos_uint32_t *)stack_start != NULL))
    {
        mos_new_tcb = task_tcb;
        /* �����µ����� */
        mos_task_init_new(	mos_new_tcb,	/* ������ƿ�ָ�� */
                            task_code,		/* ������� */
                            task_name,		/* �������ƣ��ַ�����ʽ */
                            task_pri,		/* �������ȼ� */
                            parameter,		/* �����β� */
                            stack_size,		/* ����ջ��С����λΪ�� */
                            stack_start);	/* ����ջ��ʼ��ַ */

        /* �������������б� */
        mos_task_insert_ready_table_list(g_mos_task_list_ready_table, mos_new_tcb);
    }
    else
    {
        ret = 0;
    }

    /* ��������״̬*/
    return ret;
}

/**
 * @brief �����������ʼ��
 *
 * @return �����������ʼ�������־
 */
mos_err_t mos_task_scheduler_init(void)
{
    mos_ubase_t priortiy;

    /* ����������ʼ�� */
    for(priortiy = 0U; priortiy < MOS_CONFIG_TASK_PRIORITY_MAX; priortiy++ )
    {
        mos_list_init(&g_mos_task_list_ready_table[priortiy]);
    }

    /*������ʱ�б��ʼ��*/
    g_mos_task_delay_list = &g_mos_task_delay_list_1;
    mos_list_init(g_mos_task_delay_list);

    /*������ʱ����б��ʼ��*/
    g_mos_task_delay_overflow_list = &g_mos_task_delay_list_2;
    mos_list_init(g_mos_task_delay_overflow_list);

    /* ��һ������������ʱ���ʼ��Ϊ�����ʱʱ�� */
    g_mos_task_next_task_unblock_tick = MOS_MAX_DELAY;

    return 0;
}

/**
 * @brief ��һ�ο����������
 *
 * @return ����������ȴ����־
 */
mos_err_t mos_task_scheduler_start(void)
{
    /* ��ʼ���������� */
    mos_task_idle_init();

    /* ��ȡ��һ��Ҫ���е�������ȼ����� */
    mos_task_select_highest_priority();

    /* ��һ���������� */
    mos_port_start_first_task();

    /* ���ᵽ�� */
    return 0;
}

/**
 * @brief �������
 */
mos_err_t mos_task_scheduler(void)
{
    mos_port_task_scheduler();

    return 0;
}

/**
 * @brief ����ѡ��,������g_cur_task_tcb
 */
void mos_task_switch_context(void)
{
    /* ��ȡ���ȼ���ߵľ��������TCB��Ȼ����µ�g_cur_task_tcb */
    mos_task_select_highest_priority();
}

/**
 * @brief ������ʱ
 */
void mos_task_delay(const mos_uint32_t tick)
{
    register mos_base_t temp;
    /* ʧ���ж� */
    temp = mos_port_entry_critical();

    /* ��������뵽��ʱ�б� */
    mos_task_delay_process(tick);

    /* ʹ���ж� */
    mos_port_exit_critical(temp);

    /* �����л� */
    mos_task_scheduler();
}

/**
 * @brief �������������б�
 */
void mos_task_insert_ready_table_list(mos_list_t *task_list_ready_table, mos_tcb_t *mos_tcb)
{
    /* �������������б� */
    mos_list_head_insert(&(task_list_ready_table[mos_tcb->task_priority]), &(mos_tcb->task_list));
    /* ���ö�Ӧλ���ȼ���־ */
    g_mos_task_priority_flag_32bit |= (1UL << (31UL - mos_tcb->task_priority));
}

/**
 * @brief ������Ӿ����б�ɾ��
 */
void mos_task_remove_ready_table_list(mos_list_t *task_list_ready_table, mos_tcb_t *mos_tcb)
{
    /* ������Ӿ����б����Ƴ� */
    mos_list_node_delete(&(mos_tcb->task_list));

    /* �����������ȼ�λͼ�ж�Ӧ��λ��� */
    if (mos_list_is_empty(&(task_list_ready_table[mos_tcb->task_priority])))
    {
        g_mos_task_priority_flag_32bit &= ~(1UL << (31UL - mos_tcb->task_priority));

    }
}

/**
 * @brief ϵͳ��ǰʱ����������������
 */
void mos_task_tickcount_increase(void)
{
    mos_tcb_t *mos_tcb;
    g_mos_task_cur_tick_count++;

    /* ���xConstTickCount��������л���ʱ�б� */
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
                /* ��ʱ�б�Ϊ�գ�����g_mos_task_next_task_unblock_tickΪ���ܵ����ֵ */
                g_mos_task_next_task_unblock_tick = MOS_MAX_DELAY;
                break;
            }
            else /* ��ʱ�б�Ϊ�� */
            {
                mos_tcb = MOS_LIST_ENTRY(g_mos_task_delay_list->next, mos_tcb_t, task_list);

                /* ֱ������ʱ�б���������ʱ���ڵ������Ƴ�������forѭ�� */
                if(g_mos_task_cur_tick_count < mos_tcb-> task_tick_wake)
                {
                    g_mos_task_next_task_unblock_tick = g_mos_task_cur_tick_count;
                    break;
                }

                /* ���������ʱ�б��Ƴ��������ȴ�״̬ */
                mos_list_node_delete(&mos_tcb->task_list);
                /* ����������ƿ��task_tick_wake */
                mos_tcb->task_tick_wake = 0L;

                /* ������ȴ���������ӵ������б� */
                mos_task_insert_ready_table_list(g_mos_task_list_ready_table, mos_tcb);

            }
        }
    }

    /* ϵͳ���� */
    mos_task_scheduler();
}


/* Private Fun----------------------------------------------------------------*/
/**
 * @brief ��ʼ���´���������
 */
static void mos_task_init_new(mos_tcb_t *  const mos_new_tcb,	/* ������ƿ�ָ�� */
                              task_entry_fun 	   task_code,		/* ������� */
                              const char * const task_name,		/* �������ƣ��ַ�����ʽ */
                              const mos_uint8_t  task_pri,		/* �������ȼ� */
                              const mos_uint32_t parameter,		/* �����β� */
                              const mos_uint32_t stack_size,		/* ����ջ��С����λΪ�� */
                              const mos_uint32_t stack_start)		/* ����ջ��ʼ��ַ */
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
    mos_new_tcb->task_state = 0;	//��ʱ
    mos_list_init(&mos_new_tcb->task_list);
}

/**
 * @brief ����������ں���
 */
static void mos_task_idle_entry(void *p_arg)
{
    while (1)
    {

    }
}

/**
 * @brief ��ʼ����������
 */
static void mos_task_idle_init(void)
{
    /* ��ʼ��idle */
    mos_task_create(&idle_tcb,
                    (task_entry_fun)mos_task_idle_entry,
                    "idle_task",
                    MOS_CONFIG_TASK_PRIORITY_MAX - 1,
                    (mos_uint32_t)NULL,
                    MOS_CONFIG_TASK_IDLE_STACK_SIZE,
                    (mos_uint32_t)mos_task_idle_stack);
}

/**
 * @brief  �����������ʱ�б�
 * @param  ������ʱ�б�
 * @param  ������������
 * @param  ϵͳ���Ѹ�����ʱ��
 */
static void mos_task_insert_delay_list(mos_list_t *delay_list, mos_list_t *task_node, mos_tick_t mos_task_wake_tick)
{
    mos_list_t *iterator;
    mos_tcb_t  *mos_delay_tcb;
    iterator = delay_list;
	
	/* ���ջ���ʱ��task_tick_wake��С�����ҵ�task_node����� */
    while (iterator->next != delay_list)
    {
        mos_delay_tcb = MOS_LIST_ENTRY(iterator->next, mos_tcb_t, task_list);

        if (mos_delay_tcb->task_tick_wake > mos_task_wake_tick)
        {
            break;
        }

        iterator = iterator->next;
    }

	/* �����������ʱ�б� */
    mos_list_head_insert(iterator, task_node);
}

/**
 * @brief ������ʱ����
 */
static void mos_task_delay_process(const mos_uint32_t tick)
{
    mos_tick_t mos_task_wake_tick;

    /* ��ȡϵͳʱ��������g_mos_tick_cur_tick_count��ֵ */
    const mos_tick_t mos_cur_tick_count = g_mos_task_cur_tick_count;

    /* ������Ӿ����б����Ƴ� */
    mos_task_remove_ready_table_list(g_mos_task_list_ready_table, g_cur_task_tcb);

    /* ������ʱ����ʱ��ϵͳʱ��������mos_cur_tick_count��ֵ�Ƕ��� */
    mos_task_wake_tick = mos_cur_tick_count + tick;

    /* �޸ĵ�ǰ�б���ʱ�� */
    g_cur_task_tcb->task_tick_wake = mos_task_wake_tick;

    /* ��� */
    if(mos_task_wake_tick < mos_cur_tick_count )
    {
        mos_task_insert_delay_list(g_mos_task_delay_overflow_list, &(g_cur_task_tcb->task_list), mos_task_wake_tick);
    }
    /* û����� */
    else
    {
        mos_task_insert_delay_list(g_mos_task_delay_list, &(g_cur_task_tcb->task_list), mos_task_wake_tick);

        /* ������һ���������ʱ�̱���g_mos_task_next_task_unblock_tick��ֵ */
        if(mos_task_wake_tick < g_mos_task_next_task_unblock_tick)
        {
            g_mos_task_next_task_unblock_tick = mos_task_wake_tick;
        }
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
 * @brief �л���ʱ�б��λ��һ����������ʱ��
 */
static void mos_task_reset_next_task_unblock_time(void)
{
    mos_tcb_t *mos_tcb;

	/* ����ʱ�б�Ϊ��ʱ��������ʱ��Ϊ���ֵ */
    if(mos_list_is_empty(g_mos_task_delay_list))
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
