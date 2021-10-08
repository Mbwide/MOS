/**
  ******************************************************************************
  * @file    mos_user_config.h
  * @version
  * @date
  * @brief   用户配置文件
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#ifndef _MOS_USER_CONFIG_H__
#define _MOS_USER_CONFIG_H__

#include "stm32f10x.h"

/* 配置字节对齐值 */
#define MOS_CONFIG_ALIGN_SIZE               (4)
/* 配置任务名长度 */
#define MOS_CONFIG_TASK_NAMELEN             (16)
/* 配置任务最大优先级 */
#define MOS_CONFIG_TASK_PRIORITY_MAX        (16)
/* 配置空闲任务栈大小 */
#define MOS_CONFIG_TASK_IDLE_STACK_SIZE     (512)
/* 配置CPU频率 */
#define MOS_CONFIG_CPU_FREQUENCY            (SystemCoreClock)
/* 配置系统时钟周期 */
#define MOS_CONFIG_TICK_PER_SECOND          (1000)

/* 使用时间片轮转调度 */
#define MOS_CONFIG_USE_TIME_SLICING	        (YES)
/* 使用动态内存管理 */
#define MOS_CONFIG_USE_DYNAMIC_HEAP         (YES)
/* 配置动态内存大小，单位byte */
#define MOS_CONFIG_HEAP_SIZE                (20 * 1024)

/* 使用信号量 */
#define MOS_CONFIG_USE_IPC                  (YES)
/* 使用同步信号量 */
#define MOS_CONFIG_USE_SYNC_IPC             (YES)
/* 使用互斥信号量 */
#define MOS_CONFIG_USE_MUTEX_IPC            (YES)

/* 使用shell接口 */
#define MOS_CONFIG_USE_SHELL                (YES)
/* 使用内核调试信息显示 */
#define MOS_CONFIG_USE_DEBUG_PRINTF	        (YES)

#if (MOS_CONFIG_USE_SHELL == YES || MOS_CONFIG_USE_DEBUG_PRINTF == YES)
/* shell通讯接口定义 */
#define MOS_SHELL_DEBUG_PORT                (USART1)
/* shell单行命令最大数量 */
#define MOS_CONFIG_SHELL_CMD_LEN            (5)
/* 内核调试信息发送缓存大小 */
#define MOS_CONFIG_DEBUG_TXBUF_SIZE         (128)
/* shell接口接收缓存大小 */
#define MOS_CONFIG_SHELL_RXBUF_SIZE         (64)
#endif

#endif /* __MOS_USER_CONFIG_H__ */
