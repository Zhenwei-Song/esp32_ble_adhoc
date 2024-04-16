/*
 * @Author: Zhenwei Song zhenwei.song@qq.com
 * @Date: 2023-12-05 17:18:06
 * @LastEditors: Zhenwei Song zhenwei.song@qq.com
 * @LastEditTime: 2024-01-20 09:48:09
 * @FilePath: \esp32\esp32_ble\gatt_server_service_table_modified\main\ble.h
 * @Description: 仅供学习交流使用
 * Copyright (c) 2023 by Zhenwei Song, All Rights Reserved.
 */
/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "nvs_flash.h"

/* Attributes State Machine */
enum {
    IDX_SVC,
    IDX_CHAR_A,
    IDX_CHAR_VAL_A,
    IDX_CHAR_CFG_A,

    IDX_CHAR_B,
    IDX_CHAR_VAL_B,

    IDX_CHAR_C,
    IDX_CHAR_VAL_C,

    HRS_IDX_NB,
};

#define TAG "BLE_BROADCAST"

// SemaphoreHandle_t xMutex1;

#define ADV_INTERVAL_MS 2000 // 广播间隔时间，单位：毫秒
#define SCAN_INTERVAL_MS 225
#define DATA_INTERVAL_MS 600

uint32_t duration = 0;

static const char remote_device_name[] = "OLTHR";

// 配置广播参数
esp_ble_adv_params_t adv_params = {
#if 1
    .adv_int_min = ADV_INTERVAL_MS / 0.625,        // 广播间隔时间，单位：0.625毫秒
    .adv_int_max = (ADV_INTERVAL_MS + 10) / 0.625, // 广播间隔时间，单位：0.625毫秒
#else
    .adv_int_min = 0x20,
    .adv_int_max = 0x30,
#endif
    .adv_type = ADV_TYPE_NONCONN_IND, // 不可连接
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL, // 在所有三个信道上广播
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_WLST_CON_WLST,
};

// 配置扫描参数
static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_PASSIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
#if 1
    .scan_interval = 0xff, // 50 * 0.625ms = 31.25ms
    .scan_window = 0xff,
#else
    .scan_interval = SCAN_INTERVAL_MS / 0.625,
    .scan_window = SCAN_INTERVAL_MS / 0.625,
#endif
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE};

static void all_queue_init(void);
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void ble_rec_data_task(void *pvParameters);
static void ble_send_data_task(void *pvParameters);