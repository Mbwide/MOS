/**
  ******************************************************************************
  * @file    mos_user_config.h
  * @version
  * @date
  * @brief   �û������ļ�
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */
#include "mos_typedef.h"

#ifndef __MOS_USER_CONFIG_H__
#define __MOS_USER_CONFIG_H__

/* �����ֽڶ���ֵ */
#define MOS_CONFIG_ALIGN_SIZE				(4)

/* �������������� */
#define MOS_CONFIG_TASK_NAMELEN				(16)

/* ��������������ȼ� */
#define MOS_CONFIG_TASK_PRIORITY_MAX		(16)

/* ���ÿ�������ջ��С */
#define MOS_CONFIG_TASK_IDLE_STACK_SIZE		(512)

/* ����ϵͳʱ������ */
#define MOS_CONFIG_TICK_PER_SECOND	    	(100)

/* �Ƿ�ʹ��ʱ��Ƭ��ת���� */
#define MOS_CONFIG_USE_TIME_SLICING		    (NO)

#endif /* __MOS_USER_CONFIG_H__ */
