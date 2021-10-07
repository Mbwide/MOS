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
  * @brief   ���������
  ******************************************************************************
  * @note
  *          2021-08-20 Mbwide:��ʼ�汾
  *          2021-10-04 Mbwide:�ṹ�Ż�
  *          2021-10-07 Mbwide:���shell����
  ******************************************************************************
  */

#include "mos_task.h"
#include "mos_hw.h"
#include "mos_port.h"
#include "mos_heap.h"
#include "mos_tick.h"

/* Private -------------------------------------------------------------------*/
/* ��ʱ�б�����ʹ�ã����ڽ��ʱ��������� */
static mos_list_t g_mos_task_delay_list_1;
static mos_list_t g_mos_task_delay_list_2;
/* ������ʱ����б�ָ�� */
static mos_list_t * volatile g_mos_task_delay_overflow_list;
/* ���������б� */
//static mos_list_t g_mos_task_pend_list;
/* �������ȼ���־*/
static volatile mos_uint32_t g_mos_task_priority_flag_32bit;
/* ������ȿ�����־��0Ϊ���� */
static mos_ubase_t g_scheduler_flag;

#if MOS_CONFIG_USE_DYNAMIC_HEAP
/* �������� ������ƿ� */
static mos_tcb_t idle_tcb;
#else
/* �������� ������ƿ� */
static mos_tcb_t idle_tcb;
/* ��������ջ */
static mos_uint8_t mos_task_idle_stack[MOS_CONFIG_TASK_IDLE_STACK_SIZE];
#endif


/* Public --------------------------------------------------------------------*/
/* ��ǰ�������������������ƿ� */
mos_tcb_t * volatile g_cur_task_tcb = NULL;
/* ��������� */
mos_list_t  g_mos_task_list_ready_table[MOS_CONFIG_TASK_PRIORITY_MAX];
/* ��������б� */
mos_list_t g_mos_task_suspend_list;
/* ������ʱ�б�ָ�� */
mos_list_t * volatile g_mos_task_delay_list;
/* ��һ������������ʱ�� */
extern volatile mos_uint32_t g_mos_task_next_task_unblock_tick;

#if (MOS_CONFIG_USE_SHELL == YES)
/* shell�����б� */
extern mos_list_t g_mos_shell_task_list;
#endif


/* define --------------------------------------------------------------------*/
/**
 * @brief �������ȼ���־λ��ȡ������ȼ�(Ӳ������)
 */
#define mos_get_highest_priority(top_priority, priority_flag_32bit) \
    top_priority = (mos_uint32_t) __clz(priority_flag_32bit)

/**
 * @brief �������ȼ���־λ���ĵ�ǰ�������������������ƿ�
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
 * @brief ��ʼ���´���������
 */
static void mos_task_init_new(mos_tcb_t *  const mos_new_tcb,  /* ������ƿ�ָ�� */
                              task_entry_fun 	   task_code,  /* ������� */
                              const char * const task_name,    /* �������ƣ��ַ�����ʽ */
                              const mos_uint8_t  task_pri,     /* �������ȼ� */
                              //const mos_uint32_t parameter,  /* �����β� */
                              const mos_uint32_t stack_size,   /* ����ջ��С����λΪ�� */
                              const mos_uint32_t stack_start)  /* ����ջ��ʼ��ַ */
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
	
    /* ����״̬�б� */
    mos_list_init(&mos_new_tcb->task_list);
	
#if (MOS_CONFIG_USE_SHELL == YES)
	/* ����shell�б� */
    mos_list_init(&mos_new_tcb->task_shell_list);
#endif
	
    /* IPC�����б� */
    mos_list_init(&mos_new_tcb->task_ipc_list);
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
#if MOS_CONFIG_USE_DYNAMIC_HEAP
    /* ������idle���� */
    mos_task_create(&idle_tcb,
                    (task_entry_fun)mos_task_idle_entry,
	                "IDLE_TASK",
                    MOS_CONFIG_TASK_PRIORITY_MAX - 1,
                    MOS_CONFIG_TASK_IDLE_STACK_SIZE);
#else
    /* ��̬����idle���� */
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
#if (MOS_CONFIG_USE_DYNAMIC_HEAP == YES)
mos_err_t mos_task_create( mos_tcb_t *  const task_tcb,   /* ������ƿ�ָ�� */
                           task_entry_fun 	  task_code,  /* ������� */
                           const char * const task_name,  /* �������ƣ��ַ�����ʽ */
                           const mos_uint8_t  task_pri,   /* �������ȼ� */
                           const mos_uint32_t stack_size) /* ����ջ��С����λΪ�� */
{
    mos_tcb_t *mos_new_tcb;
    mos_err_t ret = 0;

    mos_ubase_t *stack_start = (mos_ubase_t *)mos_malloc(stack_size);

    if ((task_tcb != NULL) && (stack_start != NULL))
    {
        mos_new_tcb = task_tcb;
        /* �����µ����� */
        mos_task_init_new( mos_new_tcb, /* ������ƿ�ָ�� */
                           task_code,   /* ������� */
		                   task_name,   /* �������ƣ��ַ�����ʽ */
                           task_pri,    /* �������ȼ� */
                           stack_size,  /* ����ջ��С����λΪ�� */
                           (mos_uint32_t)stack_start);	/* ����ջ��ʼ��ַ */

        /* �������������б� */
        mos_task_insert_ready_table_list(mos_new_tcb);

#if (MOS_CONFIG_USE_SHELL == YES)
	    /* ���������shell�б� */
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

    /* ��������״̬*/
    return ret;
}

#else
mos_err_t mos_task_create( mos_tcb_t *  const task_tcb,    /* ������ƿ�ָ�� */
                           task_entry_fun 	   task_code,  /* ������� */
                           //const char * const task_name, /* �������ƣ��ַ�����ʽ */
                           const mos_uint8_t  task_pri,    /* �������ȼ� */
                           //const mos_uint32_t parameter, /* �����β� */
                           const mos_uint32_t stack_size,  /* ����ջ��С����λΪ�� */
                           const mos_uint32_t stack_start) /* ����ջ��ʼ��ַ */
{
    mos_tcb_t *mos_new_tcb;
    mos_err_t ret = 0;

    if ((task_tcb != NULL) && ((mos_uint32_t *)stack_start != NULL))
    {
        mos_new_tcb = task_tcb;
        /* �����µ����� */
        mos_task_init_new( mos_new_tcb,  /* ������ƿ�ָ�� */
                           task_code,    /* ������� */
                           //task_name,  /* �������ƣ��ַ�����ʽ */
                           task_pri,     /* �������ȼ� */
                           //parameter,  /* �����β� */
                           stack_size,   /* ����ջ��С����λΪ�� */
                           stack_start); /* ����ջ��ʼ��ַ */

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
#endif

/**
 * @brief �����������ʼ��
 *
 * @return �����������ʼ�������־
 */
mos_err_t mos_task_scheduler_init(void)
{
    mos_ubase_t priortiy;

    /* ���������������־λ */
    g_scheduler_flag = 0;

    /* ����������ʼ�� */
    for (priortiy = 0U; priortiy < MOS_CONFIG_TASK_PRIORITY_MAX; priortiy++ )
    {
        mos_list_init(&g_mos_task_list_ready_table[priortiy]);
    }

    /* ��������б��ʼ�� */
    mos_list_init(&g_mos_task_suspend_list);

    /*������ʱ�б��ʼ��*/
    g_mos_task_delay_list = &g_mos_task_delay_list_1;
    mos_list_init(g_mos_task_delay_list);

    /*������ʱ����б��ʼ��*/
    g_mos_task_delay_overflow_list = &g_mos_task_delay_list_2;
    mos_list_init(g_mos_task_delay_overflow_list);

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

#if (MOS_CONFIG_USE_DEBUG_PRINTF == YES)
	mos_printf("Scheduler Runing!\r\n");
#endif

#if (MOS_CONFIG_USE_SHELL == YES)
	mos_printf("CMD > ");
#endif	
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
 * @brief ��������
 */
void mos_task_suspend(mos_tcb_t * to_suspend_task)
{
    mos_base_t temp;

    /* �����ٽ��� */
    temp = mos_port_entry_critical_temp();

    /* ���Ҫ���������������̬ */
    if (to_suspend_task->task_state == TASK_RUN)
    {
        mos_task_insert_suspend_list(to_suspend_task);
        mos_port_task_scheduler();
    }
    /* ���Ҫ���������������״̬ */
    else
    {
        mos_task_insert_suspend_list(to_suspend_task);
    }

    /* �˳��ٽ��� */
    mos_port_exit_critical_temp(temp);
}

/**
 * @brief �ָ�����
 */
void mos_task_resume(mos_tcb_t * to_resume_task)
{
    mos_base_t temp;

    /* �����ٽ��� */
    temp = mos_port_entry_critical_temp();

    mos_list_node_delete(&(to_resume_task->task_list));
    mos_task_insert_ready_table_list( to_resume_task);

    /* �˳��ٽ��� */
    mos_port_exit_critical_temp(temp);

    /* ������� */
    mos_task_scheduler();

}

/**
 * @brief ������ʱ
 */
void mos_task_delay(const mos_uint32_t tick)
{
    /* ʧ���ж� */
    mos_port_entry_critical();

    /* ��������뵽��ʱ�б� */
    mos_tick_delay_process(g_cur_task_tcb, tick);

    /* ʹ���ж� */
    mos_port_exit_critical();

    /* �����л� */
    mos_task_scheduler();
}

/**
 * @brief  ��ȡ��ǰ������ƿ�
 *
 * @return ��ǰ������ƿ�
 */
mos_tcb_t * mos_task_get_cur_tcb(void)
{
    return g_cur_task_tcb;
}

/**
 * @brief ����������뵽�����б�
 */
void mos_task_insert_suspend_list(mos_tcb_t * to_suspend_task)
{
    /* ���Ҫ���������������̬ */
    if (to_suspend_task->task_state == TASK_RUN)
    {
        mos_task_remove_ready_table_list(to_suspend_task);
        to_suspend_task->task_state = TASK_SUSPEND;
        mos_list_head_insert(&g_mos_task_suspend_list, &to_suspend_task->task_list);
    }
    /* ���Ҫ����������Ǿ���̬ */
    else if (to_suspend_task->task_state == TASK_READY)
    {
        mos_task_remove_ready_table_list(to_suspend_task);
        to_suspend_task->task_state = TASK_SUSPEND;
        mos_list_head_insert(&g_mos_task_suspend_list, &to_suspend_task->task_list);
    }
    /* ���Ҫ���������������̬ */
    else if (to_suspend_task->task_state == TASK_BLOCK)
    {
        /* ���������������ʱ���б����Ƴ� */
        mos_task_remove_delay_list(to_suspend_task);
        to_suspend_task->task_state = TASK_SUSPEND;
        mos_list_head_insert(&g_mos_task_suspend_list, &to_suspend_task->task_list);
    }

#if (MOS_CONFIG_USE_IPC == YES)

    /* ɾ�������ź���������� */
    if (!mos_list_is_empty(&to_suspend_task->task_ipc_list))
    {
        mos_list_node_delete(&to_suspend_task->task_ipc_list);
    }

#endif
}

/**
 * @brief ����������뵽�����б�
 */
void mos_task_insert_ready_table_list(mos_tcb_t *to_ready_task)
{
    /* ����̬ */
    if(to_ready_task->task_state == TASK_SUSPEND)
    {
        mos_list_node_delete(&(to_ready_task->task_list));
    }
    /* ����̬ */
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

    /* �޸�����״̬Ϊ����̬ */
    to_ready_task->task_state = TASK_READY;
    /* �������������б� */
    mos_list_head_insert(&(g_mos_task_list_ready_table[to_ready_task->task_priority]), &(to_ready_task->task_list));
    /* ���ö�Ӧλ���ȼ���־ */
    g_mos_task_priority_flag_32bit |= (1UL << (31UL - to_ready_task->task_priority));
}


/**
 * @brief ��������Ӿ����б�ɾ��
 */
void mos_task_remove_ready_table_list(mos_tcb_t *mos_tcb)
{
    /* ������Ӿ����б����Ƴ� */
    mos_list_node_delete(&(mos_tcb->task_list));

    /* �����������ȼ�λͼ�ж�Ӧ��λ��� */
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
 * @brief  �������������ʱ�б�
 * @param  ������ʱ�б�
 * @param  ������������
 * @param  ϵͳ���Ѹ�����ʱ��
 */
void mos_task_insert_delay_list(mos_list_t *delay_list, mos_list_t *task_node, mos_tick_t mos_task_wake_tick)
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

    /* �����б� */
    mos_list_head_insert(iterator, task_node);
}

/**
 * @brief  �����������ʱ�б�ɾ��
 * @param  ������ʱ�б�
 * @param  ������������
 */
void mos_task_remove_delay_list(mos_tcb_t * to_remove_delay_tcb)
{
    mos_tcb_t * delay_next_task_tcb = NULL;

    /* ���������ʱ�б��Ƴ��������ȴ�״̬ */
    mos_list_node_delete(&to_remove_delay_tcb->task_list);
    /* ����������ƿ��task_tick_wake */
    to_remove_delay_tcb->task_tick_wake = 0L;

    /* ��ʱ�б�Ϊ�գ�����g_mos_task_next_task_unblock_tickΪ���ܵ����ֵ */
    if(mos_list_is_empty(g_mos_task_delay_list))
    {
        g_mos_task_next_task_unblock_tick = MOS_MAX_DELAY;
    }
    /* ��ʱ�б�Ϊ�� */
    else
    {
        delay_next_task_tcb = MOS_LIST_ENTRY(g_mos_task_delay_list->next, mos_tcb_t, task_list);
        g_mos_task_next_task_unblock_tick = delay_next_task_tcb->task_tick_wake;
    }
}

/**
 * @brief �ر��������
 */
void mos_task_scheduler_close(void)
{
    mos_base_t temp;

    /* �����ٽ��� */
    temp = mos_port_entry_critical_temp();

    /* g_scheduler_flag > 0 ��ʾ�ر���������� */
    g_scheduler_flag++;

    /* �˳��ٽ��� */
    mos_port_exit_critical_temp(temp);
}

/**
 * @brief �����������
 */
void mos_task_scheduler_resume(void)
{
    mos_tcb_t *mos_tcb;
    mos_ubase_t	top_priority;
    mos_base_t temp;

    /* �����ٽ��� */
    temp = mos_port_entry_critical_temp();

    /* g_scheduler_flag > 0 ��ʾ�ر���������� */
    --g_scheduler_flag;

    /* ����ر�����������ڼ�ָ��˸����ȼ����񣬽���һ��������� */
    mos_get_highest_priority(top_priority, g_mos_task_priority_flag_32bit);
    mos_tcb = MOS_LIST_ENTRY(g_mos_task_list_ready_table[top_priority].next, mos_tcb_t, task_list);

    if ((mos_tcb->task_priority < g_cur_task_tcb->task_priority))
    {
        mos_port_task_scheduler();
    }

    /* �˳��ٽ��� */
    mos_port_exit_critical_temp(temp);
}

