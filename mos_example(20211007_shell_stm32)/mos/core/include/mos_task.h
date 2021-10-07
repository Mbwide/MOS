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
  * @brief   ���������
  ******************************************************************************
  * @note
  *          2021-08-20 Mbwide:��ʼ�汾
  *          2021-10-04 Mbwide:�ṹ�Ż�
  *          2021-10-07 Mbwide:���shell����
  ******************************************************************************
  */

#ifndef _MOS_TASK_H
#define _MOS_TASK_H

#include "mos_typedef.h"
#include "mos_list.h"
#include "mos_misc.h"
#include "mos_user_config.h"

/* ������ں������� */
typedef void (*task_entry_fun)(void);

/* ����״̬ */
typedef enum TASK_STATE
{
    TASK_RUN = 1, /* ����̬ */
    TASK_READY,   /* ����̬ */
    TASK_BLOCK,   /* ����̬ */
    TASK_SUSPEND  /* ����̬ */
} TASK_STATE_T;

/* ������ƿ鶨�� */
typedef struct mos_task_control_block
{
    volatile void   *stack_pointer;                       /* ����ջָ��sp */
    char            task_name[MOS_CONFIG_TASK_NAMELEN];   /* �������� */
    task_entry_fun  task_entry;                           /* ������ں��� */
    mos_uint32_t    task_param;                           /* �����β� */

    mos_uint32_t    stack_top;                            /* ����ջ����ַ,�͵�ַ */
    mos_uint32_t    stack_size;                           /* ����ջ��С */

    mos_uint16_t    task_priority;                        /* �������ȼ� */
    mos_uint16_t    task_base_priority;                   /* ��������ȼ� */
    mos_uint16_t    task_state;                           /* ����״̬*/

    mos_uint32_t    task_tick_wake;                       /* ������ʱ��*/
    mos_uint32_t    task_tick_wake_over;                  /* ������ʱ�������־λ*/

    mos_list_t      task_list;                            /* ����������������*/
#if (MOS_CONFIG_USE_SHELL == YES)
    mos_list_t      task_shell_list;                      /* ��������shell�б�*/
#endif
    mos_list_t      task_ipc_list;                        /* �������������ͨ������*/
} mos_tcb_t;

/* Public Fun-----------------------------------------------------------------*/
#if MOS_CONFIG_USE_DYNAMIC_HEAP
/* ��̬���񴴽� */
mos_err_t mos_task_create( mos_tcb_t *  const task_tcb,    /* ������ƿ�ָ�� */
                           task_entry_fun 	  task_code,   /* ������� */
                           const char * const task_name,   /* �������ƣ��ַ�����ʽ */
                           const mos_uint8_t  task_pri,    /* �������ȼ� */
                           const mos_uint32_t stack_size); /* ����ջ��С����λΪ�� */
#else
/* ��̬���񴴽� */
mos_err_t mos_task_create(mos_tcb_t *  const task_tcb,	    /* ������ƿ�ָ�� */
                          task_entry_fun 	   task_code,	/* ������� */
                          //const char * const task_name,	/* �������ƣ��ַ�����ʽ */
                          const mos_uint8_t  task_pri,	    /* �������ȼ� */
                          //const mos_uint32_t parameter,	/* �����β� */
                          const mos_uint32_t stack_size,	/* ����ջ��С����λΪ�� */
                          const mos_uint32_t stack_start);  /* ����ջ��ʼ��ַ */
#endif

/* �����������ʼ�� */
mos_err_t mos_task_scheduler_init(void);
/* ��һ�ο���������� */
mos_err_t mos_task_scheduler_start(void);
/* ������� */
mos_err_t mos_task_scheduler(void);
/* ����ѡ��,�����µ�ǰ�������� */
void mos_task_switch_context(void);

/* ��������� */
void mos_task_suspend(mos_tcb_t * to_suspend_task);
/* ������ָ� */
void mos_task_resume(mos_tcb_t * to_suspend_task);
/* ������ʱ */
void mos_task_delay(const mos_uint32_t tick);
/* ��ȡ��ǰ������ƿ� */
mos_tcb_t * mos_task_get_cur_tcb(void);

/* ����������뵽�����б� */
void mos_task_insert_suspend_list(mos_tcb_t * to_suspend_task);
/* ����������뵽�����б� */
void mos_task_insert_ready_table_list(mos_tcb_t *to_ready_task);
/* ��������Ӿ����б�ɾ�� */
void mos_task_remove_ready_table_list(mos_tcb_t *mos_tcb);
/* �������������ʱ�б� */
void mos_task_insert_delay_list(mos_list_t *delay_list, mos_list_t *task_node, mos_tick_t mos_task_wake_tick);
/* �����������ʱ�б�ɾ�� */
void mos_task_remove_delay_list(mos_tcb_t * to_remove_delay_tcb);

#endif /* _MOS_TASK_H */

