# MOS

### 1 简介

一款开源的实时操作系统MicroOS，支持 ARM 系列 M3 内核架构和 M4 内核架构（暂不支持FPU），目前支持静态任务创建，多优先级，支持时间片轮转调度，正在开发中。

### 2 源码目录

1. core 包括内核文件
   - include：头文件
     - mos_init.h
     - mos_list.h 链表实现
     - mos_misc.h
     - mos_sys.h
     - mos_task.h
     - mos_tick.h
   - mos_init.c       内核初始化
   - mos_misc.c    内核杂项（字符处理等）
   - mos_sys.c       系统支持(临界区等)
   - mos_task.c     任务与调度（任务创建，延时，调度器开启等）
   - mos_tick.c      定时器
2. ports  系统板级支持包接口
   - mos_hw.c 	系统硬件支持（栈初始化等）
   - mos_hw.h 
   - mos_port_asm.s 系统软件支持，汇编实现（PendSV中断服务函数，SVC中断服务函数）
   - mos_port.c  系统软件支持（滴答定时间配置，滴答定时器中断等）
   - mos_port.h
   - mos_typedef.c  数据类型重定义
3. mos_user_config.h  用户配置文件

### 3 移植过程（STM32F103标准库）

#### 3.1 静态方法

##### 3.1.1 修改接口文件

1. 屏蔽标准库中断服务函数
   - SVC_Handler
   - PendSV_Handler
   - SysTick_Handler
2. 配置mos_user_config.h 
   - 设置CPU频率
   - 设置MOS时钟周期
3. 引入头文件 mos_init.h mos_task.h mos_port.h

##### 3.1.2任务创建

1. 设置系统中断优先级分组
2. 硬件初始化
3. 进入临界区
4. 内核初始化
5. 任务创建：需要以下参数
   - 任务控制块指针
   - 任务入口函数
   - 任务名称
   - 任务优先级
   - 任务入口函数形参
   - 任务堆栈大小，单位为字
   - 任务栈起始地址
6. 退出临界区
7. 任务调度开启

