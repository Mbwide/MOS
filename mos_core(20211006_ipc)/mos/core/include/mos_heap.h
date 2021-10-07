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
  * @file    mos_heap.h
  * @version V1.0.0
  * @date    2021-09-10
  * @brief   动态内存管理
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */
#ifndef _MOS_HEAP_H
#define _MOS_HEAP_H

#include "mos_typedef.h"

/* Public Fun-----------------------------------------------------------------*/
/* 内存池初始化 */
void mos_heap_init(void);
/* 内存申请 */
void *mos_malloc(mos_ubase_t need_size);
/* 内存施放 */
void mos_free(void *mem_add);

#endif /* _MOS_HEAP_H */

