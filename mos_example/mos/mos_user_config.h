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

#ifndef __MOS_USER_CONFIG_H__
#define __MOS_USER_CONFIG_H__

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

#endif /* __MOS_USER_CONFIG_H__ */
