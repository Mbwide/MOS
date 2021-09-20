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
  * @file    mos_heap.c
  * @version V1.0.0
  * @date    2021-09-10
  * @brief   ��̬�ڴ����
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#include "mos_user_config.h"
#include "mos_heap.h"
#include "mos_port.h"

/* �ڴ����Сֵ */
#define BLOCK_MIN_SIZE ((mos_ubase_t )(sizeof(mos_heap_head_t) << 1))

/* �ڴ������ͷ��С */
#define HEAD_T_SIZE sizeof(mos_heap_head_t)

/* �ڴ������ͷ */
typedef struct mos_heap_head
{
	mos_ubase_t used;
	mos_ubase_t block_size;
	struct mos_heap_head * next;
} mos_heap_head_t;


/* Private -------------------------------------------------------------------*/
/* �ڴ�� */
static mos_uint8_t heap[MOS_CONFIG_HEAP_SIZE];
/* �ڴ����ʼ����ͷ���������ڴ�أ���heap_head_start.nextָ���һ��δʹ�õ��ڴ������ͷ*/
static mos_heap_head_t heap_head_start;
/* �ڴ���������βָ�루�����ڴ�أ�*/
static mos_heap_head_t *heap_head_end;
/* �ڴ��ܴ�С */
static mos_ubase_t heap_total_size;
/* ʣ���ڴ��С */
static mos_ubase_t heap_free_size;

/* �Ż��ڴ�� */
static void block_insert_free_list(mos_heap_head_t *new_block);

/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief  �ڴ�س�ʼ��
 */
void mos_heap_init(void)
{
	/* ָ���ڴ�ص�һ���ڴ������ͷ�������ڴ�أ� */
	mos_heap_head_t * first_block;
	
	/* ���ڼ����ڴ濪ʼ�ͽ�����ַ��ֵ */
	mos_ubase_t add_begin;
	mos_ubase_t add_end;
	
	/* �ڴ���� */
	add_begin = MOS_ALIGN_UP((mos_uint32_t)heap, MOS_CONFIG_ALIGN_SIZE);
	add_end   = MOS_ALIGN_DOWN((mos_uint32_t)(heap + MOS_CONFIG_HEAP_SIZE), MOS_CONFIG_ALIGN_SIZE);
	
	/* �ڴ��ܴ�С��ʣ���ڴ��С */
	heap_total_size = add_end - add_begin - HEAD_T_SIZE;
	heap_free_size = heap_total_size;
	
	/* ��ʼ���ڴ����ʼ����ͷ */
	heap_head_start.used = 0;
	heap_head_start.block_size = 0;
	heap_head_start.next = (mos_heap_head_t *)add_begin;
	
	/* ��ʼ���ڴ���������ͷ */
	heap_head_end = (mos_heap_head_t *)(add_end - HEAD_T_SIZE);
	heap_head_end->used = 0;
	heap_head_end->block_size = 0;
	heap_head_end->next = NULL;
	
	/* ��ʼ����һ���ڴ������ͷ */
	first_block = (mos_heap_head_t *)add_begin;
	first_block->used = 0;
	first_block->block_size = heap_free_size;
	first_block->next = heap_head_end;
}

/**
 * @brief  �����ڴ�
 * @param  ������ڴ��С����λΪbyte
 *
 * @return �������뵽���ڴ��׵�ַ��������ʧ�ܣ�����NULL
 */
void *mos_malloc(mos_ubase_t need_size)
{
	void *return_ptr = NULL;
	mos_heap_head_t *pre_block;
	mos_heap_head_t *cur_block;
	mos_heap_head_t *new_block;
	
	if (need_size == 0) return NULL;
	
	/* �����С��Ҫ�ֽڶ��� */
	need_size = MOS_ALIGN_UP(need_size, MOS_CONFIG_ALIGN_SIZE);
	/* �����С����ʣ���ڴ��С����NULL */
	if (need_size > heap_free_size) return NULL;
	/* �����С+����ͷ��С */
	need_size += HEAD_T_SIZE;
	/* �ڴ����Сֵ */
	if (need_size < BLOCK_MIN_SIZE) need_size = BLOCK_MIN_SIZE;
	
	/* �����ٽ��� */
	mos_port_entry_critical();
	
	pre_block = &heap_head_start;
	cur_block = heap_head_start.next;
	
	/* Ѱ�ҷ��ϴ�С���ڴ�� */
	while((cur_block->block_size < need_size) && cur_block->next != NULL)
	{
		pre_block = cur_block;
		cur_block = cur_block->next;
	}
	
	/* �ҵ��� */
	if (cur_block != heap_head_end)
	{
		/* ���ص�ַҪȥ���ڴ������ͷ */
		return_ptr = (void *)((mos_uint8_t *)pre_block->next + HEAD_T_SIZE);
		/* ��Ҫ���ص��ڴ������ͷ�ӿ����ڴ���ڴ������ͷ����ɾ��*/
		pre_block->next = cur_block->next;
		
		/* �ڴ��̫���е�����ķŻ��ڴ�� */
		if ((cur_block->block_size - need_size) > BLOCK_MIN_SIZE)
		{
			/* ��ȡҪ�Ż��ڴ�ص��ڴ������ͷ */
			new_block = (mos_heap_head_t *)((mos_ubase_t)(cur_block) + need_size);
			new_block->used = 0;
			new_block->block_size = cur_block->block_size - need_size;
            
			/* �Ż��ڴ�� */
			block_insert_free_list(new_block);
			
			/* �޸�Ҫ���ص��ڴ������ͷ */
			cur_block->used = 1;
			cur_block->block_size = need_size;
		
		}
	}
	
	/* �޸�ʣ���ڴ��С */
	heap_free_size -= cur_block->block_size;
	
	/* �˳��ٽ��� */
	mos_port_exit_critical();
	
	return return_ptr;
}

/**
 * @brief  �ڴ�ʩ��
 * @param  Ҫʩ�ŵ��ڴ��׵�ַ
 */
void mos_free(void *mem_add)
{
	mos_heap_head_t *cur_block;
	mos_uint8_t * p_block;
	
	p_block = (mos_uint8_t *)mem_add;
	
	if (mem_add != NULL) 
	{
		p_block -= HEAD_T_SIZE;
		cur_block = (mos_heap_head_t *)p_block;
		
		if (cur_block->used == 1) 
		{
			cur_block->used = 0;
			
			/* �����ٽ��� */
	        mos_port_entry_critical();
			
			/* �޸�ʣ���ڴ��С */
			heap_free_size += cur_block->block_size;
			
			/* �Ż��ڴ�� */
			block_insert_free_list(cur_block);			
			
			/* �˳��ٽ��� */
	        mos_port_exit_critical();
		}
	}
}


/* Private Fun----------------------------------------------------------------*/
/**
 * @brief  �Ż��ڴ��
 * @param  Ҫ�Ż��ڴ�ص��ڴ������ͷָ��
 */
static void block_insert_free_list(mos_heap_head_t *new_block)
{
	mos_heap_head_t *cur_block;
	mos_uint8_t *p_block;
	
	cur_block = &heap_head_start;
	
	/* ���ݵ�ַѰ�Ҳ���� */
	while (cur_block->next < new_block)
	{
		cur_block = cur_block->next;
	}
	
	/* ��ǰ�ϲ� */
	p_block = (mos_uint8_t *)cur_block;
    if ((p_block + cur_block->block_size) == (mos_uint8_t *)new_block)
	{
		/* �½ڵ��Ϊ�ϲ���Ľ�㣬���޸ĸýڵ��С */
		cur_block->block_size += new_block->block_size;
		new_block = cur_block;
	}
	
	/* ���ϲ� */
	p_block = (mos_uint8_t *)new_block;
	if((p_block + new_block->block_size ) == (mos_uint8_t *)cur_block->next)
	{
		/* ������Ժϲ����ҵ�ǰ�ڵ����һ���ڵ㲻��β�� */
	    if(cur_block->next != heap_head_end)
		{
			new_block->block_size += cur_block->next->block_size;
			new_block->next = cur_block->next->next;
		}
		/* ��һ���ڵ���β�����½ڵ�ָ��β�ڵ� */
		else
		{
			new_block->next = heap_head_end;
		}
	}
	else
	{
		/* û�����ϲ����½ڵ�������ӵ���ǰ�ڵ����һ�ڵ�*/
		new_block->next = cur_block->next;
	}
	
	/* û����ǰ�ϲ����½ڵ���ǰ���ӵ���ǰ�ڵ� */
	if(cur_block != new_block)
	{
		cur_block->next = new_block;
	}
}


