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
  * @file    mos_ipc.h
  * @version V1.0.0
  * @date    2021-09-15
  * @brief   任务间通信
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */
#ifndef _MOS_IPC_H
#define _MOS_IPC_H

#include "mos_typedef.h"
#include "mos_list.h"

/* 信号量类型标志 */
typedef enum ipc_flag
{
	INIT_IPC,
	SYNC_IPC,
	COUNT_IPC,
	MUTEX_IPC,	
} ipc_flag_e;

/* 信号量头属性 */
typedef struct mos_ipc
{
    mos_ubase_t flag;
	mos_list_t block_list;
} mos_ipc_t;

/* 同步信号量控制块 */
typedef struct mos_sync
{
	mos_ipc_t sync_ipc;
	mos_ubase_t value;	
} mos_sync_t;


/* Public Fun-----------------------------------------------------------------*/
/* 同步信号量创建 */
mos_err_t mos_ipc_sync_creat(mos_sync_t *sync_ipc);
/* 同步信号量获取 */
mos_err_t mos_ipc_sync_take(mos_sync_t *sync_ipc, mos_uint32_t to_block_tick);
/* 同步信号量施放 */
mos_err_t mos_ipc_sync_give(mos_sync_t *sync_ipc);


#endif /* _MOS_IPC_H */

