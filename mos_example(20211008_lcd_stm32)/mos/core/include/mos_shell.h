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
  * @file    mos_shell.h
  * @version V1.0.0
  * @date    2021-10-06
  * @brief   shell
  ******************************************************************************
  * @note
  *          2021-10-06 Mbwide:≥ı º∞Ê±æ
  ******************************************************************************
  */
#ifndef _MOS_SHELL_H
#define _MOS_SHELL_H

#include "mos_typedef.h"
/* Public Fun-----------------------------------------------------------------*/
void mos_shell_process(mos_uint8_t c);
mos_err_t mos_shell_init(void);
#endif /* _MOS_SHELL_H */

