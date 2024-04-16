/*
 * @Author: Zhenwei Song zhenwei.song@qq.com
 * @Date: 2024-01-16 15:05:23
 * @LastEditors: Zhenwei Song zhenwei.song@qq.com
 * @LastEditTime: 2024-01-28 13:28:43
 * @FilePath: \esp32\esp32_ble\gatt_server_service_table_modified\main\ble_timer.h
 * @Description: 仅供学习交流使用
 * Copyright (c) 2024 by Zhenwei Song, All Rights Reserved.
 */

#ifndef BLE_TIMER_H_
#define BLE_TIMER_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/projdefs.h"

#define REFRESH_DOWN_ROUTING_TABLE_TIME 1000 // 影响路由表的维护，定为1s
#define ADV_TIME 20
#define REC_TIME 0
#define HELLO_TIME 5000
#define MESSAGE_TIME 5000
#define RESET_TIMER1_TIMEOUT_TIME 1000
#define RESET_TIMER2_TIMEOUT_TIME 1000
#define RERR_REPEAT_TIME 1000

#define NEIGHBOR_TABLE_COUNT 30 // 邻居表维护时长5s

#define TIME1_TIMER_PERIOD 10000000
#define TIME2_TIMER_PERIOD 10000000 // 5 seconds
#define TIME3_TIMER_PERIOD 10000000 // 10 seconds
#define TIME4_TIMER_PERIOD 20000000 // 20 seconds

extern SemaphoreHandle_t xCountingSemaphore_timeout1;
extern SemaphoreHandle_t xCountingSemaphore_timeout2;

extern esp_timer_handle_t ble_time1_timer;
extern esp_timer_handle_t ble_time2_timer;
extern esp_timer_handle_t ble_time3_timer;
extern esp_timer_handle_t ble_time4_timer;

extern bool timer1_timeout;
extern bool timer2_timeout;

extern bool timer1_running;
extern bool timer2_running;
extern bool timer3_running;

extern bool entry_network_flag;

void ble_timer_init(void);

void time1_timer_cb(void);

void time2_timer_cb(void);

void time3_timer_cb(void);

void time4_timer_cb(void);

void time5_timer_cb(void);

#endif