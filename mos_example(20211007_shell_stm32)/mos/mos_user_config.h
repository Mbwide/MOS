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

#include "stm32f10x.h"

/* �����ֽڶ���ֵ */
#define MOS_CONFIG_ALIGN_SIZE               (4)
/* �������������� */
#define MOS_CONFIG_TASK_NAMELEN             (16)
/* ��������������ȼ� */
#define MOS_CONFIG_TASK_PRIORITY_MAX        (16)
/* ���ÿ�������ջ��С */
#define MOS_CONFIG_TASK_IDLE_STACK_SIZE     (512)
/* ����CPUƵ�� */
#define MOS_CONFIG_CPU_FREQUENCY            (SystemCoreClock)
/* ����ϵͳʱ������ */
#define MOS_CONFIG_TICK_PER_SECOND          (1000)

/* ʹ��ʱ��Ƭ��ת���� */
#define MOS_CONFIG_USE_TIME_SLICING	        (YES)
/* ʹ�ö�̬�ڴ���� */
#define MOS_CONFIG_USE_DYNAMIC_HEAP         (YES)
/* ���ö�̬�ڴ��С����λbyte */
#define MOS_CONFIG_HEAP_SIZE                (20 * 1024)

/* ʹ���ź��� */
#define MOS_CONFIG_USE_IPC                  (YES)
/* ʹ��ͬ���ź��� */
#define MOS_CONFIG_USE_SYNC_IPC             (YES)
/* ʹ�û����ź��� */
#define MOS_CONFIG_USE_MUTEX_IPC            (YES)

/* ʹ��shell�ӿ� */
#define MOS_CONFIG_USE_SHELL                (YES)
/* ʹ���ں˵�����Ϣ��ʾ */
#define MOS_CONFIG_USE_DEBUG_PRINTF	        (YES)

#if (MOS_CONFIG_USE_SHELL == YES || MOS_CONFIG_USE_DEBUG_PRINTF == YES)
/* shellͨѶ�ӿڶ��� */
#define MOS_SHELL_DEBUG_PORT                (USART1)
/* shell��������������� */
#define MOS_CONFIG_SHELL_CMD_LEN            (5)
/* �ں˵�����Ϣ���ͻ����С */
#define MOS_CONFIG_DEBUG_TXBUF_SIZE         (128)
/* shell�ӿڽ��ջ����С */
#define MOS_CONFIG_SHELL_RXBUF_SIZE         (64)
#endif

#endif /* __MOS_USER_CONFIG_H__ */
