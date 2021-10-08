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
  * @file    mos_shell.c
  * @version V1.0.0
  * @date    2021-10-07
  * @brief   shell
  ******************************************************************************
  * @note
  *          2021-10-07 Mbwide:初始版本
  ******************************************************************************
  */

#include "mos_port.h"
#include "mos_shell.h"
#include "mos_misc.h"
#include "mos_heap.h"
#include "mos_task.h"
#include "mos_list.h"
#include "mos_ipc.h"
#include "mos_tick.h"
#include "mos_user_config.h"

/* Public --------------------------------------------------------------------*/
#if (MOS_CONFIG_USE_SHELL == YES)
/* 任务列表 */
mos_list_t g_mos_shell_task_list;

#if (MOS_CONFIG_USE_IPC == (YES))
/* 信号量列表 */
mos_list_t g_mos_shell_ipc_list;
#endif


/* Private -------------------------------------------------------------------*/
/* 命令接收缓冲,分割前 */
static char mos_shell_rx_buf[MOS_CONFIG_SHELL_RXBUF_SIZE];
/* 分割后的命令 */
static char **mos_shell_cmd_buf;

/* 接收状态标记 */
static mos_uint16_t SHELL_RX_STA = 0;   


/* Private Fun----------------------------------------------------------------*/
/**
 * @brief  shell命令，ls
 * @param  命令
 */
static void mos_shell_ls_cmd(char *argv	)
{
    mos_printf("CPU CORE      : %s\n", CPU_DATA);
    mos_printf("CPU FREQUENCY : %dMHZ\n", MOS_CONFIG_CPU_FREQUENCY / 1000000U);

    mos_printf("MOS Version   : %d.%d.%d\n",
               MOS_VERSION, MOS_SUBVERSION, MOS_REVISION);
    mos_printf("MOS PER_SECOND: %dHZ\n", MOS_CONFIG_TICK_PER_SECOND);
    mos_printf("\n");
    return;
}

/**
 * @brief  shell命令，help
 * @param  命令
 */
static void mos_shell_help_cmd(char *argv)
{
    mos_printf(" help: CMD Help Information\n");
    mos_printf(" ls  : Hardware And OS Information\n");
    mos_printf("       -task: OS TASK  Information\n");

#if (MOS_CONFIG_USE_IPC == (YES))
    mos_printf("       -ipc : OS IPC   Information\n");
#endif
    mos_printf("       -heap: OS HEAP  Information\n");
    mos_printf("       -time: OS TIME  Information\n");
    mos_printf("\n");
    return;
}

/**
 * @brief  shell命令，heap
 * @param  命令
 */
static void mos_shell_heap_cmd(char *argv)
{
    mos_printf("Heap Total Size: %6d Byte\n", mos_get_heap_total_size());
    mos_printf("Heap Free  Size: %6d Byte\n", mos_get_heap_free_size());
    mos_printf("\n");
    return;
}

/**
 * @brief  shell命令，time
 * @param  命令
 */
static void mos_shell_time_cmd(char *argv)
{
    mos_tick_t cur_time;
    mos_tick_t hh;
    mos_tick_t mm;
    mos_tick_t ss;
    cur_time = mos_tick_get_cur_tick_count();
    cur_time = cur_time / MOS_CONFIG_TICK_PER_SECOND;
    hh = cur_time / 3600;
    cur_time = cur_time % 3600;
    mm = cur_time / 60;
    ss = cur_time % 60;

    mos_printf("System Clock: hh mm ss\n");
    mos_printf("              %02d:%02d:%02d\n", hh, mm, ss);
    mos_printf("\n");
    return;
}

#if (MOS_CONFIG_USE_IPC == (YES))
/**
 * @brief  shell命令，ipc
 * @param  命令
 */
static void mos_shell_ipc_cmd(char *argv)
{
    mos_ipc_t *mos_ipc;
    mos_list_t *ipc_iterator;

    mos_tcb_t *mos_tcb;
    mos_list_t *task_iterator;

    if (!mos_strcmp(argv, "ipc"))
    {
        ipc_iterator = &g_mos_shell_ipc_list;

        if (ipc_iterator->next != &g_mos_shell_ipc_list)
        {
            mos_printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - \r\n");

            while (ipc_iterator->next != &g_mos_shell_ipc_list)
            {
                mos_ipc = MOS_LIST_ENTRY(ipc_iterator->next, mos_ipc_t, shell_list);

                switch(mos_ipc->flag)
                {
                    case SYNC_IPC:
                        mos_printf("IPC SIGN: SYNC_IPC\n");
                        break;

                    case MUTEX_IPC:
                        mos_printf("IPC SIGN: MUTEX_IPC\n");
                        break;
                }

                task_iterator = &mos_ipc->block_list;

                if (task_iterator->next != &mos_ipc->block_list)
                {
                    while (task_iterator->next != &mos_ipc->block_list)
                    {
                        mos_tcb = MOS_LIST_ENTRY(task_iterator->next, mos_tcb_t, task_ipc_list);
                        mos_printf("IPC BLOCK TASK: %s\n", mos_tcb->task_name);
                        task_iterator = task_iterator->next;
                    }
                }
                else
                {
                    mos_printf("NO TASK BLOCK!\n");
                }

                ipc_iterator = ipc_iterator->next;
                mos_printf("\n");
            }
        }
		else 
		{
			mos_printf("NO IPC!\n");
			mos_printf("\n");
		}
    }
}
#endif

/**
 * @brief  shell命令，task
 * @param  命令
 */
static void mos_shell_task_cmd(char *argv)
{
    mos_tcb_t *mos_tcb;
    mos_list_t *iterator;

    if (!mos_strcmp(argv, "task"))
    {
        iterator = &g_mos_shell_task_list;

        if (iterator->next != &g_mos_shell_task_list)
        {
            mos_printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - \r\n");

            while (iterator->next != &g_mos_shell_task_list)
            {
                mos_tcb = MOS_LIST_ENTRY(iterator->next, mos_tcb_t, task_shell_list);
                mos_printf("%s  ", mos_tcb->task_name);
                iterator = iterator->next;
            }

            mos_printf("\n");
        }
    }

    if (!mos_strcmp(argv, "task"))
    {
        iterator = &g_mos_shell_task_list;

        if (iterator->next != &g_mos_shell_task_list)
        {
            mos_printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - \r\n");

            while (iterator->next != &g_mos_shell_task_list)
            {
                mos_tcb = MOS_LIST_ENTRY(iterator->next, mos_tcb_t, task_shell_list);
                mos_printf("TASK NAME:%s  \r\n", mos_tcb->task_name);

                switch(mos_tcb->task_state)
                {
                    case TASK_RUN:
                        mos_printf("TASK STATE: RUN\n");
                        break;

                    case TASK_READY:
                        mos_printf("TASK STATE: READY\n");
                        break;

                    case TASK_BLOCK:
                        mos_printf("TASK STATE: BLOCK\n");
                        break;

                    case TASK_SUSPEND:
                        mos_printf("TASK STATE: SUSPEND\n");
                        break;
                }

                mos_printf("TASK STACK_SIZE:%d  \n", mos_tcb->stack_size);
                mos_printf("TASK CURRENT PRIORITY:%d  \n", mos_tcb->task_priority);
                iterator = iterator->next;
                mos_printf("\r\n");
            }
        }
    }
}

/**
 * @brief  shell命令行处理
 * @param  命令个数
 * @param  具体命令
 */
static void mos_shell_cmd(int argc, char *argv[])
{
//    for (int i = 0; i < argc; i++)
//    {
//        mos_printf("argv[%d]:%s\n", i, *(argv + i));
//    }

    if (argc < 1)
    {
        mos_printf("Cmd Not Found!'\n");
        return;
    }

    if (argc == 1)
    {
        if (!mos_strcmp(argv[0], "ls"))
        {
            mos_shell_ls_cmd(argv[0]);
        }
        else if (!mos_strcmp(argv[0], "help"))
        {
            mos_shell_help_cmd(argv[0]);
        }
        else
        {
            goto cmd_not_found;
        }

    }
    else if (argc == 2)
    {
        if (!mos_strcmp(argv[0], "ls"))
        {
            if (!mos_strcmp(argv[1], "task"))
            {
                mos_shell_task_cmd(argv[1]);
            }

#if (MOS_CONFIG_USE_IPC == (YES))
            else if (!mos_strcmp(argv[1], "ipc"))
            {
                mos_shell_ipc_cmd(argv[1]);
            }
#endif

            else if (!mos_strcmp(argv[1], "heap"))
            {
                mos_shell_heap_cmd(argv[1]);
            }
            else if (!mos_strcmp(argv[1], "time"))
            {
                mos_shell_time_cmd(argv[1]);
            }
            else
            {
                goto cmd_not_found;
            }
        }
    }

    mos_printf("CMD > ");

    for(int i = 0; i < argc; i++)
    {
        mos_memset(mos_shell_cmd_buf[i], 0, MOS_CONFIG_SHELL_RXBUF_SIZE);
    }

    return ;

cmd_not_found:

    for(int i = 0; i < argc; i++)
    {
        mos_memset(mos_shell_cmd_buf[i], 0, MOS_CONFIG_SHELL_RXBUF_SIZE);
    }

    mos_printf("Cmd Not Found!\n");
    mos_printf("CMD > ");
}

/**
 * @brief  shell命令分割
 * @param  分割后的命令
 * @param  命令接收缓冲,分割前
 * @param  命令接收缓冲长度
 *
 * @return 分割后的命令数
 */
static mos_uint32_t mos_shell_spilt(char *buf[], char *src, mos_int32_t len)
{
    int num = 0;
    int dst_cur_len = 0;

    mos_printf("%s\r\n", src);

    for (int i = 0; i < len; i++)
    {
        if (src[i] != ' ')
        {
            if(src[i] == '-') continue;

            while (i < len && src[i] != ' ')
            {
                buf[num][dst_cur_len] = src[i];
                i++;
                dst_cur_len++;
            }

            buf[num][dst_cur_len] = '\0';
            num++;
            dst_cur_len = 0;
        }
    }

    return num;
}

/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief  shell接口初始化
 *
 * @return 初始化错误标志
 */
mos_err_t mos_shell_init()
{
    /* 任务列表表初始化 */
    mos_list_init(&g_mos_shell_task_list);

#if (MOS_CONFIG_USE_IPC == (YES))
    /* 信号量列表初始化 */
    mos_list_init(&g_mos_shell_ipc_list);
#endif

    mos_shell_cmd_buf = (char **)mos_malloc(sizeof(char *) * MOS_CONFIG_SHELL_CMD_LEN);

    if(mos_shell_cmd_buf == NULL) return MOS_ENULL;

    for (int i = 0 ; i < MOS_CONFIG_SHELL_CMD_LEN; i++)
    {
        mos_shell_cmd_buf[i] = (char *)mos_malloc(sizeof(char) * MOS_CONFIG_SHELL_RXBUF_SIZE);

        if(mos_shell_cmd_buf[MOS_CONFIG_SHELL_CMD_LEN] == NULL) return MOS_ENULL;
    }

    return MOS_EOK;
}

/**
 * @brief  shell接收处理接口
 *
 * @return 接收到的字符
 */
void mos_shell_process(mos_uint8_t c)
{
    int len = 0;

    /*接收未完成*/
    if((SHELL_RX_STA & 0x8000) == 0)
    {
        /*接收到了 \n*/
        if(SHELL_RX_STA & 0x4000)
        {
            if(c != 0x0a)
            {
                /*接收错误,重新开始*/
                SHELL_RX_STA = 0;
            }
            else
            {
                /*接收完成*/
                SHELL_RX_STA |= 0x8000;
                mos_shell_rx_buf[SHELL_RX_STA & 0X3FFF] = '\0';
				
				/*命令分割*/
                len = mos_shell_spilt(mos_shell_cmd_buf, mos_shell_rx_buf, SHELL_RX_STA & 0X3FFF);
				/*命令处理*/
                mos_shell_cmd(len, mos_shell_cmd_buf);
                SHELL_RX_STA = 0;
            }
        }
        /*还没收到 \r*/
        else
        {
            if(c == 0x0d)
            {
                SHELL_RX_STA |= 0x4000;
            }
            else
            {
                mos_shell_rx_buf[SHELL_RX_STA & 0X3FFF] = c ;
                SHELL_RX_STA++;

                if(SHELL_RX_STA > (MOS_CONFIG_SHELL_RXBUF_SIZE - 1))
                {
                    /*接收错误,重新开始*/
                    SHELL_RX_STA = 0;
                }
            }
        }
    }
}

#endif

