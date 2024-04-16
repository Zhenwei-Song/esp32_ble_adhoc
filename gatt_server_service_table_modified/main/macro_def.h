/*
 * @Author: Zhenwei Song zhenwei.song@qq.com
 * @Date: 2024-01-20 09:47:36
 * @LastEditors: Zhenwei Song zhenwei.song@qq.com
 * @LastEditTime: 2024-01-29 16:01:19
 * @FilePath: \esp32\esp32_ble\gatt_server_service_table_modified\main\macro_def.h
 * @Description: 仅供学习交流使用
 * Copyright (c) 2024 by Zhenwei Song, All Rights Reserved.
 */
#ifndef MACRO_DEF_H_
#define MACRO_DEF_H_

#define SELF_ROOT // 自己是root

// #define BUTTON_MY_MESSAGE // 按键发送自己的消息
#define BUTTON_BLOCK_MESSAGE // 按键发送阻塞消息

#define FILTER // 包过滤
//       #define GPIO //GPIO相关
//       #define THROUGHPUT//吞吐量测试
#define DOWN_ROUTINGTABLE // 下传路由表（链表）
#define QUEUE             // 发送、接收队列
#define BUTTON            // 按键
#define BLE_TIMER         // 定时器

#define PRINT_MY_INFO
#define PRINT_NEIGHBOR_TABLE
// #define PRINT_UP_ROUTING_TABLE
// #define PRINT_DOWN_ROUTING_TABLE
// #define PRINT_CONTROL_PACKAGES_RECEIVED
// #define PRINT_MESSAGE_PACKAGES_RECEIVED
// #define PRINT_CONTROL_PACKAGES_STATES
// #define PRINT_MASSAGE_PACKAGES_STATES
// #define PRINT_ENTRY_NETWORK_FLAG_STATES
// #define PRINT_TIMER_STATES
// #define PRINT_GAP_EVENT_STATES
// #define PRINT_NEIGHBOR_TABLE_STATES
// #define PRINT_UP_ROUTING_TABLE_STATES
// #define PRINT_DOWN_ROUTING_TABLE_STATES

#define PRINT_HELLO_DETAIL
#define PRINT_ANRREQ_DETAIL
#define PRINT_ANRREP_DETAIL
#define PRINT_ANHSP_DETAIL
#define PRINT_RRER_DETAIL
#define PRINT_HSRREP_DETAIL
#define PRINT_MESSAGE_DETAIL
#define PRINT_BLOCK_MESSAGE_DETAIL

// #define ONLY_SEND_HELLO

 #define PRINT_MESSAGE_FOR_OPENWRT
// #define SENGDING_MESSAGE_PERIODIC

#endif