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
  * @file    mos_misc.h
  * @version V1.0.0
  * @date    2021-08-01
  * @brief   内核杂项
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */
#ifndef _MOS_MISC_H
#define _MOS_MISC_H

#include "mos_typedef.h"

/* Public Fun-----------------------------------------------------------------*/
/* 将某一块内存中的内容全部设置为指定值 */
void *mos_memset(void *str, mos_int32_t c, mos_int32_t len);

/* 从源内存地址的起始位置开始拷贝若干个字节到目标内存地址中 */
void *mos_memcpy(void *dst, const void *src, mos_int32_t len);

/* 将指定长度的字符串复制到字符数组中 */
char *mos_strncpy(char *dst, const char *src, mos_int32_t len);

/* 返回字符串中首次出现子串的地址 */
char *mos_strstr(const char *str1, const char *str2);

#endif
