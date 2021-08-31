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
  * @version V1.0.0
  * @date    2021-08-20
  * @brief   ���������
  ******************************************************************************
  * @note
  *
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

/* ������ƿ鶨�� */
typedef struct mos_task_control_block
{

    volatile void	*stack_pointer;							/* ����ջָ��sp */
    char			task_name[MOS_CONFIG_TASK_NAMELEN];		/* �������� */
    task_entry_fun	task_entry;								/* ������ں��� */
    mos_uint32_t	task_param;								/* �����β� */

    mos_uint32_t	stack_top;								/* ����ջ����ַ,�͵�ַ */
    mos_uint32_t	stack_size;								/* ����ջ��С */

    mos_uint16_t	task_priority;							/* �������ȼ� */
    mos_uint16_t	task_state;								/* ����״̬*/

    mos_uint32_t	task_tick_wake;							/* ������ʱ��*/
    mos_uint32_t	task_tick_wake_over;					/* ������ʱ�������־λ*/
    mos_list_t 		task_list;								/* ������������*/

} mos_tcb_t;


/* Public Fun-----------------------------------------------------------------*/
/* ���񴴽� */
mos_err_t mos_task_create(	mos_tcb_t *  const task_tcb,	/* ������ƿ�ָ�� */
                            task_entry_fun 	   task_code,	/* ������� */
                            const char * const task_name,	/* �������ƣ��ַ�����ʽ */
                            const mos_uint8_t  task_pri,	/* �������ȼ� */
                            const mos_uint32_t parameter,	/* �����β� */
                            const mos_uint32_t stack_size,	/* ����ջ��С����λΪ�� */
                            const mos_uint32_t stack_start);/* ����ջ��ʼ��ַ */
/* �����������ʼ�� */
mos_err_t mos_task_scheduler_init(void);
/* ��һ�ο���������� */
mos_err_t mos_task_scheduler_start(void);
/* ������� */
mos_err_t mos_task_scheduler(void);
/* ����ѡ��,�����µ�ǰ�������� */
void mos_task_switch_context(void);

/* ������ʱ */
void mos_task_delay(const mos_uint32_t tick);

/* �������������б� */
void mos_task_insert_ready_table_list(mos_list_t *task_list_ready_table, mos_tcb_t *mos_tcb);
/* ������Ӿ����б�ɾ�� */
void mos_task_remove_ready_table_list(mos_list_t *task_list_ready_table, mos_tcb_t *mos_tcb);

/* ϵͳ��ǰʱ���������������� */
mos_bool_t mos_task_tickcount_increase(void);

#endif /* _MOS_TASK_H */

