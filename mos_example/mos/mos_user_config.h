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

#ifndef _MOS_USER_CONFIG_H__
#define _MOS_USER_CONFIG_H__

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

/* ���ö�̬�ڴ��С����λbyte */
#define MOS_CONFIG_HEAP_SIZE	    		(20 * 1024)

/* �Ƿ�ʹ��ʱ��Ƭ��ת���� */
#define MOS_CONFIG_USE_TIME_SLICING		    (NO)

/* ʹ�ö�̬�ڴ���� */
#define MOS_CONFIG_USE_DYNAMIC_HEAP		    (YES)

#endif /* __MOS_USER_CONFIG_H__ */
