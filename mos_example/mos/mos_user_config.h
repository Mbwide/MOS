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

/* 配置字节对齐值 */
#define MOS_CONFIG_ALIGN_SIZE				(4)

/* 配置任务名长度 */
#define MOS_CONFIG_TASK_NAMELEN				(16)

/* 配置任务最大优先级 */
#define MOS_CONFIG_TASK_PRIORITY_MAX		(16)

/* 配置空闲任务栈大小 */
#define MOS_CONFIG_TASK_IDLE_STACK_SIZE		(512)

/* 配置系统时钟周期 */
#define MOS_CONFIG_TICK_PER_SECOND	    	(100)

/* 是否使用时间片轮转调度 */
#define MOS_CONFIG_USE_TIME_SLICING		    (NO)

/* 配置动态内存大小，单位byte */
#define MOS_CONFIG_HEAP_SIZE	    		(20 * 1024)

/* 使用动态内存管理 */
#define MOS_CONFIG_USE_DYNAMIC_HEAP		    (YES)

/* 使用信号量 */
#define MOS_CONFIG_USE_IPC		            (YES)

/* 使用同步信号量 */
#define MOS_CONFIG_USE_SYNC_IPC		        (YES)

#endif /* __MOS_USER_CONFIG_H__ */
